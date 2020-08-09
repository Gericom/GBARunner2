#include <nds/arm9/sprite.h>
#include <nds/arm9/background.h>
#include "vram.h"
#include "scfg.h"
#include "../../common/fifo.h"
#include "vector.h"
#include "string.h"
#include "vramheap.h"
#include "qsort.h"
#include "fat.h"
#include "consts.s"
#include "gui/UIContext.h"
#include "gui/FileBrowser.h"
#include "gui/SettingsScreen.h"
#include "settings.h"
#include "bios/bios.h"
#include "crc16.h"
#include "emu/romGpio.h"
#include "gbaBoot.h"
#include "dsp/dsp.h"
#include "dsp/DspProcess.h"
#include "dsp/twlwram.h"
#include "dsp/dsp_ipc.h"
#include "GBARunner2_cdc.h"
#include "sd_access.h"

//#define DONT_CREATE_SAVE_FILES

extern "C" void dc_invalidate_range(void* start, uint32_t length);
extern "C" void dc_flush_range(void*      start, uint32_t length);
extern "C" void dc_flush_all();
extern "C" void dc_invalidate_all();
extern "C" void dc_wait_write_buffer_empty();

#ifdef ARM7_DLDI
//buffer should be in main memory
extern "C" ITCM_CODE __attribute__ ((noinline)) void read_sd_sectors_safe(sec_t sector, sec_t numSectors, void* buffer)
{
	//remote procedure call on arm7
//assume buffer is in the vram cd block
//uint32_t arm7_address = ((uint32_t)buffer) - ((uint32_t)vram_cd) + 0x06000000;
//dc_wait_write_buffer_empty();
//dc_invalidate_all();
//dc_flush_range(vram_cd, 256 * 1024);
//dc_flush_all();
//map cd to arm7
//REG_VRAMCNT_CD = VRAM_CD_ARM7;
	//REG_VRAMCNT_C = VRAM_C_ARM7;
	REG_SEND_FIFO = 0xAA5500DF;
	REG_SEND_FIFO = sector;
	REG_SEND_FIFO = numSectors;
	REG_SEND_FIFO = (uint32_t)buffer;//arm7_address;
	dc_invalidate_range(buffer, numSectors * 512);
	//wait for response
	do
	{
		while(*((vu32*)0x04000184) & (1 << 8));
	} while(REG_RECV_FIFO != 0x55AAAA55);
	//REG_VRAMCNT_C = VRAM_C_ARM9;
//REG_VRAMCNT_CD = VRAM_CD_ARM9;
	//invalidate
	
	//dc_invalidate_all();
}

//buffer should be in main memory
extern "C" ITCM_CODE __attribute__((noinline)) void write_sd_sectors_safe(sec_t sector, sec_t numSectors, const void* buffer)
{
	//remote procedure call on arm7
	//assume buffer is in the vram cd block
	//uint32_t arm7_address = ((uint32_t)buffer) - ((uint32_t)vram_cd) + 0x06000000;
	//dc_wait_write_buffer_empty();
	//dc_invalidate_all();
	//dc_flush_range(vram_cd, 256 * 1024);
	//dc_flush_all();
	//map cd to arm7
	//REG_VRAMCNT_CD = VRAM_CD_ARM7;
	//REG_VRAMCNT_C = VRAM_C_ARM7;
	dc_flush_range((void*)buffer, numSectors * 512);
	dc_wait_write_buffer_empty();
	REG_SEND_FIFO = 0xAA5500F0;
	REG_SEND_FIFO = sector;
	REG_SEND_FIFO = numSectors;
	REG_SEND_FIFO = (uint32_t)buffer;//arm7_address;
									 //wait for response
	do
	{
		while (*((vu32*)0x04000184) & (1 << 8));
	} while (REG_RECV_FIFO != 0x55AAAA55);
	//REG_VRAMCNT_C = VRAM_C_ARM9;
	//REG_VRAMCNT_CD = VRAM_CD_ARM9;
	//invalidate
	//dc_invalidate_range(buffer, numSectors * 512);
	//dc_invalidate_all();
}
#endif

