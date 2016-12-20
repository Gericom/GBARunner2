#include <nds.h>
#include <string.h>
#include <sd_access.h>

#define PUT_IN_VRAM	__attribute__((section(".vram")))

extern uint8_t _io_dldi;

//FN_MEDIUM_READSECTORS _DLDI_readSectors_ptr = (FN_MEDIUM_READSECTORS)(*((uint32_t*)(&_io_dldi + 0x10)));
//extern FN_MEDIUM_WRITESECTORS _DLDI_writeSectors_ptr;

#define _DLDI_readSectors_ptr ((FN_MEDIUM_READSECTORS)(*((uint32_t*)(&_io_dldi + 0x10))))

#define vram_cd		((vram_cd_t*)0x06840000)

PUT_IN_VRAM void MI_WriteByte(void *address, uint8_t value)
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

//sd_info_t gSDInfo;

//simple means without any caching and therefore slow, but that doesn't matter for the functions that use this
PUT_IN_VRAM static uint32_t get_cluster_fat_value_simple(uint32_t cluster)
{
	uint32_t fat_offset = cluster * 4;
	uint32_t fat_sector = vram_cd->sd_info.first_fat_sector + (fat_offset >> 9); //sector_size);
	uint32_t ent_offset = fat_offset & 0x1FF;//% sector_size;
	void* tmp_buf = (void*)0x06840000;
	_DLDI_readSectors_ptr(fat_sector, 1, tmp_buf);
	return *((uint32_t*)(((uint8_t*)tmp_buf) + ent_offset)) & 0x0FFFFFFF;
}

PUT_IN_VRAM static inline uint32_t get_sector_from_cluster(uint32_t cluster)
{
	return vram_cd->sd_info.first_cluster_sector + (cluster - 2) * vram_cd->sd_info.nr_sectors_per_cluster;
}

PUT_IN_VRAM void initialize_cache()
{
	vram_cd->sd_info.access_counter = 0;
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
	//memset(&vram_cd->cluster_cache, 0, sizeof(vram_cd->cluster_cache));
	//memset(&vram_cd->gba_rom_is_cluster_cached_table, 0xFF, sizeof(vram_cd->gba_rom_is_cluster_cached_table));
	//memset(&vram_cd->cluster_cache_info, 0, sizeof(vram_cd->cluster_cache_info));
	vram_cd->cluster_cache_info.total_nr_cacheblocks = sizeof(vram_cd->cluster_cache) >> vram_cd->sd_info.cluster_shift;
	if(vram_cd->cluster_cache_info.total_nr_cacheblocks >= 256)
		vram_cd->cluster_cache_info.total_nr_cacheblocks = 255;
}

