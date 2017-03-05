#include <nds.h>
#include "vector.h"
#include "vram.h"
#include "vramheap.h"
#include "qsort.h"
#include "sd_access.h"

#include "consts.s"

#define SCREEN_COLS 32
#define SCREEN_ROWS 24
#define ENTRIES_START_ROW 2
#define ENTRIES_PER_SCREEN (SCREEN_ROWS - ENTRIES_START_ROW)
#define SKIP_ENTRIES (ENTRIES_PER_SCREEN/2 - 1)

extern uint8_t _io_dldi;

//FN_MEDIUM_READSECTORS _DLDI_readSectors_ptr = (FN_MEDIUM_READSECTORS)(*((uint32_t*)(&_io_dldi + 0x10)));
//extern FN_MEDIUM_WRITESECTORS _DLDI_writeSectors_ptr;

#define _DLDI_readSectors_ptr ((FN_MEDIUM_READSECTORS)(*((uint32_t*)(&_io_dldi + 0x10))))

#define vram_cd		((vram_cd_t*)0x06820000)

ITCM_CODE __attribute__ ((noinline)) static void MI_WriteByte(void *address, uint8_t value)
{
    uint16_t     val = *(uint16_t *)((uint32_t)address & ~1);

    if ((uint32_t)address & 1)
    {
        *(uint16_t *)((uint32_t)address & ~1) = (uint16_t)(((value & 0xff) << 8) | (val & 0xff));
    }
    else
    {
        *(uint16_t *)((uint32_t)address & ~1) = (uint16_t)((val & 0xff00) | (value & 0xff));
    }
}

PUT_IN_VRAM int strlen( char* str )
{
    char* ptr = str;
    while ( *ptr != '\0' )
        ++ptr;
    return ptr - str;
}

PUT_IN_VRAM inline int to_lower(const char* c)
{
	return (*c >= 'A' && *c <= 'Z')?(*c +'a'-'A'):*c;
}

PUT_IN_VRAM int strcasecmp(const char* one,const char* another )
{
    for ( ; to_lower(one) == to_lower(another); ++one, ++another )
        if ( *one == '\0' )
            return 0;
    return to_lower(one) - to_lower(another);
}

PUT_IN_VRAM int strcmp(const char* one,const char* another )
{
    for ( ; *one == *another; ++one, ++another )
        if ( *one == '\0' )
            return 0;
    return *one - *another;
}

//extern "C" bool read_sd_sectors_safe(sec_t sector, sec_t numSectors, void* buffer);

//after all it seems like that irq thing is not needed
#define read_sd_sectors_safe	_DLDI_readSectors_ptr

//simple means without any caching and therefore slow, but that doesn't matter for the functions that use this
PUT_IN_VRAM static uint32_t get_cluster_fat_value_simple(uint32_t cluster)
{
	uint32_t fat_offset = cluster * 4;
	uint32_t fat_sector = vram_cd->sd_info.first_fat_sector + (fat_offset >> 9); //sector_size);
	uint32_t ent_offset = fat_offset & 0x1FF;//% sector_size;
	void* tmp_buf = (void*)0x06820000;
	read_sd_sectors_safe(fat_sector, 1, tmp_buf);//_DLDI_readSectors_ptr(fat_sector, 1, tmp_buf);
	return *((uint32_t*)(((uint8_t*)tmp_buf) + ent_offset)) & 0x0FFFFFFF;
}

ITCM_CODE static inline uint32_t get_sector_from_cluster(uint32_t cluster)
{
	return vram_cd->sd_info.first_cluster_sector + (cluster - 2) * vram_cd->sd_info.nr_sectors_per_cluster;
}