extern "C" PUT_IN_VRAM void initialize_cache()
{
	vram_cd->sd_info.access_counter = 0;
	//--Issue #2--
	//WATCH OUT! These 3 loops should be loops, and not calls to memset
	//Newer versions of the gcc compiler enable -ftree-loop-distribute-patterns when O3 is used
	//and replace those loops with calls to memset, which is not available anymore at the time this is called
	//Solution: Use O2 instead (which is a stupid solution in my opinion)
	for (u32 i = 0; i < sizeof(vram_cd->cluster_cache) / 4; i++)
		((uint32_t*)&vram_cd->cluster_cache)[i] = 0;
	for (u32 i = 0; i < sizeof(vram_cd->gba_rom_is_cluster_cached_table) / 4; i++)
		((uint32_t*)&vram_cd->gba_rom_is_cluster_cached_table)[i] = 0xFFFFFFFF;
	for (u32 i = 0; i < sizeof(vram_cd->cluster_cache_info) / 4; i++)
		((uint32_t*)&vram_cd->cluster_cache_info)[i] = 0;
	vram_cd->cluster_cache_info.total_nr_cacheblocks = sizeof(vram_cd->cluster_cache) / 512;
	//>> vram_cd->sd_info.cluster_shift;
	for (u32 i = 0; i < vram_cd->cluster_cache_info.total_nr_cacheblocks; i++)
	{
		if (i > 0)
			vram_cd->cluster_cache_info.cache_linked_list[i].prev = i - 1;
		else
			vram_cd->cluster_cache_info.cache_linked_list[i].prev = CACHE_LINKED_LIST_NIL;
		if (i < vram_cd->cluster_cache_info.total_nr_cacheblocks - 1)
			vram_cd->cluster_cache_info.cache_linked_list[i].next = i + 1;
		else
			vram_cd->cluster_cache_info.cache_linked_list[i].next = CACHE_LINKED_LIST_NIL;
	}
	vram_cd->cluster_cache_info.cache_list_head = 0;
	vram_cd->cluster_cache_info.cache_list_tail = vram_cd->cluster_cache_info.total_nr_cacheblocks - 1;
	vram_cd->sound_emu_work.req_size_lock = 0;
	vram_cd->sound_emu_work.req_size = 0;
	vram_cd->sound_emu_work.req_write_ptr = 0;
	vram_cd->sound_emu_work.req_read_ptr = 0;
	vram_cd->sound_emu_work.resp_size_lock = 0;
	vram_cd->sound_emu_work.resp_size = 0;
	vram_cd->sound_emu_work.resp_write_ptr = 0;
	vram_cd->sound_emu_work.resp_read_ptr = 0;
	//vram_cd->save_work.save_enabled = 0;
	//vram_cd->save_work.save_state = SAVE_WORK_STATE_CLEAN;
#ifdef HANDLER_STATISTICS
	for (u32 i = 0; i < (2 * 1024 * 1024) / 4; i++)
		((u32*)STATISTICS_ADDRESS)[i] = 0;
#endif
}

extern "C" PUT_IN_VRAM void sd_write_save()
{
	//mute sound
    REG_SEND_FIFO = 0xAA5500FF;
    REG_SEND_FIFO = 0;
#ifdef USE_DSP_AUDIO
	//pause dsp audioto prevent ear ouchy
	dsp_sendIpcCommand(0x01000000, 1);
#endif
	vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) + UNCACHED_OFFSET);
	if (!vramcd_uncached->save_work.save_enabled || vramcd_uncached->save_work.save_state != SAVE_WORK_STATE_SDSAVE)
		return;
	uint16_t crc = crc16(0xFFFF, vram_cd->save_work.save_fat_table, sizeof(vram_cd->save_work.save_fat_table));
	if (vramcd_uncached->save_work.fat_table_crc != crc)
	{
		vramcd_uncached->save_work.save_state = SAVE_WORK_STATE_CLEAN;
		return;
	}
	u32* cluster_table = &vram_cd->save_work.save_fat_table[0];
	u32 cur_cluster = *cluster_table++;
	uint32_t data_read = 0;
	int toread = (vram_cd->sd_info.nr_sectors_per_cluster * 512 > vram_cd->save_work.saveSize) ? 
		vram_cd->save_work.saveSize >> 9 : vram_cd->sd_info.nr_sectors_per_cluster;
	while (cur_cluster != 0 && (data_read + (toread << 9)) <= vram_cd->save_work.saveSize)
	{
		write_sd_sectors_safe(cur_cluster, toread, (void*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + data_read));
		data_read += toread << 9;
		cur_cluster = *cluster_table++;
	}
	vramcd_uncached->save_work.save_state = SAVE_WORK_STATE_CLEAN;
	//unmute sound and resync
    REG_SEND_FIFO = 0xAA5500FF;
    REG_SEND_FIFO = 0x7F | 0x80000000;
#ifdef USE_DSP_AUDIO
	//continue dsp audio
	dsp_sendIpcCommand(0x01000000, 0);
#endif
}

