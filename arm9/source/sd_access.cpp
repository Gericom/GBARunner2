#include "vram.h"
#include "vector.h"
#include "string.h"
#include "vramheap.h"
#include "qsort.h"
#include "sd_access.h"
#include "fat.h"
#include "consts.s"
#include "fat/Directory.h"
#include "fat/File.h"
#include "fat/DirectoryEnumerator.h"

#define REG_SEND_FIFO	(*((vu32*)0x04000188))
#define REG_RECV_FIFO	(*((vu32*)0x04100000))

typedef enum KEYPAD_BITS {
	KEY_A = BIT(0),  //!< Keypad A button.
	KEY_B = BIT(1),  //!< Keypad B button.
	KEY_SELECT = BIT(2),  //!< Keypad SELECT button.
	KEY_START = BIT(3),  //!< Keypad START button.
	KEY_RIGHT = BIT(4),  //!< Keypad RIGHT button.
	KEY_LEFT = BIT(5),  //!< Keypad LEFT button.
	KEY_UP = BIT(6),  //!< Keypad UP button.
	KEY_DOWN = BIT(7),  //!< Keypad DOWN button.
	KEY_R = BIT(8),  //!< Right shoulder button.
	KEY_L = BIT(9),  //!< Left shoulder button.
	KEY_X = BIT(10), //!< Keypad X button.
	KEY_Y = BIT(11), //!< Keypad Y button.
	KEY_TOUCH = BIT(12), //!< Touchscreen pendown.
	KEY_LID = BIT(13)  //!< Lid state.
} KEYPAD_BITS;

//#define DONT_CREATE_SAVE_FILES

ITCM_CODE __attribute__ ((noinline)) void MI_WriteByte(void *address, uint8_t value)
{
    uint16_t val = *(uint16_t *)((uint32_t)address & ~1);

    if ((uint32_t)address & 1)
		*(uint16_t *)((uint32_t)address & ~1) = (uint16_t)(((value & 0xff) << 8) | (val & 0xff));
    else
		*(uint16_t *)((uint32_t)address & ~1) = (uint16_t)((val & 0xff00) | (value & 0xff));
}

extern "C" void dc_invalidate_range(void* start, uint32_t length);
extern "C" void dc_flush_range(void* start, uint32_t length);
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
	//wait for response
	do
	{
		while(*((vu32*)0x04000184) & (1 << 8));
	} while(REG_RECV_FIFO != 0x55AAAA55);
	//REG_VRAMCNT_C = VRAM_C_ARM9;
	//REG_VRAMCNT_CD = VRAM_CD_ARM9;
	//invalidate
	dc_invalidate_range(buffer, numSectors * 512);
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

//sd_info_t gSDInfo;

//simple means without any caching and therefore slow, but that doesn't matter for the functions that use this
PUT_IN_VRAM uint32_t get_cluster_fat_value_simple(uint32_t cluster)
{
	uint32_t fat_offset = cluster * 4;
	uint32_t fat_sector = vram_cd->sd_info.first_fat_sector + (fat_offset >> 9); //sector_size);
	uint32_t ent_offset = fat_offset & 0x1FF;//% sector_size;
	void* tmp_buf = (void*)vram_cd->tmp_cluster;
	read_sd_sectors_safe(fat_sector, 1, tmp_buf);//_DLDI_readSectors_ptr(fat_sector, 1, tmp_buf);
	return *((uint32_t*)(((uint8_t*)tmp_buf) + ent_offset)) & 0x0FFFFFFF;
}