PUT_IN_VRAM void initialize_cache()
{
	vram_cd->sd_info.access_counter = 0;
	//--Issue #2--
	//WATCH OUT! These 3 loops should be loops, and not calls to memset
	//Newer versions of the gcc compiler enable -ftree-loop-distribute-patterns when O3 is used
	//and replace those loops with calls to memset, which is not available anymore at the time this is called
	//Solution: Use O2 instead (which is a stupid solution in my opinion)
	for(int i = 0; i < sizeof(vram_cd->cluster_cache) / 4; i++)
	{
		((uint32_t*)&vram_cd->cluster_cache)[i] = 0;
	}
	for(int i = 0; i < sizeof(vram_cd->gba_rom_is_cluster_cached_table) / 4; i++)
	{
		((uint32_t*)&vram_cd->gba_rom_is_cluster_cached_table)[i] = 0xFFFFFFFF;
	}
	for(int i = 0; i < sizeof(vram_cd->cluster_cache_info) / 4; i++)
	{
		((uint32_t*)&vram_cd->cluster_cache_info)[i] = 0;
	}
	vram_cd->cluster_cache_info.total_nr_cacheblocks = sizeof(vram_cd->cluster_cache) >> vram_cd->sd_info.cluster_shift;
	if(vram_cd->cluster_cache_info.total_nr_cacheblocks >= 256)
		vram_cd->cluster_cache_info.total_nr_cacheblocks = 255;
}

PUT_IN_VRAM uint32_t get_entrys_first_cluster(dir_entry_t* dir_entry)
{
	uint32_t first_cluster = dir_entry->regular_entry.cluster_nr_bottom | (dir_entry->regular_entry.cluster_nr_top << 16);
	
	if(first_cluster == 0)
	{
		return vram_cd->sd_info.root_directory_cluster;
	}	
	return first_cluster;
}

PUT_IN_VRAM void store_long_name_part(uint8_t* buffer, dir_entry_t* cur_dir_entry, int position)
{	
	buffer[position + 0] = READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[0]);
	buffer[position + 1] = READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[1]);
	buffer[position + 2] = READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[2]);
	buffer[position + 3] = READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[3]);
	buffer[position + 4] = READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[4]);
		
	if(position < 26)
	{
		buffer[position + 5] = cur_dir_entry->long_name_entry.name_part_two[0];
		buffer[position + 6] = cur_dir_entry->long_name_entry.name_part_two[1];
		buffer[position + 7] = cur_dir_entry->long_name_entry.name_part_two[2];
		buffer[position + 8] = cur_dir_entry->long_name_entry.name_part_two[3];
		buffer[position + 9] = cur_dir_entry->long_name_entry.name_part_two[4];
		buffer[position + 10] = cur_dir_entry->long_name_entry.name_part_two[5];		
		buffer[position + 11] = cur_dir_entry->long_name_entry.name_part_three[0];
		buffer[position + 12] = cur_dir_entry->long_name_entry.name_part_three[1];
	}
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

PUT_IN_VRAM int comp_dir_entries(const entry_names_t* &dir1, const entry_names_t* &dir2)
{
	if(dir1->is_folder && !dir2->is_folder)
	{
		return -1;
	}
	if(!dir1->is_folder && dir2->is_folder)
	{
		return 1;
	}
	return strcasecmp(dir1->long_name, dir2->long_name);
}