PUT_IN_VRAM void copy_bios(uint8_t* bios_dst)
{
	void* tmp_buf = (void*)0x06840000;
	uint32_t root_dir_sector = get_sector_from_cluster(vram_cd->sd_info.root_directory_cluster);
	_DLDI_readSectors_ptr(root_dir_sector, vram_cd->sd_info.nr_sectors_per_cluster, tmp_buf + 512);
	bool name_found = false;
	bool found = false;
	dir_entry_t* gba_file_entry = 0;
	uint32_t cur_cluster = vram_cd->sd_info.root_directory_cluster;
	*((vu32*)0x06202000) = 0x54524545;
	while(true)
	{
		dir_entry_t* dir_entries = (dir_entry_t*)(tmp_buf + 512);
		for(int i = 0; i < vram_cd->sd_info.nr_sectors_per_cluster * 512 / 32; i++)
		{
			dir_entry_t* cur_dir_entry = &dir_entries[i];
			if((cur_dir_entry->attrib & DIR_ATTRIB_LONG_FILENAME) == DIR_ATTRIB_LONG_FILENAME)
			{
				//construct name
				if((cur_dir_entry->long_name_entry.order & ~0x40) == 1)
				{
					if(READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[0]) == (uint16_t)'b' &&
						READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[1]) == (uint16_t)'i' &&
						READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[2]) == (uint16_t)'o' &&
						READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[3]) == (uint16_t)'s' &&
						READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[4]) == (uint16_t)'.' &&
						cur_dir_entry->long_name_entry.name_part_two[0] == (uint16_t)'b' &&
						cur_dir_entry->long_name_entry.name_part_two[1] == (uint16_t)'i' &&
						cur_dir_entry->long_name_entry.name_part_two[2] == (uint16_t)'n')
					{
						name_found = true;
					}
				}
			}
			else if(cur_dir_entry->regular_entry.record_type == 0)
			{
				*((vu32*)0x06202000) = 0x5453414C;
				//last entry
				while(1);//not found
			}
			else if(cur_dir_entry->regular_entry.record_type == 0xE5)
			{
				//erased
			}
			else
			{
				if(name_found)
				{
					*((vu32*)0x06202000) = 0x444E4946;
					gba_file_entry = cur_dir_entry;
					found = true;
					break;
				}
				else if(cur_dir_entry->regular_entry.short_name[0] == 'B' &&
					cur_dir_entry->regular_entry.short_name[1] == 'I' &&
					cur_dir_entry->regular_entry.short_name[2] == 'O' &&
					cur_dir_entry->regular_entry.short_name[3] == 'S' &&
					cur_dir_entry->regular_entry.short_name[4] == ' ' &&
					cur_dir_entry->regular_entry.short_name[5] == ' ' &&
					cur_dir_entry->regular_entry.short_name[6] == ' ' &&
					cur_dir_entry->regular_entry.short_name[7] == ' ' &&
					cur_dir_entry->regular_entry.short_name[8] == 'B' &&
					cur_dir_entry->regular_entry.short_name[9] == 'I' &&
					cur_dir_entry->regular_entry.short_name[10] == 'N')
				{
					*((vu32*)0x06202000) = 0x534F4942;
					gba_file_entry = cur_dir_entry;
					found = true;
					break;
				}
			}
		}
		if(found) break;
		//follow the chain
		uint32_t next = get_cluster_fat_value_simple(cur_cluster);
		if(next >= 0x0FFFFFF8)
		{
			*((vu32*)0x06202000) = 0x5453414C;
			while(1);//last
		}
		cur_cluster = next;
		_DLDI_readSectors_ptr(get_sector_from_cluster(cur_cluster), vram_cd->sd_info.nr_sectors_per_cluster, tmp_buf + 512);
	}
	uint32_t* cluster_table = &vram_cd->gba_rom_cluster_table[0];
	cur_cluster = gba_file_entry->regular_entry.cluster_nr_bottom | (gba_file_entry->regular_entry.cluster_nr_top << 16);
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
	*((vu32*)0x06202000) = 0x59504F43;
	while(cur_cluster < 0x0FFFFFF8 && (data_read + vram_cd->sd_info.nr_sectors_per_cluster * 512) <= data_max)
	{
		_DLDI_readSectors_ptr(get_sector_from_cluster(cur_cluster), vram_cd->sd_info.nr_sectors_per_cluster, (void*)(bios_dst + data_read));
		data_read += vram_cd->sd_info.nr_sectors_per_cluster * 512;
		cur_cluster = *cluster_table++;
	}
}