#ifdef USE_DSP_AUDIO
static bool initDsp()
{
	REG_SCFG_EXT |= SCFG_EXT_ENABLE_DSP | SCFG_EXT_EXT_IRQ;
	twr_setBlockMapping(TWR_WRAM_BLOCK_A, TWR_WRAM_BASE, 0, TWR_WRAM_BLOCK_IMAGE_SIZE_32K);
	//map nwram
	twr_setBlockMapping(TWR_WRAM_BLOCK_B, 0x03800000, 256 * 1024, TWR_WRAM_BLOCK_IMAGE_SIZE_256K);
	twr_setBlockMapping(TWR_WRAM_BLOCK_C, 0x03C00000, 256 * 1024, TWR_WRAM_BLOCK_IMAGE_SIZE_256K);
	DspProcess dspProc = DspProcess();
	if(!dspProc.ExecuteDsp1((const dsp_dsp1_t*)GBARunner2_cdc))
		return false;
	//remove nwram from the memory map
	twr_setBlockMapping(TWR_WRAM_BLOCK_B, TWR_WRAM_BASE, 0, TWR_WRAM_BLOCK_IMAGE_SIZE_32K);
	twr_setBlockMapping(TWR_WRAM_BLOCK_C, TWR_WRAM_BASE, 0, TWR_WRAM_BLOCK_IMAGE_SIZE_32K);
	//enable dsp irqs
	*(vu32*)0x04000210 |= 1 << 24;
	return true;
}
#endif

//to be called after dldi has been initialized (with the appropriate init function)
extern "C" PUT_IN_VRAM void sd_init()
{
	vramheap_init();
	REG_DISPCNT = 0xA0000;
    VRAM_D_CR = 0x84;
	VRAM_I_CR = 0x00;
	UIContext* uiContext = new UIContext();
	uiContext->GetUIManager().Update();
	while (*((vu16*)0x04000004) & 1);
	while (!(*((vu16*)0x04000004) & 1));
	uiContext->GetUIManager().VBlank();
#if defined(USE_DSI_16MB) || defined(USE_3DS_32MB)
	if(!REG_SCFG_EXT)
		uiContext->FatalError("SCFG Locked!");
#ifdef USE_DSP_AUDIO
	if(!initDsp())
		uiContext->FatalError("DSP init failed!");
#endif
#endif
	if (f_mount(&vram_cd->fatFs, "", 1) != FR_OK)
		uiContext->FatalError("Couldn't mount sd card!");
#ifndef ISNITRODEBUG
	if (f_stat("0:/_gba", NULL) != FR_OK)
		if(f_mkdir("0:/_gba") != FR_OK)
			uiContext->FatalError("Couldn't create /_gba folder!");
#endif
	switch (bios_load())
	{
		case BIOS_LOAD_RESULT_OK:
			break;
		case BIOS_LOAD_RESULT_ERROR:
			uiContext->FatalError("Error while loading bios!");
			break;
		case BIOS_LOAD_RESULT_NOT_FOUND:
			uiContext->FatalError("Bios not found!");
			break;
		case BIOS_LOAD_RESULT_INVALID:
			uiContext->FatalError("Bios invalid!");
			break;
	}
	settings_initialize();

	int next = 0;

	//argv boot
	if(*(u32*)0x03000000 == 0x5f617267)
	{
		const char* argvPath = (const char*)0x03000004;
		if(argvPath[2] == ':')
			argvPath += 3;
		else if(argvPath[3] == ':')
			argvPath += 4;
		if(gbab_loadRom(argvPath) == ROM_LOAD_RESULT_OK)
			next = 2;
	}
	
	while(true)
	{
		uiContext->ResetVram();
		if(next == 0)
		{
			FileBrowser* fileBrowser = new FileBrowser(uiContext);
			next = fileBrowser->Run();
			delete fileBrowser;
		}
		else if(next == 1)
		{
			SettingsScreen* settings = new SettingsScreen(uiContext);
			settings->Run();
			delete settings;
#ifndef ISNITRODEBUG
			if(!settings_save())
				uiContext->FatalError("Couldn't save settings!");
#endif
			next = 0;
		}
		else if(next == 2)
			break;
	}	
	delete uiContext;
	MI_WriteByte(&vram_cd->sd_info.nr_sectors_per_cluster, vram_cd->fatFs.csize);
	vram_cd->sd_info.cluster_shift = 31 - __builtin_clz(vram_cd->sd_info.nr_sectors_per_cluster * 512);
	vram_cd->sd_info.cluster_mask = (1 << vram_cd->sd_info.cluster_shift) - 1;
	initialize_cache();
	rio_init(RIO_NONE);
	gbab_setupGfx();
#ifdef USE_DSP_AUDIO
	//continue dsp audio
	dsp_sendIpcCommand(0x01000000, 0);
#endif
}