PUT_IN_VRAM void get_dir_entry(uint32_t cur_dir_cluster, const char* given_short_name, dir_entry_t* result)
{	
	void* tmp_buf = (void*)0x06820000;
	uint32_t cur_dir_sector = get_sector_from_cluster(cur_dir_cluster);
	read_sd_sectors_safe(cur_dir_sector, vram_cd->sd_info.nr_sectors_per_cluster, tmp_buf + 512);
	bool last_entry = false;
	
	while(true)
	{
		dir_entry_t* dir_entries = (dir_entry_t*)(tmp_buf + 512);
		for(int i = 0; i < vram_cd->sd_info.nr_sectors_per_cluster * 512 / 32; i++)
		{
			dir_entry_t* cur_dir_entry = &dir_entries[i];
			
			if((cur_dir_entry->attrib & DIR_ATTRIB_LONG_FILENAME) == DIR_ATTRIB_LONG_FILENAME)
			{
				//skip long name entries
			}
			else if((cur_dir_entry->attrib & DIR_ATTRIB_VOLUME_ID) == DIR_ATTRIB_VOLUME_ID)
			{
				//skip VOLUME_ID entry
			}
			else if(cur_dir_entry->regular_entry.record_type == 0)
			{
				*((vu32*)0x06202000) = 0x4E464E44; //DNFN Directory not found
				last_entry = true;
				break;
			}
			else if(cur_dir_entry->regular_entry.record_type == 0xE5)
			{
				//erased
			}
			else
			{
				if( cur_dir_entry->regular_entry.short_name[0] == given_short_name[0] &&
					cur_dir_entry->regular_entry.short_name[1] == given_short_name[1] &&
					cur_dir_entry->regular_entry.short_name[2] == given_short_name[2] &&
					cur_dir_entry->regular_entry.short_name[3] == given_short_name[3] &&
					cur_dir_entry->regular_entry.short_name[4] == given_short_name[4] &&
					cur_dir_entry->regular_entry.short_name[5] == given_short_name[5] &&
					cur_dir_entry->regular_entry.short_name[6] == given_short_name[6] &&
					cur_dir_entry->regular_entry.short_name[7] == given_short_name[7] &&
					cur_dir_entry->regular_entry.short_name[8] == given_short_name[8] &&
					cur_dir_entry->regular_entry.short_name[9] == given_short_name[9] &&
					cur_dir_entry->regular_entry.short_name[10] == given_short_name[10])
				{
					arm9_memcpy16((uint16_t*)result, (uint16_t*)cur_dir_entry, sizeof(dir_entry_t) / 2);
					return;
				}	
			}
		}
		if(last_entry) break;
		//follow the chain
		uint32_t next = get_cluster_fat_value_simple(cur_dir_cluster);
		if(next >= 0x0FFFFFF8)
		{
			*((vu32*)0x06202000) = 0x5453414C; //LAST
			//while(1);//last
			break;
		}
		cur_dir_cluster = next;
		read_sd_sectors_safe(get_sector_from_cluster(cur_dir_cluster), vram_cd->sd_info.nr_sectors_per_cluster, tmp_buf + 512);
	}
	
	//return entry with root_dir_cluster
	result->regular_entry.short_name[0] = 0x00;
	result->regular_entry.cluster_nr_bottom  = 0x0000;
	result->regular_entry.cluster_nr_top = 0x0000;
}

PUT_IN_VRAM void print_folder_contents(/*std::vector<entry_names_t>*/vector& entries_names, int startRow)
{
	//print a line on second row
	*((vu64*)0x06202000 + 4 + 0) = 0xC4C4C4C4C4C4C4C4;
	*((vu64*)0x06202000 + 4 + 1) = 0xC4C4C4C4C4C4C4C4;
	*((vu64*)0x06202000 + 4 + 2) = 0xC4C4C4C4C4C4C4C4;
	*((vu64*)0x06202000 + 4 + 3) = 0xC4C4C4C4C4C4C4C4;
	
	clear_rows(2, 23);
	
	for(int i=0; i<(vector_count(&entries_names) - startRow) && i < ENTRIES_PER_SCREEN; i++)
	{
		int len = strlen(((entry_names_t*)entries_names[i + startRow])->long_name);
			
		MI_WriteByte((void*)0x06202000 + 32 * (i+ENTRIES_START_ROW) + 1, ((entry_names_t*)entries_names[i + startRow])->long_name[0]);
		
		for(int j=0; j<31 && j<len; j++)
		{
			MI_WriteByte((void*)0x06202000 + 32 * (i+ENTRIES_START_ROW) + j + 1, ((entry_names_t*)entries_names[i + startRow])->long_name[j]);
		}
	}
}