//to be called after dldi has been initialized (with the appropriate init function)
extern "C" PUT_IN_VRAM void sd_init(uint8_t* bios_dst)
{
	void* tmp_buf = (void*)0x06840000;//vram block d, mapped to the arm 7
	_DLDI_readSectors_ptr(0, 1, tmp_buf);//read mbr
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
		_DLDI_readSectors_ptr(boot_sect, 1, tmp_buf);//read boot sector
	}
	bootsect_t* bootsect = (bootsect_t*)tmp_buf;
	//we need to calculate some stuff and save that for later use
	MI_WriteByte(&vram_cd->sd_info.nr_sectors_per_cluster, bootsect->nr_sector_per_cluster);
	vram_cd->sd_info.first_fat_sector = boot_sect + bootsect->nr_reserved_sectors;
	vram_cd->sd_info.first_cluster_sector = boot_sect + bootsect->nr_reserved_sectors + (bootsect->nr_fats * bootsect->fat32_nr_sectors_per_fat);
	vram_cd->sd_info.root_directory_cluster = bootsect->fat32_root_dir_cluster;

	vram_cd->sd_info.cluster_shift = 31 - __builtin_clz(bootsect->nr_sector_per_cluster * 512);
	vram_cd->sd_info.cluster_mask = (1 << vram_cd->sd_info.cluster_shift) - 1;

	copy_bios(bios_dst);

	//we'll search for runner.gba
	uint32_t root_dir_sector = get_sector_from_cluster(vram_cd->sd_info.root_directory_cluster);
	_DLDI_readSectors_ptr(root_dir_sector, vram_cd->sd_info.nr_sectors_per_cluster, tmp_buf + 512);
	bool name_found = false;
	bool found = false;
	dir_entry_t* gba_file_entry = 0;
	uint32_t cur_cluster = vram_cd->sd_info.root_directory_cluster;
	*((vu32*)0x06202000) = 0x54524545;
	while(true)
	{
		dir_entry_t* dir_entries = (dir_entry_t*)(tmp_buf + 512);
		for(int i = 0; i < vram_cd->sd_info.nr_sectors_per_cluster * 512 / 32; i++)
		{
			dir_entry_t* cur_dir_entry = &dir_entries[i];
			if((cur_dir_entry->attrib & DIR_ATTRIB_LONG_FILENAME) == DIR_ATTRIB_LONG_FILENAME)
			{
				//construct name
				if((cur_dir_entry->long_name_entry.order & ~0x40) == 1)
				{
					if(READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[0]) == (uint16_t)'r' &&
						READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[1]) == (uint16_t)'u' &&
						READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[2]) == (uint16_t)'n' &&
						READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[3]) == (uint16_t)'n' &&
						READ_U16_SAFE(&cur_dir_entry->long_name_entry.name_part_one[4]) == (uint16_t)'e' &&
						cur_dir_entry->long_name_entry.name_part_two[0] == (uint16_t)'r' &&
						cur_dir_entry->long_name_entry.name_part_two[1] == (uint16_t)'.' &&
						cur_dir_entry->long_name_entry.name_part_two[2] == (uint16_t)'g' &&
						cur_dir_entry->long_name_entry.name_part_two[3] == (uint16_t)'b' &&
						cur_dir_entry->long_name_entry.name_part_two[4] == (uint16_t)'a')
					{
						name_found = true;
					}
				}
			}
			else if(cur_dir_entry->regular_entry.record_type == 0)
			{
				*((vu32*)0x06202000) = 0x5453414C;
				//last entry
				while(1);//not found
			}
			else if(cur_dir_entry->regular_entry.record_type == 0xE5)
			{
				//erased
			}
			else
			{
				if(name_found)
				{
					*((vu32*)0x06202000) = 0x444E4946;
					gba_file_entry = cur_dir_entry;
					found = true;
					break;
				}
				else if(cur_dir_entry->regular_entry.short_name[0] == 'R' &&
					cur_dir_entry->regular_entry.short_name[1] == 'U' &&
					cur_dir_entry->regular_entry.short_name[2] == 'N' &&
					cur_dir_entry->regular_entry.short_name[3] == 'N' &&
					cur_dir_entry->regular_entry.short_name[4] == 'E' &&
					cur_dir_entry->regular_entry.short_name[5] == 'R' &&
					cur_dir_entry->regular_entry.short_name[6] == ' ' &&
					cur_dir_entry->regular_entry.short_name[7] == ' ' &&
					cur_dir_entry->regular_entry.short_name[8] == 'G' &&
					cur_dir_entry->regular_entry.short_name[9] == 'B' &&
					cur_dir_entry->regular_entry.short_name[10] == 'A')
				{
					*((vu32*)0x06202000) = 0x444E4946;
					gba_file_entry = cur_dir_entry;
					found = true;
					break;
				}
			}
		}
		if(found) break;
		//follow the chain
		uint32_t next = get_cluster_fat_value_simple(cur_cluster);
		if(next >= 0x0FFFFFF8)
		{
			*((vu32*)0x06202000) = 0x5453414C;
			while(1);//last
		}
		cur_cluster = next;
		_DLDI_readSectors_ptr(get_sector_from_cluster(cur_cluster), vram_cd->sd_info.nr_sectors_per_cluster, tmp_buf + 512);
	}
	vram_cd->sd_info.gba_rom_size = gba_file_entry->regular_entry.file_size;
	//build the cluster table
	uint32_t* cluster_table = &vram_cd->gba_rom_cluster_table[0];
	cur_cluster = gba_file_entry->regular_entry.cluster_nr_bottom | (gba_file_entry->regular_entry.cluster_nr_top << 16);
	while(cur_cluster < 0x0FFFFFF8)
	{
		*cluster_table = cur_cluster;
		cluster_table++;
		cur_cluster = get_cluster_fat_value_simple(cur_cluster);
	}
	*cluster_table = cur_cluster;
	//copy data to main memory
	cluster_table = &vram_cd->gba_rom_cluster_table[0];
	cur_cluster = *cluster_table++;//gba_file_entry->regular_entry.cluster_nr_bottom | (gba_file_entry->regular_entry.cluster_nr_top << 16);
	uint32_t data_max = 0x3B0000;
	uint32_t data_read = 0;
	*((vu32*)0x06202000) = 0x59504F43;
	while(cur_cluster < 0x0FFFFFF8 && (data_read + vram_cd->sd_info.nr_sectors_per_cluster * 512) <= data_max)
	{
		_DLDI_readSectors_ptr(get_sector_from_cluster(cur_cluster), vram_cd->sd_info.nr_sectors_per_cluster, (void*)(0x02040000 + data_read));//tmp_buf + 512);
		data_read += vram_cd->sd_info.nr_sectors_per_cluster * 512;
		cur_cluster = *cluster_table++;//get_cluster_fat_value_simple(cur_cluster);
	}
	*((vu32*)0x06202000) = 0x20204B4F;
	initialize_cache();
}