PUT_IN_VRAM void initialize_cache()
{
	vram_cd->sd_info.access_counter = 0;
	//--Issue #2--
	//WATCH OUT! These 3 loops should be loops, and not calls to memset
	//Newer versions of the gcc compiler enable -ftree-loop-distribute-patterns when O3 is used
	//and replace those loops with calls to memset, which is not available anymore at the time this is called
	//Solution: Use O2 instead (which is a stupid solution in my opinion)
	for(u32 i = 0; i < sizeof(vram_cd->cluster_cache) / 4; i++)
		((uint32_t*)&vram_cd->cluster_cache)[i] = 0;
	for(u32 i = 0; i < sizeof(vram_cd->gba_rom_is_cluster_cached_table) / 4; i++)
		((uint32_t*)&vram_cd->gba_rom_is_cluster_cached_table)[i] = 0xFFFFFFFF;
	for(u32 i = 0; i < sizeof(vram_cd->cluster_cache_info) / 4; i++)
		((uint32_t*)&vram_cd->cluster_cache_info)[i] = 0;
	vram_cd->cluster_cache_info.total_nr_cacheblocks = sizeof(vram_cd->cluster_cache) / 512; //>> vram_cd->sd_info.cluster_shift;
	for (u32 i = 0; i < vram_cd->cluster_cache_info.total_nr_cacheblocks; i++)
	{
		if(i > 0)
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

PUT_IN_VRAM void clear_rows(int from_row, int to_row)
{	
	for(int i=from_row; i<=to_row; i++)
	{		
		*((vu64*)0x06202000 + 4*i + 0) = 0x2020202020202020;
		*((vu64*)0x06202000 + 4*i + 1) = 0x2020202020202020;
		*((vu64*)0x06202000 + 4*i + 2) = 0x2020202020202020;
		*((vu64*)0x06202000 + 4*i + 3) = 0x2020202020202020;
	}
}

PUT_IN_VRAM int comp_dir_entries(const DirectoryEntry* &dir1, const DirectoryEntry* &dir2)
{
	if (dir1->GetIsDirectory() && !dir2->GetIsDirectory())
		return -1;
	if (!dir1->GetIsDirectory() && dir2->GetIsDirectory())
		return 1;
	return strcasecmp(dir1->GetName(), dir2->GetName());
}

PUT_IN_VRAM void print_folder_contents(vector& entries_names, int startRow)
{
	//print a line on second row
	*((vu64*)0x06202000 + 4 + 0) = 0xC4C4C4C4C4C4C4C4;
	*((vu64*)0x06202000 + 4 + 1) = 0xC4C4C4C4C4C4C4C4;
	*((vu64*)0x06202000 + 4 + 2) = 0xC4C4C4C4C4C4C4C4;
	*((vu64*)0x06202000 + 4 + 3) = 0xC4C4C4C4C4C4C4C4;
	
	clear_rows(2, 23);
	
	for(int i=0; i<(vector_count(&entries_names) - startRow) && i < ENTRIES_PER_SCREEN; i++)
	{
		const char* name = ((DirectoryEntry*)entries_names[i + startRow])->GetName();
		int len = strlen(name);
			
		MI_WriteByte((u8*)0x06202000 + 32 * (i+ENTRIES_START_ROW) + 1, name[0]);
		
		for(int j=0; j<31 && j<len; j++)
			MI_WriteByte((u8*)0x06202000 + 32 * (i+ENTRIES_START_ROW) + j + 1, name[j]);
	}
}

PUT_IN_VRAM void get_folder_contents(vector& entries_names, Directory* dir)
{	
	DirectoryEnumerator* enumerator = dir->GetEnumerator();
	DirectoryEntry* entry;
	while ((entry = enumerator->GetNext()) != NULL)
	{
		const char* name = entry->GetName();
		if(!strcmp(name, "."))
		{
			delete entry;
			continue;
		}
		uint8_t* point_ptr = (uint8_t*)strrchr(name, '.');
		if (entry->GetIsDirectory() || (point_ptr && !strcasecmp((char*)point_ptr, ".gba")))
			vector_add(&entries_names, entry);
		else
			delete entry;
	}
	delete enumerator;

	qsort(entries_names.data, entries_names.count, sizeof(DirectoryEntry*), (int(*)(const void*, const void*))comp_dir_entries);
}

PUT_IN_VRAM File* get_game_first_cluster(Directory* root, Directory* &gbaDir)
{	
	uint16_t keys = 0;
	uint16_t old_keys = 0;
	uint16_t new_keys = 0;
	uint16_t held_keys = 0;
	int countdown = KEY_REPEAT_FREQ;
	int delay = KEY_HOLD_DELAY;
	int start_at_position = 0;
	int cursor_position = 0;

	Directory* curDir = (Directory*)root->GetEntryByPath("GBA");
	if (curDir == NULL)
		curDir = root;

	vector entries_names;
	vector_init(&entries_names);
	
	get_folder_contents(entries_names, curDir);
	print_folder_contents(entries_names, start_at_position);
	
	while(1) {
		//show cursor
		MI_WriteByte((u8*)0x06202000 + 32*(cursor_position - start_at_position + ENTRIES_START_ROW), 0x1A);
		
		do {
			old_keys = new_keys;
			new_keys = ~*((vu16*)0x04000130);
			
			if(new_keys != old_keys)
			{
				countdown = KEY_REPEAT_FREQ;
				delay = KEY_HOLD_DELAY;
				held_keys = new_keys & ~old_keys;
			}
			if(--countdown == 0)
			{
				countdown = KEY_REPEAT_FREQ;
				if(delay > 0)
					delay--;
				else
					held_keys = new_keys;
			}
			
			keys = held_keys;			
			held_keys = 0;
			
			while (*((vu16*)0x04000006) != 192);
			while (*((vu16*)0x04000006) == 192);
		} while (!keys);
		
		//hide cursor
		if(keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
			MI_WriteByte((u8*)0x06202000 + 32*(cursor_position - start_at_position + ENTRIES_START_ROW), ' ');

		if (keys & KEY_L)
		{
			*((vu16*)0x04000304) ^= 0x8000;
		}

		if(keys & KEY_UP)
		{
			cursor_position--;
			if(cursor_position < 0)
				cursor_position = vector_count(&entries_names) - 1;
		}
		else if(keys & KEY_DOWN)
		{
			cursor_position++;
			if(cursor_position > (vector_count(&entries_names) - 1))
				cursor_position = 0;
		}
		else if(keys & KEY_LEFT)
		{
			if((cursor_position - SKIP_ENTRIES) < 0)
			{
				if(cursor_position != 0)
					cursor_position = 0;
				else
					cursor_position = vector_count(&entries_names) - 1;
			}
			else
				cursor_position = cursor_position - SKIP_ENTRIES;
		}
		else if(keys & KEY_RIGHT)
		{
			if(cursor_position + SKIP_ENTRIES > (vector_count(&entries_names) - 1))
			{
				if(cursor_position != (vector_count(&entries_names) - 1))
					cursor_position = vector_count(&entries_names) - 1;
				else
					cursor_position = 0;
			}
			else		
				cursor_position = cursor_position + SKIP_ENTRIES;
		}		
		else if (keys & KEY_A)
		{
			*((vu32*)0x06202000) = 0x44414f4c; //LOAD
			
			DirectoryEntry* file = (DirectoryEntry*)entries_names[cursor_position];
			if(file->GetIsDirectory())
			{
				if (curDir != root)
					delete curDir;
				curDir = (Directory*)file;

				for (int i = 0; i < vector_count(&entries_names); i++)
					if(entries_names[i] != curDir)
						delete (DirectoryEntry*)entries_names[i];
				vector_free(&entries_names);
				
				start_at_position = 0;
				cursor_position = 0;
				get_folder_contents(entries_names, curDir);
				print_folder_contents (entries_names, start_at_position);
				*((vu32*)0x06202000) = 0x20202020;
				continue;
			}
			else
			{
				clear_rows(2, 23);
				for (int i = 0; i < vector_count(&entries_names); i++)
					if (entries_names[i] != file)
						delete (DirectoryEntry*)entries_names[i];
				vector_free(&entries_names);
				gbaDir = curDir;
				return (File*)file;
			}
		}		
		else if(keys & KEY_B)
		{			
			*((vu32*)0x06202000) = 0x44414f4c; //LOAD	
			Directory* parent = (Directory*)curDir->GetEntryByPath("..");
			if (parent == NULL)
				continue;

			if (curDir != root)
				delete curDir;
			curDir = parent;

			for (int i = 0; i < vector_count(&entries_names); i++)
				if (entries_names[i] != curDir)
					delete (DirectoryEntry*)entries_names[i];
			vector_free(&entries_names);

			start_at_position = 0;
			cursor_position = 0;
			get_folder_contents(entries_names, curDir);
			print_folder_contents(entries_names, start_at_position);
			*((vu32*)0x06202000) = 0x20202020;
			continue;
		}

		if(cursor_position < start_at_position)
			start_at_position = cursor_position;
		else if(cursor_position > start_at_position + ENTRIES_PER_SCREEN - 1)
			start_at_position = cursor_position - ENTRIES_PER_SCREEN + 1;
		else
			continue;
		print_folder_contents(entries_names, start_at_position);
	}
}

PUT_IN_VRAM void copy_bios(uint8_t* bios_dst, File* biosFile)
{	
	u32 cur_cluster = biosFile->GetFirstCluster();
	
	uint32_t* cluster_table = &vram_cd->gba_rom_cluster_table[0];
	while(cur_cluster < 0x0FFFFFF8)
	{
		*cluster_table = cur_cluster;
		cluster_table++;
		cur_cluster = get_cluster_fat_value_simple(cur_cluster);
	}
	*cluster_table = cur_cluster;
	cluster_table = &vram_cd->gba_rom_cluster_table[0];
	cur_cluster = *cluster_table++;
	uint32_t data_max = 16 * 1024;
	uint32_t data_read = 0;
	*((vu32*)0x06202000) = 0x59504F43; //COPY
	int toread = (vram_cd->sd_info.nr_sectors_per_cluster * 512 > 16 * 1024) ? 16 * 1024 / 512 : vram_cd->sd_info.nr_sectors_per_cluster;
	void* tmp_buf = (void*)vram_cd->tmp_cluster;
	while(cur_cluster < 0x0FFFFFF8 && (data_read + toread * 512) <= data_max)
	{
		read_sd_sectors_safe(get_sector_from_cluster(cur_cluster), toread, tmp_buf);
		arm9_memcpy16((uint16_t*)(bios_dst + data_read), (uint16_t*)tmp_buf, toread * 512 / 2);
		data_read += toread * 512;
		cur_cluster = *cluster_table++;
	}
	*((vu32*)0x06202000) = 0x20202020;
}


PUT_IN_VRAM uint16_t crc16(uint16_t crc, const void *buf, uint32_t size)
{
	static uint16_t crc16_tab[256] = {
		0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
		0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
		0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
		0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
		0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
		0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
		0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
		0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
		0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
		0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
		0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
		0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
		0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
		0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
		0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
		0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
		0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
		0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
		0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
		0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
		0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
		0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
		0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
		0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
		0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
		0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
		0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
		0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
		0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
		0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
		0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
		0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
	};

	const uint8_t *p = (const uint8_t*)buf;

	while (size--)
		crc = crc16_tab[(crc ^ (*p++)) & 0xFF] ^ (crc >> 8);

	return crc;
}

PUT_IN_VRAM void get_save(Directory* romDir, File* romFile)
{	
	vram_cd->save_work.save_state = SAVE_WORK_STATE_CLEAN;

	char nameBuf[256];
	const char* name = romFile->GetName();
	for (int i = 0; i < 256; i++)
	{
		char c = name[i];
		nameBuf[i] = c;
		if (c == 0)
			break;
	}
	
	char* long_name_ptr = strrchr(nameBuf, '.');
	long_name_ptr[1] = 's';
	long_name_ptr[2] = 'a';
	long_name_ptr[3] = 'v';
	long_name_ptr[4] = '\0';

	File* saveFile = (File*)romDir->GetEntryByPath(nameBuf);
		
	if(!saveFile)
	{		
#ifdef DONT_CREATE_SAVE_FILES
		for (int i = 0; i < 64 * 1024 / 4; i++)
		{
			((uint32_t*)MAIN_MEMORY_ADDRESS_SAVE_DATA)[i] = 0;
		}
		vram_cd->save_work.save_enabled = 0;
		return;
#else
		int entries_count;
		uint8_t short_name_buff[11];
		int is_lossy = gen_short_name((uint8_t*)nameBuf, short_name_buff, romDir);
		
		if(is_lossy < 0)
		{
			*((vu32*)0x06202000) = 0x47434E43;//CNCG Could not generate
			while(1);
		}
		else if(is_lossy == 1)
		{
			entries_count = 1;
			int totlen = strlen(nameBuf);
			while (totlen > 0)
			{
				entries_count++;
				totlen -= 13;
			}
			//entries_count = (strlen(nameBuf) + 12)/13 + 1;
		}
		else
		{
			entries_count = 1;
		}
		
		*((vu32*)0x06202000) = 0x56415343;//CSAV Creating save file
		
		uint32_t first_cluster = allocate_clusters(0, SAVE_DATA_SIZE);
		write_entries_to_sd(
				(uint8_t*)nameBuf,
				short_name_buff, 
				entries_count, 
				DIR_ATTRIB_ARCHIVE, 
				romDir->GetFirstCluster(),
				first_cluster,
				SAVE_DATA_SIZE);
		saveFile = (File*)romDir->GetEntryByPath(nameBuf);
		if (!saveFile)
		{
			*((vu32*)0x06202000) = 0x47434E43;//CNCG Could not generate
			while (1);
		}
#endif
	}
	
	//call to function to read the save file
	uint32_t* cluster_table = &vram_cd->save_work.save_fat_table[0];
	u32 cur_cluster = saveFile->GetFirstCluster();
	while (cur_cluster < 0x0FFFFFF8)
	{
		*cluster_table = cur_cluster;
		cluster_table++;
		cur_cluster = get_cluster_fat_value_simple(cur_cluster);
	}
	*cluster_table = cur_cluster;
	cluster_table = &vram_cd->save_work.save_fat_table[0];
	cur_cluster = *cluster_table++;
	uint32_t data_read = 0;
	*((vu32*)0x06202000) = 0x59504F43; //COPY
	int toread = (vram_cd->sd_info.nr_sectors_per_cluster * 512 > SAVE_DATA_SIZE) ? SAVE_DATA_SIZE / 512 : vram_cd->sd_info.nr_sectors_per_cluster;
	while (cur_cluster < 0x0FFFFFF8 && (data_read + toread * 512) <= SAVE_DATA_SIZE)
	{
		read_sd_sectors_safe(get_sector_from_cluster(cur_cluster), toread, (void*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + data_read));
		data_read += toread * 512;
		cur_cluster = *cluster_table++;
	}
	delete saveFile;
	vram_cd->save_work.fat_table_crc = crc16(0xFFFF, vram_cd->save_work.save_fat_table, sizeof(vram_cd->save_work.save_fat_table));
	vram_cd->save_work.save_enabled = 1;
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
	while (cur_cluster < 0x0FFFFFF8 && (data_read + toread * 512) <= SAVE_DATA_SIZE)
	{
		write_sd_sectors_safe(get_sector_from_cluster(cur_cluster), toread, (void*)(MAIN_MEMORY_ADDRESS_SAVE_DATA + data_read));
		data_read += toread * 512;
		cur_cluster = *cluster_table++;
	}
	vramcd_uncached->save_work.save_state = SAVE_WORK_STATE_CLEAN;
}

//to be called after dldi has been initialized (with the appropriate init function)
extern "C" PUT_IN_VRAM void sd_init(uint8_t* bios_dst)
{
	vramheap_init();
	void* tmp_buf = (void*)vram_cd->tmp_cluster;
	read_sd_sectors_safe(0, 1, tmp_buf);//read mbr
	mbr_t* mbr = (mbr_t*)tmp_buf;
	if(mbr->signature != 0xAA55)
	{
		*((vu32*)0x06202000) = 0x4E4D4252;//NMBR = no mbr found
		while(1);
	}
	sec_t boot_sect = 0;
	if(mbr->non_usefull_stuff[2] != 0x90)
	{
		if(mbr->partitions[0].partition_type != MBR_PARTITION_TYPE_FAT32 && mbr->partitions[0].partition_type != MBR_PARTITION_TYPE_FAT32_LBA)
		{
			*((vu32*)0x06202000) = 0x4E464154;//NFAT = no fat found
			while(1);
		}
		boot_sect = mbr->partitions[0].lba_partition_start;
		read_sd_sectors_safe(boot_sect, 1, tmp_buf);//read boot sector
	}
	bootsect_t* bootsect = (bootsect_t*)tmp_buf;
	//we need to calculate some stuff and save that for later use
	MI_WriteByte(&vram_cd->sd_info.nr_sectors_per_cluster, bootsect->nr_sector_per_cluster);
	vram_cd->sd_info.first_fat_sector = boot_sect + bootsect->nr_reserved_sectors;
	vram_cd->sd_info.first_cluster_sector = boot_sect + bootsect->nr_reserved_sectors + (bootsect->nr_fats * bootsect->fat32_nr_sectors_per_fat);
	vram_cd->sd_info.root_directory_cluster = bootsect->fat32_root_dir_cluster;
	vram_cd->sd_info.sectors_per_fat = bootsect->fat32_nr_sectors_per_fat;

	vram_cd->sd_info.cluster_shift = 31 - __builtin_clz(bootsect->nr_sector_per_cluster * 512);
	vram_cd->sd_info.cluster_mask = (1 << vram_cd->sd_info.cluster_shift) - 1;
		
	uint32_t cur_cluster = vram_cd->sd_info.root_directory_cluster;
	Directory* root = new Directory(cur_cluster, NULL);	

	File* biosFile = (File*)root->GetEntryByPath("GBA/bios.bin");
	if(!biosFile)
		biosFile = (File*)root->GetEntryByPath("bios.bin");

	if (!biosFile)
	{
		*((vu32*)0x06202000) = 0x464e4942; //BNFN BIOS not found
		while (1);
	}
	
	copy_bios(bios_dst, biosFile);
	
	Directory* romDir;
	File* gbaFile = get_game_first_cluster(root, romDir);
		
	get_save(romDir, gbaFile);

	if (romDir != root)
		delete romDir;
	delete root;
		
	vram_cd->sd_info.gba_rom_size = gbaFile->GetFileSize();
	//build the cluster table
	uint32_t* cluster_table = &vram_cd->gba_rom_cluster_table[0];
	cur_cluster = gbaFile->GetFirstCluster();
	delete gbaFile;
	while(cur_cluster < 0x0FFFFFF8)
	{
		*cluster_table = cur_cluster;
		cluster_table++;
		cur_cluster = get_cluster_fat_value_simple(cur_cluster);
	}
	*cluster_table = cur_cluster;
	//copy data to main memory
	cluster_table = &vram_cd->gba_rom_cluster_table[0];
	cur_cluster = *cluster_table++;
	uint32_t data_read = 0;
	*((vu32*)0x06202000) = 0x59504F43; //COPY
	while(cur_cluster < 0x0FFFFFF8 && (data_read + vram_cd->sd_info.nr_sectors_per_cluster * 512) <= ROM_DATA_LENGTH)
	{
		read_sd_sectors_safe(get_sector_from_cluster(cur_cluster), vram_cd->sd_info.nr_sectors_per_cluster, (void*)(MAIN_MEMORY_ADDRESS_ROM_DATA + data_read));//_DLDI_readSectors_ptr(get_sector_from_cluster(cur_cluster), vram_cd->sd_info.nr_sectors_per_cluster, (void*)(0x02040000 + data_read));//tmp_buf + 512);
		data_read += vram_cd->sd_info.nr_sectors_per_cluster * 512;
		cur_cluster = *cluster_table++;//get_cluster_fat_value_simple(cur_cluster);
	}
	*((vu32*)0x06202000) = 0x20204B4F;
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
		vram_cd->gba_rom_is_cluster_cached_table[vram_cd->cluster_cache_info.cache_block_info[block].cluster_index] = 0xFFFF;
#endif
	//wipe this old block
	//vram_cd->gba_rom_is_cluster_cached_table[vram_cd->cluster_cache_info.cache_block_info[block].cluster_index] = 0xFFFF;
	//vram_cd->cluster_cache_info.cache_block_info[block].in_use = 0;
	//vram_cd->cluster_cache_info.cache_block_info[block].counter = 0;
	//vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 0;
	return block;
}

/*extern "C" ITCM_CODE int ensure_cluster_cached(uint32_t cluster_index)
{
	int block = vram_cd->gba_rom_is_cluster_cached_table[cluster_index];
	if(block == 0xFFFF)
	{
		//load it
		block = get_new_cache_block();
		vram_cd->gba_rom_is_cluster_cached_table[cluster_index] = block;
		vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
		vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
		vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
#endif
#ifdef CACHE_STRATEGY_LFU
		vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 0;
#endif
		read_sd_sectors_safe(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index >> (vram_cd->sd_info.cluster_shift - 9)]) + (cluster_index & (vram_cd->sd_info.cluster_mask >> 9)), 1/*vram_cd->sd_info.nr_sectors_per_cluster/, &vram_cd->cluster_cache[block << 9/*vram_cd->sd_info.cluster_shift/]);
		//read_sd_sectors_safe(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);//_DLDI_readSectors_ptr(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);
	}
	//it is in cache now
	return block;
}

extern "C" ITCM_CODE void* get_cluster_data(uint32_t cluster_index)
{
	int block = ensure_cluster_cached(cluster_index);

#ifdef CACHE_STRATEGY_LRU_LIST
	//move block to tail
	cluster_cache_block_link_t* curBlock = &vram_cd->cluster_cache_info.cache_linked_list[block];
	vram_cd->cluster_cache_info.cache_linked_list[curBlock->prev].next = curBlock->next;
	vram_cd->cluster_cache_info.cache_linked_list[curBlock->next].prev = curBlock->prev;
	vram_cd->cluster_cache_info.cache_linked_list[vram_cd->cluster_cache_info.cache_list_tail].next = block;
	curBlock->prev = vram_cd->cluster_cache_info.cache_list_tail;
	curBlock->next = CACHE_LINKED_LIST_NIL;
	vram_cd->cluster_cache_info.cache_list_tail = block;
#endif

	//int block = vram_cd->gba_rom_is_cluster_cached_table[cluster_index];
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
#endif
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
	vram_cd->sd_info.access_counter++;
#endif
#ifdef CACHE_STRATEGY_LFU
	if(vram_cd->cluster_cache_info.cache_block_info[block].counter2 < 0x7F)
		vram_cd->cluster_cache_info.cache_block_info[block].counter2++;
#endif
	//increase_cluster_cache_counters();
	//vram_cd->cluster_cache_info.cache_block_info[block].counter = 0;
	return (void*)&vram_cd->cluster_cache[block << 9];//vram_cd->sd_info.cluster_shift];
}*/

extern "C" ITCM_CODE uint32_t sdread32_uncached(uint32_t address)
{
	uint32_t cluster_index = address >> 9;// >> vram_cd->sd_info.cluster_shift;
	int block = get_new_cache_block();
	vram_cd->gba_rom_is_cluster_cached_table[cluster_index] = block;
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
	vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;	
	read_sd_sectors_safe(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index >> (vram_cd->sd_info.cluster_shift - 9)]) + (cluster_index & (vram_cd->sd_info.cluster_mask >> 9)), 1/*vram_cd->sd_info.nr_sectors_per_cluster*/, &vram_cd->cluster_cache[block << 9/*vram_cd->sd_info.cluster_shift*/]);
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
#endif
#ifdef CACHE_STRATEGY_LFU
	vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 1;
#endif
	uint32_t cluster_offset = address & 0x1FF; //& vram_cd->sd_info.cluster_mask;
	void* cluster_data = (void*)&vram_cd->cluster_cache[block << 9];//vram_cd->sd_info.cluster_shift];
	return *((uint32_t*)((u8*)cluster_data + cluster_offset));
}

extern "C" ITCM_CODE uint16_t sdread16_uncached(uint32_t address)
{
	uint32_t cluster_index = address >> 9;//vram_cd->sd_info.cluster_shift;
	int block = get_new_cache_block();
	vram_cd->gba_rom_is_cluster_cached_table[cluster_index] = block;
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
	vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
	read_sd_sectors_safe(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index >> (vram_cd->sd_info.cluster_shift - 9)]) + (cluster_index & (vram_cd->sd_info.cluster_mask >> 9)), 1/*vram_cd->sd_info.nr_sectors_per_cluster*/, &vram_cd->cluster_cache[block << 9/*vram_cd->sd_info.cluster_shift*/]); 
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
#endif
#ifdef CACHE_STRATEGY_LFU
	vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 1;