//gets an empty one or wipes the oldest
extern "C" ITCM_CODE int get_new_cache_block()
{
	/*int oldest = -1;
	int oldest_counter_val = -1;
	for(int i = 0; i < vram_cd->cluster_cache_info.total_nr_cacheblocks; i++)
	{
		if(!vram_cd->cluster_cache_info.cache_block_info[i].in_use)
			return i;
		if(vram_cd->cluster_cache_info.cache_block_info[i].counter > oldest_counter_val)
		{
			oldest = i;
			oldest_counter_val = vram_cd->cluster_cache_info.cache_block_info[i].counter;
		}
	}
	//wipe this old block
	vram_cd->gba_rom_is_cluster_cached_table[vram_cd->cluster_cache_info.cache_block_info[oldest].cluster_index] = 0xFF;
	vram_cd->cluster_cache_info.cache_block_info[oldest].in_use = 0;
	vram_cd->cluster_cache_info.cache_block_info[oldest].counter = 0;
	*/
	int block;
#if defined(CACHE_STRATEGY_LRU)
	int least_used = -1;
	int least_used_val = 0x7FFFFFFF;
	for(int i = 0; i < vram_cd->cluster_cache_info.total_nr_cacheblocks; i++)
	{
		if(!vram_cd->cluster_cache_info.cache_block_info[i].in_use)
			return i;
		if(vram_cd->cluster_cache_info.cache_block_info[i].counter < least_used_val)
		{
			least_used = i;
			least_used_val = vram_cd->cluster_cache_info.cache_block_info[i].counter;
		}
	}
	block = least_used;
#endif
#ifdef CACHE_STRATEGY_LFU
	int least_used = vram_cd->cluster_cache_info.total_nr_cacheblocks - 1;//-1;
	int least_used_val = 0x7FFFFFFF;
	for(int i = 0; i < vram_cd->cluster_cache_info.total_nr_cacheblocks; i++)
	{
		if(!vram_cd->cluster_cache_info.cache_block_info[i].in_use)
			return i;

		if((vram_cd->sd_info.access_counter - vram_cd->cluster_cache_info.cache_block_info[i].counter) > 750)//2500)
		{
			least_used = i;
			break;
		}

		//if(vram_cd->cluster_cache_info.cache_block_info[i].counter2 < least_used_val)
	//{
	//	least_used = i;
	//	least_used_val = vram_cd->cluster_cache_info.cache_block_info[i].counter2;
		//}
		if((vram_cd->sd_info.access_counter + 10 * vram_cd->cluster_cache_info.cache_block_info[i].counter2) < least_used_val)
		{
			least_used = i;
			least_used_val = vram_cd->cluster_cache_info.cache_block_info[i].counter2;
		}
	}
	block = least_used;
#endif
#ifdef CACHE_STRATEGY_MRU
	int most_used = -1;
	int most_used_val = -1;
	for(int i = 0; i < vram_cd->cluster_cache_info.total_nr_cacheblocks; i++)
	{
		if(!vram_cd->cluster_cache_info.cache_block_info[i].in_use)
			return i;
		if(vram_cd->cluster_cache_info.cache_block_info[i].counter > most_used_val)
		{
			most_used = i;
			most_used_val = vram_cd->cluster_cache_info.cache_block_info[i].counter;
		}
	}
	block = most_used;
#endif
#ifdef CACHE_STRATEGY_ROUND_ROBIN
	block = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
	if(vram_cd->sd_info.access_counter >= vram_cd->cluster_cache_info.total_nr_cacheblocks)
		vram_cd->sd_info.access_counter = 0;
	if(vram_cd->cluster_cache_info.cache_block_info[block].in_use)
		vram_cd->gba_rom_is_cluster_cached_table[vram_cd->cluster_cache_info.cache_block_info[block].cluster_index] = 0xFFFF;
#endif
#ifdef CACHE_STRATEGY_LRU_LIST
	block = vram_cd->cluster_cache_info.cache_list_head;
	//move block to tail
	cluster_cache_block_link_t* curBlock = &vram_cd->cluster_cache_info.cache_linked_list[block];
	vram_cd->cluster_cache_info.cache_list_head = curBlock->next;
	vram_cd->cluster_cache_info.cache_linked_list[curBlock->next].prev = CACHE_LINKED_LIST_NIL;
	vram_cd->cluster_cache_info.cache_linked_list[vram_cd->cluster_cache_info.cache_list_tail].next = block;
	curBlock->prev = vram_cd->cluster_cache_info.cache_list_tail;
	curBlock->next = CACHE_LINKED_LIST_NIL;
	vram_cd->cluster_cache_info.cache_list_tail = block;
	if (vram_cd->cluster_cache_info.cache_block_info[block].in_use)
		vram_cd->gba_rom_is_cluster_cached_table[vram_cd->cluster_cache_info.cache_block_info[block].cluster_index] =
			0xFFFF;
#endif
	//wipe this old block
	//vram_cd->gba_rom_is_cluster_cached_table[vram_cd->cluster_cache_info.cache_block_info[block].cluster_index] = 0xFFFF;
	//vram_cd->cluster_cache_info.cache_block_info[block].in_use = 0;
	//vram_cd->cluster_cache_info.cache_block_info[block].counter = 0;
	//vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 0;
	return block;
}