//gets an empty one or wipes the oldest
PUT_IN_VRAM int get_new_cache_block()
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
	//wipe this old block
	MI_WriteByte(&vram_cd->gba_rom_is_cluster_cached_table[vram_cd->cluster_cache_info.cache_block_info[least_used].cluster_index], 0xFF);
	vram_cd->cluster_cache_info.cache_block_info[least_used].in_use = 0;
	vram_cd->cluster_cache_info.cache_block_info[least_used].counter = 0;
	return least_used;
}

PUT_IN_VRAM int ensure_cluster_cached(uint32_t cluster_index)
{
	int block = vram_cd->gba_rom_is_cluster_cached_table[cluster_index];
	if(block == 0xFF)
	{
		//load it
		block = get_new_cache_block();
		MI_WriteByte(&vram_cd->gba_rom_is_cluster_cached_table[cluster_index], block);
		vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
		vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
		vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
		_DLDI_readSectors_ptr(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);
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

/*void increase_cluster_cache_counters()
{
	vram_cd_t* vram_cd = (vram_cd_t*)0x06000000;
	for(int i = 0; i < vram_cd->cluster_cache_info.total_nr_cacheblocks; i++)
	{
		if(vram_cd->cluster_cache_info.cache_block_info[i].in_use)
			vram_cd->cluster_cache_info.cache_block_info[i].counter++;
	}
}*/

PUT_IN_VRAM void* get_cluster_data(uint32_t cluster_index)
{
	int block = ensure_cluster_cached(cluster_index);
	//int block = vram_cd->gba_rom_is_cluster_cached_table[cluster_index];
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
	//increase_cluster_cache_counters();
	//vram_cd->cluster_cache_info.cache_block_info[block].counter = 0;
	return (void*)&vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift];
}

/*void read_gba_rom_small(uint32_t address, uint32_t size)
{
	vram_cd_t* vram_cd = (vram_cd_t*)0x06000000;
	uint8_t* dst = vram_cd->arm9_transfer_region;
	uint32_t cluster = address >> gSDInfo.cluster_shift;
	uint32_t cluster_offset = address & gSDInfo.cluster_mask;
	void* cluster_data = get_cluster_data(cluster);
	if(size == 1)
		*dst = *((uint8_t*)(cluster_data + cluster_offset));
	else if(size == 2)
		*((uint16_t*)dst) = *((uint16_t*)(cluster_data + cluster_offset));
	else
		*((uint32_t*)dst) = *((uint32_t*)(cluster_data + cluster_offset));
}*/

extern "C" PUT_IN_VRAM uint32_t sdread32(uint32_t address)
{
	if(address >= vram_cd->sd_info.gba_rom_size)
		return 0;
	uint32_t cluster = address >> vram_cd->sd_info.cluster_shift;
	uint32_t cluster_offset = address & vram_cd->sd_info.cluster_mask;
	void* cluster_data = get_cluster_data(cluster);
	return *((uint32_t*)(cluster_data + cluster_offset));
}

extern "C" PUT_IN_VRAM uint32_t sdread32_uncached(uint32_t address)
{
	uint32_t cluster_index = address >> vram_cd->sd_info.cluster_shift;
	int block = get_new_cache_block();
	vram_cd->gba_rom_is_cluster_cached_table[cluster_index] = block;
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
	vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	_DLDI_readSectors_ptr(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
	uint32_t cluster_offset = address & vram_cd->sd_info.cluster_mask;
	void* cluster_data = (void*)&vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift];
	return *((uint32_t*)(cluster_data + cluster_offset));
}

extern "C" PUT_IN_VRAM uint32_t sdread16(uint32_t address)
{
	if(address >= vram_cd->sd_info.gba_rom_size)
		return 0;
	uint32_t cluster = address >> vram_cd->sd_info.cluster_shift;
	uint32_t cluster_offset = address & vram_cd->sd_info.cluster_mask;
	void* cluster_data = get_cluster_data(cluster);
	return *((uint16_t*)(cluster_data + cluster_offset));
}

extern "C" PUT_IN_VRAM uint32_t sdread16_uncached(uint32_t address)
{
	uint32_t cluster_index = address >> vram_cd->sd_info.cluster_shift;
	int block = get_new_cache_block();
	vram_cd->gba_rom_is_cluster_cached_table[cluster_index] = block;
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
	vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	_DLDI_readSectors_ptr(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
	uint32_t cluster_offset = address & vram_cd->sd_info.cluster_mask;
	void* cluster_data = (void*)&vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift];
	return *((uint16_t*)(cluster_data + cluster_offset));
}

extern "C" PUT_IN_VRAM uint32_t sdread8(uint32_t address)
{
	if(address >= vram_cd->sd_info.gba_rom_size)
		return 0;
	uint32_t cluster = address >> vram_cd->sd_info.cluster_shift;
	uint32_t cluster_offset = address & vram_cd->sd_info.cluster_mask;
	void* cluster_data = get_cluster_data(cluster);
	return *((uint8_t*)(cluster_data + cluster_offset));
}

extern "C" PUT_IN_VRAM uint32_t sdread8_uncached(uint32_t address)
{
	uint32_t cluster_index = address >> vram_cd->sd_info.cluster_shift;
	int block = get_new_cache_block();
	vram_cd->gba_rom_is_cluster_cached_table[cluster_index] = block;
	vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
	vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	_DLDI_readSectors_ptr(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), vram_cd->sd_info.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift]);
	vram_cd->cluster_cache_info.cache_block_info[block].counter = vram_cd->sd_info.access_counter;
	vram_cd->sd_info.access_counter++;
	uint32_t cluster_offset = address & vram_cd->sd_info.cluster_mask;
	void* cluster_data = (void*)&vram_cd->cluster_cache[block << vram_cd->sd_info.cluster_shift];
	return *((uint8_t*)(cluster_data + cluster_offset));
}