#endif
	uint32_t cluster_offset = address & 0x1FF; //vram_cd->sd_info.cluster_mask;
	void* cluster_data = (void*)&vram_cd->cluster_cache[block << 9];//vram_cd->sd_info.cluster_shift];
	return *((uint16_t*)((u8*)cluster_data + cluster_offset));
}

extern "C" ITCM_CODE uint8_t sdread8_uncached(uint32_t address)
{
	uint32_t cluster_index = address >> 9;//vram_cd->sd_info.cluster_shift;
	int block = get_new_cache_block();
	vram_cd->gba_rom_is_cluster_cached_table[cluster_index] = block;
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
	vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
	read_sd_sectors_safe(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index >> (vram_cd->sd_info.cluster_shift - 9)]) + (cluster_index & (vram_cd->sd_info.cluster_mask >> 9)), 1/*vram_cd->sd_info.nr_sectors_per_cluster*/, &vram_cd->cluster_cache[block << 9/*vram_cd->sd_info.cluster_shift*/]); 
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
#endif
#ifdef CACHE_STRATEGY_LFU
	vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 1;
#endif
	uint32_t cluster_offset = address & 0x1FF; //vram_cd->sd_info.cluster_mask;
	void* cluster_data = (void*)&vram_cd->cluster_cache[block << 9];//vram_cd->sd_info.cluster_shift];
	return *((uint8_t*)((u8*)cluster_data + cluster_offset));
}

