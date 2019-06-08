#include <nds/arm9/sprite.h>
#include <nds/arm9/background.h>
#include "vram.h"
#include "vector.h"
#include "string.h"
#include "vramheap.h"
#include "qsort.h"
#include "fat.h"
#include "consts.s"
#include "gui/FileBrowser.h"
#include "sd_access.h"
#include "crc16.h"

#define REG_SEND_FIFO	(*((vu32*)0x04000188))
#define REG_RECV_FIFO	(*((vu32*)0x04100000))

typedef enum KEYPAD_BITS
{
	KEY_A = BIT(0),
	//!< Keypad A button.
	KEY_B = BIT(1),
	//!< Keypad B button.
	KEY_SELECT = BIT(2),
	//!< Keypad SELECT button.
	KEY_START = BIT(3),
	//!< Keypad START button.
	KEY_RIGHT = BIT(4),
	//!< Keypad RIGHT button.
	KEY_LEFT = BIT(5),
	//!< Keypad LEFT button.
	KEY_UP = BIT(6),
	//!< Keypad UP button.
	KEY_DOWN = BIT(7),
	//!< Keypad DOWN button.
	KEY_R = BIT(8),
	//!< Right shoulder button.g
	KEY_L = BIT(9),
	//!< Left shoulder button.
	KEY_X = BIT(10),
	//!< Keypad X button.
	KEY_Y = BIT(11),
	//!< Keypad Y button.
	KEY_TOUCH = BIT(12),
	//!< Touchscreen pendown.
	KEY_LID = BIT(13) //!< Lid state.
} KEYPAD_BITS;

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
	dc_invalidate_range(buffer, numSectors * 512);
	REG_SEND_FIFO = 0xAA5500DF;
	REG_SEND_FIFO = sector;
	REG_SEND_FIFO = numSectors;
	REG_SEND_FIFO = (uint32_t)buffer;//arm7_address;
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

PUT_IN_VRAM void initialize_cache()
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
}

extern "C" PUT_IN_VRAM void sd_write_save()
{
	vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) | 0x00800000);
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
	int toread = (vram_cd->sd_info.nr_sectors_per_cluster * 512 > SAVE_DATA_SIZE) ? SAVE_DATA_SIZE / 512 : vram_cd->sd_info.nr_sectors_per_cluster;
	while (cur_cluster != 0 && (data_read + toread * 512) <= SAVE_DATA_SIZE)
	{
		write_sd_sectors_safe(cur_cluster, toread, (void*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + data_read));
		data_read += toread * 512;
		cur_cluster = *cluster_table++;
	}
	vramcd_uncached->save_work.save_state = SAVE_WORK_STATE_CLEAN;
}

//to be called after dldi has been initialized (with the appropriate init function)
extern "C" PUT_IN_VRAM void sd_init(uint8_t* bios_dst)
{
	vramheap_init();
	FileBrowser* fileBrowser = new FileBrowser();
	fileBrowser->Run();
	delete fileBrowser;
	MI_WriteByte(&vram_cd->sd_info.nr_sectors_per_cluster, vram_cd->fatFs.csize);
	vram_cd->sd_info.cluster_shift = 31 - __builtin_clz(vram_cd->sd_info.nr_sectors_per_cluster * 512);
	vram_cd->sd_info.cluster_mask = (1 << vram_cd->sd_info.cluster_shift) - 1;
	initialize_cache();
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