extern "C" ITCM_CODE
uint32_t sdread32_uncached(uint32_t address)
{
	uint32_t cluster_index = address >> 9; // >> vram_cd->sd_info.cluster_shift;
	int      block = get_new_cache_block();
	vram_cd->gba_rom_is_cluster_cached_table[cluster_index] = block;
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
	vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
	read_sd_sectors_safe(/*get_sector_from_cluster(*/vram_cd->gba_rom_cluster_table[cluster_index >> (vram_cd->sd_info.
cluster_shift - 9)]/*)*/ + (cluster_index & (vram_cd->sd_info.cluster_mask >> 9)), 1
/*vram_cd->sd_info.nr_sectors_per_cluster*/, &vram_cd->cluster_cache[block << 9/*vram_cd->sd_info.cluster_shift*/]);
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
#endif
#ifdef CACHE_STRATEGY_LFU
	vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 1;
#endif
	uint32_t cluster_offset = address & 0x1FF; //& vram_cd->sd_info.cluster_mask;
	void*    cluster_data = (void*)&vram_cd->cluster_cache[block << 9]; //vram_cd->sd_info.cluster_shift];
	return *((uint32_t*)((u8*)cluster_data + cluster_offset));
}

extern "C" ITCM_CODE
uint16_t sdread16_uncached(uint32_t address)
{
	uint32_t cluster_index = address >> 9; //vram_cd->sd_info.cluster_shift;
	int      block = get_new_cache_block();
	vram_cd->gba_rom_is_cluster_cached_table[cluster_index] = block;
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
	vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
	read_sd_sectors_safe(/*get_sector_from_cluster(*/vram_cd->gba_rom_cluster_table[cluster_index >> (vram_cd->sd_info.
cluster_shift - 9)]/*)*/ + (cluster_index & (vram_cd->sd_info.cluster_mask >> 9)), 1
/*vram_cd->sd_info.nr_sectors_per_cluster*/, &vram_cd->cluster_cache[block << 9/*vram_cd->sd_info.cluster_shift*/]);
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
#endif
#ifdef CACHE_STRATEGY_LFU
	vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 1;
#endif
	uint32_t cluster_offset = address & 0x1FF; //vram_cd->sd_info.cluster_mask;
	void*    cluster_data = (void*)&vram_cd->cluster_cache[block << 9]; //vram_cd->sd_info.cluster_shift];
	return *((uint16_t*)((u8*)cluster_data + cluster_offset));
}

extern "C" ITCM_CODE
uint8_t sdread8_uncached(uint32_t address)
{
	uint32_t cluster_index = address >> 9; //vram_cd->sd_info.cluster_shift;
	int      block = get_new_cache_block();
	vram_cd->gba_rom_is_cluster_cached_table[cluster_index] = block;
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
	vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
	read_sd_sectors_safe(/*get_sector_from_cluster(*/vram_cd->gba_rom_cluster_table[cluster_index >> (vram_cd->sd_info.
cluster_shift - 9)]/*)*/ + (cluster_index & (vram_cd->sd_info.cluster_mask >> 9)), 1
/*vram_cd->sd_info.nr_sectors_per_cluster*/, &vram_cd->cluster_cache[block << 9/*vram_cd->sd_info.cluster_shift*/]);
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
#endif
#ifdef CACHE_STRATEGY_LFU
	vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 1;
#endif
	uint32_t cluster_offset = address & 0x1FF; //vram_cd->sd_info.cluster_mask;
	void*    cluster_data = (void*)&vram_cd->cluster_cache[block << 9]; //vram_cd->sd_info.cluster_shift];
	return *((uint8_t*)((u8*)cluster_data + cluster_offset));
}