PUT_IN_VRAM void get_folder_contents(vector& entries_names, uint32_t cur_dir_cluster) 
{	
	for(int i = 0; i < vector_count(&entries_names); i++)
		vramheap_free(entries_names[i]);
	vector_free(&entries_names);

	void* tmp_buf = (void*)0x06820000;
	uint32_t cur_dir_sector = get_sector_from_cluster(cur_dir_cluster);
	read_sd_sectors_safe(cur_dir_sector, vram_cd->sd_info.nr_sectors_per_cluster, tmp_buf + 512);

	bool found_long_name = false;
	uint8_t name_buffer[32];
	for(int i = 0; i < 32; i++)
		name_buffer[i] = 0;
	
	while(true)
	{
		dir_entry_t* dir_entries = (dir_entry_t*)(tmp_buf + 512);
		for(int i = 0; i < vram_cd->sd_info.nr_sectors_per_cluster * 512 / 32; i++)
		{
			dir_entry_t* cur_dir_entry = &dir_entries[i];
			
			if((cur_dir_entry->attrib & DIR_ATTRIB_LONG_FILENAME) == DIR_ATTRIB_LONG_FILENAME)
			{
				//construct name				
				int name_part_order = cur_dir_entry->long_name_entry.order & ~0x40;
				if(name_part_order > 0 && name_part_order < 4)
				{					
					store_long_name_part(name_buffer, cur_dir_entry, (name_part_order - 1) * 13);
					if(name_part_order == 1)
					{
						found_long_name = true;
					}
				}
			}
			else if(cur_dir_entry->attrib & (DIR_ATTRIB_VOLUME_ID | 
											 DIR_ATTRIB_HIDDEN | 
											 DIR_ATTRIB_SYSTEM))
			{
				//skip VOLUME_ID, HIDDEN or SYSTEM entry
				for(int j = 0; j < 16; j++)
				{
					*(uint16_t*)(name_buffer + j*2) = 0x2020;
				}
				continue;
			}
			else if(cur_dir_entry->regular_entry.record_type == 0)
			{
				break;
			}
			else if(cur_dir_entry->regular_entry.record_type == 0xE5)
			{
				//erased
			}
			else
			{
				entry_names_t* file = (entry_names_t*)vramheap_alloc(sizeof(entry_names_t));				
				int len = 0;
				
				if(!found_long_name)
				{
					for(int j = 0; j < 8; j++)
					{
						name_buffer[j] = cur_dir_entry->regular_entry.short_name[j];
						if(name_buffer[j] != ' ')
							len = j + 1;
					}
					if(cur_dir_entry->regular_entry.short_name[8] != ' ')
					{
						name_buffer[len++] = '.';
						for(int j = 0; j < 3; j++)
						{
							if(cur_dir_entry->regular_entry.short_name[8 + j] == ' ')
								break;
							name_buffer[len++] = cur_dir_entry->regular_entry.short_name[8 + j];
						}
					}
				}
				len = 0;
				for(int i = 0; i < 32 / 2; i++)
				{
					((uint16_t*)file->long_name)[i] = ((uint16_t*)name_buffer)[i];
				}
				for(int i = 0; i < 31; i++)
				{
					if(name_buffer[i] != ' ')
						len = i + 1;
				}
				MI_WriteByte(&file->long_name[len], 0);
				
				for(int j = 0; j < 10 / 2; j++)
				{
					((uint16_t*)file->short_name)[j] = ((uint16_t*)cur_dir_entry->regular_entry.short_name)[j];
				}
				((uint16_t*)file->short_name)[5] = cur_dir_entry->regular_entry.short_name[10];
							
				if((cur_dir_entry->attrib & DIR_ATTRIB_DIRECTORY) == DIR_ATTRIB_DIRECTORY)
				{
					file->is_folder = true;
				}
				else
				{
					file->is_folder = false;
				}
				
				if(file->is_folder || !strcmp(file->short_name + 8, "GBA"))
					vector_add(&entries_names, file);
				
				
				for(int j = 0; j < 16; j++)
				{
					*(uint16_t*)(name_buffer + j*2) = 0x2020;
				}
				
				found_long_name = false;					
			}
		}		
		//follow the chain
		uint32_t next = get_cluster_fat_value_simple(cur_dir_cluster);
		if(next >= 0x0FFFFFF8)
		{
			*((vu32*)0x06202000) = 0x5453414C; //LAST
			break;
			while(1);//last
		}
		cur_dir_cluster = next;
		read_sd_sectors_safe(get_sector_from_cluster(cur_dir_cluster), vram_cd->sd_info.nr_sectors_per_cluster, tmp_buf + 512);
	}
  qsort(entries_names.data, entries_names.count, sizeof(entry_names_t*), (int (*)(const void*, const void*))comp_dir_entries);

  if(!strcmp(((entry_names_t*)entries_names[0])->short_name, ".          "))
  {
    vramheap_free(entries_names[0]);
    vector_delete(&entries_names, 0);
  }

  for(int j = 0; j<vector_count(&entries_names); j++ )
  {
    if(((entry_names_t*)entries_names[j])->is_folder)
    {
      int len = strlen(((entry_names_t*)entries_names[j])->long_name);
      for(int i = len - 1; i >= 0; i--)
      {
        MI_WriteByte(&((entry_names_t*)entries_names[j])->long_name[i + 1], ((entry_names_t*)entries_names[j])->long_name[i]);
      }
      MI_WriteByte(&((entry_names_t*)entries_names[j])->long_name[0], '\\');
      MI_WriteByte(&((entry_names_t*)entries_names[j])->long_name[len + 1], '\\');
      MI_WriteByte(&((entry_names_t*)entries_names[j])->long_name[len + 2], '\0');
    }
    int len = strlen(((entry_names_t*)entries_names[j])->long_name);
  }
}