extern "C" PUT_IN_VRAM void read_gba_rom(uint32_t address, uint32_t size, uint8_t* dst)
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
	for(int i = 0; i < left_in_this_cluster / 2; i++)
	{
		((uint16_t*)dst)[i] = ((uint16_t*)((uint8_t*)cluster_data + cluster_offset))[i];
	}
	//memcpy(dst, cluster_data + cluster_offset, left_in_this_cluster);
	size_left -= left_in_this_cluster;
	if(size_left <= 0) return;
	dst += left_in_this_cluster;
	cluster++;
	//read whole clusters
	while(size_left >= (1 << vram_cd->sd_info.cluster_shift))
	{
		cluster_data = get_cluster_data(cluster++);
		for(int i = 0; i < (1 << vram_cd->sd_info.cluster_shift) / 2; i++)
		{
			((uint16_t*)dst)[i] = ((uint16_t*)cluster_data)[i];
		}
		//memcpy(dst, cluster_data, 1 << vram_cd->sd_info.cluster_shift);
		//dmaCopyWords(3, cluster_data, dst, 1 << vram_cd->sd_info.cluster_shift);
		//memcpy(dst, cluster_data, 1 << gSDInfo.cluster_shift);
		size_left -= 1 << vram_cd->sd_info.cluster_shift;
		dst += 1 << vram_cd->sd_info.cluster_shift;
	}
	if(size_left <= 0) return;
	//read data that's left
	cluster_data = get_cluster_data(cluster);
	for(int i = 0; i < size_left / 2; i++)
	{
		((uint16_t*)dst)[i] = ((uint16_t*)cluster_data)[i];
	}
	//memcpy(dst, cluster_data, size_left);
}