//extern "C" void copy_512(uint16_t* src, uint16_t* dst);


/*extern "C" ITCM_CODE void read_gba_rom(uint32_t address, uint32_t size, uint8_t* dst)
{
	if(address + size >= vram_cd->sd_info.gba_rom_size)
		return;
	//uint8_t* dst = vram_cd->arm9_transfer_region;
	uint32_t cluster = address >> 9;//vram_cd->sd_info.cluster_shift;
	uint32_t cluster_offset = address & 0x1FF;//vram_cd->sd_info.cluster_mask;
	uint32_t size_left = size;
	//read the part of the data that's in this cluster
	uint32_t left_in_this_cluster = (1 << /*vram_cd->sd_info.cluster_shift/9) - cluster_offset;
	if(left_in_this_cluster > size)
		left_in_this_cluster = size;
	void* cluster_data = get_cluster_data(cluster);
	//uint16_t* pDst = (uint16_t*)dst;
	//uint16_t* pSrc = (uint16_t*)((uint8_t*)cluster_data + cluster_offset);
	arm9_memcpy16((uint16_t*)dst, (uint16_t*)((uint8_t*)cluster_data + cluster_offset), left_in_this_cluster / 2);
	size_left -= left_in_this_cluster;
	if(size_left <= 0) return;
	dst += left_in_this_cluster;
	cluster++;
	//read whole clusters
	while(size_left >= 512)//(1 << vram_cd->sd_info.cluster_shift))
	{
		cluster_data = get_cluster_data(cluster++);
		//copy_512((uint16_t*)cluster_data, (uint16_t*)dst);
		arm9_memcpy16((uint16_t*)dst, (uint16_t*)cluster_data, /*(1 << vram_cd->sd_info.cluster_shift)/512 / 2);
		size_left -= 512;//1 << vram_cd->sd_info.cluster_shift;
		dst += 512;//1 << vram_cd->sd_info.cluster_shift;
	}
	if(size_left <= 0) return;
	//read data that's left
	cluster_data = get_cluster_data(cluster);
	arm9_memcpy16((uint16_t*)dst, (uint16_t*)cluster_data, size_left / 2);
}*/