PUT_IN_VRAM void get_game_first_cluster(uint32_t cur_dir_cluster, dir_entry_t* result)
{	
	uint16_t keys = 0;
	uint16_t old_keys = 0;
	int start_at_position = 0;
	int cursor_position = 0;
	vector entries_names;
	vector_init(&entries_names);
	
	get_folder_contents(entries_names, cur_dir_cluster);
	print_folder_contents(entries_names, start_at_position);
	
	while(1) {
		//show cursor
		MI_WriteByte((void*)0x06202000 + 32*(cursor_position - start_at_position + ENTRIES_START_ROW), 0x1A);
		
		do {
			old_keys = keys;
			keys = ~*((vu16*)0x04000130);
		} while (keys == old_keys);
		
		//hide cursor
		if(keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
		{
			MI_WriteByte((void*)0x06202000 + 32*(cursor_position - start_at_position + ENTRIES_START_ROW), ' ');
		}

		if(keys & KEY_UP)
		{
			cursor_position--;
			if(cursor_position < 0)
			{
				cursor_position = vector_count(&entries_names) - 1;
			}
		}
		else if(keys & KEY_DOWN)
		{
			cursor_position++;
			if(cursor_position > (vector_count(&entries_names) - 1))
			{
				cursor_position = 0;
			}
		}
		else if(keys & KEY_LEFT)
		{
			if((cursor_position - SKIP_ENTRIES) < 0)
			{
				if(cursor_position != 0)
				{
					cursor_position = 0;
				}
				else
				{
					cursor_position = vector_count(&entries_names) - 1;
				}			
			}
			else
			{				
				cursor_position = cursor_position - SKIP_ENTRIES;
			}
		}
		else if(keys & KEY_RIGHT)
		{
			if(cursor_position + SKIP_ENTRIES > (vector_count(&entries_names) - 1))
			{
				if(cursor_position != (vector_count(&entries_names) - 1))
				{
					cursor_position = vector_count(&entries_names) - 1;
				}
				else
				{					
					cursor_position = 0;
				}
			}
			else
			{				
				cursor_position = cursor_position + SKIP_ENTRIES;
			}
		}		
		else if (keys & KEY_A)
		{
			*((vu32*)0x06202000) = 0x44414f4c; //LOAD
			
			entry_names_t* file = (entry_names_t*)entries_names[cursor_position];
			if(file->is_folder)
			{
				dir_entry_t sel_dir_entry;
				get_dir_entry(cur_dir_cluster, file->short_name, &sel_dir_entry);
				cur_dir_cluster = get_entrys_first_cluster(&sel_dir_entry);
				
				start_at_position = 0;
				cursor_position = 0;
				get_folder_contents (entries_names, cur_dir_cluster);
				print_folder_contents (entries_names, start_at_position);
				*((vu32*)0x06202000) = 0x20202020;
				continue;
			}
			else
			{				
				dir_entry_t sel_dir_entry;
				get_dir_entry(cur_dir_cluster, file->short_name, &sel_dir_entry);
				clear_rows(2, 23);
				arm9_memcpy16((uint16_t*)result, (uint16_t*)&sel_dir_entry, sizeof(dir_entry_t) / 2);
				return;
			}
		}		
		else if(keys & KEY_B)
		{			
			*((vu32*)0x06202000) = 0x44414f4c; //LOAD		
			dir_entry_t prev_dir_entry;
			get_dir_entry(cur_dir_cluster, "..         ", &prev_dir_entry);
			if(prev_dir_entry.regular_entry.short_name[0] == 0x00)
			{
				*((vu32*)0x06202000) = 0x544f4f52; //DNFN Directory not found
			}
			cur_dir_cluster = get_entrys_first_cluster(&prev_dir_entry);
			
			start_at_position = 0;
			cursor_position = 0;
			get_folder_contents (entries_names, cur_dir_cluster);
			print_folder_contents (entries_names, start_at_position);
			*((vu32*)0x06202000) = 0x20202020;	
			continue;
		}

		if(cursor_position < start_at_position)
		{
			start_at_position = cursor_position;
		}
		else if(cursor_position > start_at_position + ENTRIES_PER_SCREEN - 1)
		{
			start_at_position = cursor_position - ENTRIES_PER_SCREEN + 1;
		}
		else
		{
			continue;
		}
		print_folder_contents(entries_names, start_at_position);
	}
}

PUT_IN_VRAM void copy_bios(uint8_t* bios_dst, uint32_t cur_cluster)
{	
	dir_entry_t bios_dir_entry;
	get_dir_entry(cur_cluster, "BIOS    BIN", &bios_dir_entry);
	cur_cluster = get_entrys_first_cluster(&bios_dir_entry);
	
	if(bios_dir_entry.regular_entry.short_name[0] == 0x00)
	{
		//look for bios in root
		get_dir_entry(cur_cluster, "BIOS    BIN", &bios_dir_entry);
		cur_cluster = get_entrys_first_cluster(&bios_dir_entry);
		
		if(bios_dir_entry.regular_entry.short_name[0] == 0x00)
		{			
			*((vu32*)0x06202000) = 0x464e4942; //BNFN BIOS not found
			while(1);
		}
	}
	
	uint32_t* cluster_table = &vram_cd->gba_rom_cluster_table[0];
	cur_cluster = bios_dir_entry.regular_entry.cluster_nr_bottom | (bios_dir_entry.regular_entry.cluster_nr_top << 16);
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
	while(cur_cluster < 0x0FFFFFF8 && (data_read + toread * 512) <= data_max)
	{
		read_sd_sectors_safe(get_sector_from_cluster(cur_cluster), toread, (void*)(bios_dst + data_read));//_DLDI_readSectors_ptr(get_sector_from_cluster(cur_cluster), vram_cd->sd_info.nr_sectors_per_cluster, (void*)(bios_dst + data_read));
		data_read += toread * 512;
		cur_cluster = *cluster_table++;
	}
	*((vu32*)0x06202000) = 0x20202020;
}

//to be called after dldi has been initialized (with the appropriate init function)
extern "C" PUT_IN_VRAM void sd_init(uint8_t* bios_dst)
{
	vramheap_init();
	void* tmp_buf = (void*)0x06820000;//vram block d, mapped to the arm 7
	read_sd_sectors_safe(0, 1, tmp_buf);//read mbr
	mbr_t* mbr = (mbr_t*)tmp_buf;
	if(mbr->signature != 0xAA55)
	{
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

	vram_cd->sd_info.cluster_shift = 31 - __builtin_clz(bootsect->nr_sector_per_cluster * 512);
	vram_cd->sd_info.cluster_mask = (1 << vram_cd->sd_info.cluster_shift) - 1;

		
	dir_entry_t gba_file_entry;
	uint32_t cur_cluster = vram_cd->sd_info.root_directory_cluster;
	
	//cd /GBA/
	dir_entry_t cur_dir_entry;
	get_dir_entry(cur_cluster, "GBA        ", &cur_dir_entry);
	cur_cluster = get_entrys_first_cluster(&cur_dir_entry);
	
	copy_bios(bios_dst, cur_cluster);
				
	get_game_first_cluster(cur_cluster, &gba_file_entry);
		
	vram_cd->sd_info.gba_rom_size = gba_file_entry.regular_entry.file_size;
	//build the cluster table
	uint32_t* cluster_table = &vram_cd->gba_rom_cluster_table[0];
	cur_cluster = gba_file_entry.regular_entry.cluster_nr_bottom | (gba_file_entry.regular_entry.cluster_nr_top << 16);
	while(cur_cluster < 0x0FFFFFF8)
	{
		*cluster_table = cur_cluster;
		cluster_table++;
		cur_cluster = get_cluster_fat_value_simple(cur_cluster);
	}
	*cluster_table = cur_cluster;
	//copy data to main memory
	cluster_table = &vram_cd->gba_rom_cluster_table[0];
	cur_cluster = *cluster_table++;//gba_file_entry.regular_entry.cluster_nr_bottom | (gba_file_entry.regular_entry.cluster_nr_top << 16);
	uint32_t data_max = 0x3B0000;
	uint32_t data_read = 0;
	*((vu32*)0x06202000) = 0x59504F43; //COPY
	while(cur_cluster < 0x0FFFFFF8 && (data_read + vram_cd->sd_info.nr_sectors_per_cluster * 512) <= data_max)
	{
		read_sd_sectors_safe(get_sector_from_cluster(cur_cluster), vram_cd->sd_info.nr_sectors_per_cluster, (void*)(0x02040000 + data_read));//_DLDI_readSectors_ptr(get_sector_from_cluster(cur_cluster), vram_cd->sd_info.nr_sectors_per_cluster, (void*)(0x02040000 + data_read));//tmp_buf + 512);
		data_read += vram_cd->sd_info.nr_sectors_per_cluster * 512;
		cur_cluster = *cluster_table++;//get_cluster_fat_value_simple(cur_cluster);
	}
	*((vu32*)0x06202000) = 0x20204B4F;
	initialize_cache();
}

//gets an empty one or wipes the oldest
ITCM_CODE int get_new_cache_block()
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
	if(!vram_cd->cluster_cache_info.cache_block_info[block].in_use)
		return block;
#endif
	//wipe this old block
	MI_WriteByte(&vram_cd->gba_rom_is_cluster_cached_table[vram_cd->cluster_cache_info.cache_block_info[block].cluster_index], 0xFF);
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 0;
	vram_cd->cluster_cache_info.cache_block_info[block].counter = 0;
	vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 0;
	return block;
}

ITCM_CODE int ensure_cluster_cached(uint32_t cluster_index)
{
	int block = vram_cd->gba_rom_is_cluster_cached_table[cluster_index];
	if(block == 0xFF)
	{
		//load it
		block = get_new_cache_block();
		MI_WriteByte(&vram_cd->gba_rom_is_cluster_cached_table[cluster_index], block);
		vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
		vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
		vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
#endif
#ifdef CACHE_STRATEGY_LFU
		vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 0;
#endif
		read_sd_sectors_safe(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);//_DLDI_readSectors_ptr(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);
	}
	//it is in cache now
	return block;
}

extern "C" PUT_IN_VRAM void ensure_next_cluster_cached(uint32_t address)
{
	if(address >= vram_cd->sd_info.gba_rom_size)
		return;
	uint32_t cluster = address >> vram_cd->sd_info.cluster_shift;
	ensure_cluster_cached(cluster + 1);
}

ITCM_CODE void* get_cluster_data(uint32_t cluster_index)
{
	int block = ensure_cluster_cached(cluster_index);
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
	return (void*)&vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift];
}

extern "C" ITCM_CODE uint32_t sdread32_uncached(uint32_t address)
{
	uint32_t cluster_index = address >> vram_cd->sd_info.cluster_shift;
	int block = get_new_cache_block();
	MI_WriteByte(&vram_cd->gba_rom_is_cluster_cached_table[cluster_index], block);
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
	vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;	
	read_sd_sectors_safe(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);//_DLDI_readSectors_ptr(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
#endif
#ifdef CACHE_STRATEGY_LFU
	vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 1;
#endif
	uint32_t cluster_offset = address & vram_cd->sd_info.cluster_mask;
	void* cluster_data = (void*)&vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift];
	return *((uint32_t*)(cluster_data + cluster_offset));
}

extern "C" ITCM_CODE uint16_t sdread16_uncached(uint32_t address)
{
	uint32_t cluster_index = address >> vram_cd->sd_info.cluster_shift;
	int block = get_new_cache_block();
	MI_WriteByte(&vram_cd->gba_rom_is_cluster_cached_table[cluster_index], block);
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
	vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
	read_sd_sectors_safe(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);//_DLDI_readSectors_ptr(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
#endif
#ifdef CACHE_STRATEGY_LFU
	vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 1;
#endif
	uint32_t cluster_offset = address & vram_cd->sd_info.cluster_mask;
	void* cluster_data = (void*)&vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift];
	return *((uint16_t*)(cluster_data + cluster_offset));
}

extern "C" ITCM_CODE uint8_t sdread8_uncached(uint32_t address)
{
	uint32_t cluster_index = address >> vram_cd->sd_info.cluster_shift;
	int block = get_new_cache_block();
	MI_WriteByte(&vram_cd->gba_rom_is_cluster_cached_table[cluster_index], block);
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
	vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
	read_sd_sectors_safe(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);//_DLDI_readSectors_ptr(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);
#if defined(CACHE_STRATEGY_LRU) || defined(CACHE_STRATEGY_LFU) || defined(CACHE_STRATEGY_MRU)
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
#endif
#ifdef CACHE_STRATEGY_LFU
	vram_cd->cluster_cache_info.cache_block_info[block].counter2 = 1;
#endif
	uint32_t cluster_offset = address & vram_cd->sd_info.cluster_mask;
	void* cluster_data = (void*)&vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift];
	return *((uint8_t*)(cluster_data + cluster_offset));
}


extern "C" ITCM_CODE void read_gba_rom(uint32_t address, uint32_t size, uint8_t* dst)
{
	if(size > sizeof(vram_cd->arm9_transfer_region) || address >= vram_cd->sd_info.gba_rom_size)
		return;
	/*if(size <= 4 && size != 3)
	{
		read_gba_rom_small(address, size);
		return;
	}*/
	//uint8_t* dst = vram_cd->arm9_transfer_region;
	uint32_t cluster = address >> vram_cd->sd_info.cluster_shift;
	uint32_t cluster_offset = address & vram_cd->sd_info.cluster_mask;
	uint32_t size_left = size;
	//read the part of the data that's in this cluster
	uint32_t left_in_this_cluster = (1 << vram_cd->sd_info.cluster_shift) - cluster_offset;
	if(left_in_this_cluster > size)
		left_in_this_cluster = size;
	void* cluster_data = get_cluster_data(cluster);
	//uint16_t* pDst = (uint16_t*)dst;
	//uint16_t* pSrc = (uint16_t*)((uint8_t*)cluster_data + cluster_offset);
	arm9_memcpy16((uint16_t*)dst, (uint16_t*)((uint8_t*)cluster_data + cluster_offset), left_in_this_cluster / 2);
	size_left -= left_in_this_cluster;
	if(size_left = 0) return;
	dst += left_in_this_cluster;
	cluster++;
	//read whole clusters
	while(size_left >= (1 << vram_cd->sd_info.cluster_shift))
	{
		cluster_data = get_cluster_data(cluster++);
		arm9_memcpy16((uint16_t*)dst, (uint16_t*)cluster_data, (1 << vram_cd->sd_info.cluster_shift) / 2);
		size_left -= 1 << vram_cd->sd_info.cluster_shift;
		dst += 1 << vram_cd->sd_info.cluster_shift;
	}
	if(size_left = 0) return;
	//read data that's left
	cluster_data = get_cluster_data(cluster);
	arm9_memcpy16((uint16_t*)dst, (uint16_t*)cluster_data, size_left / 2);
}