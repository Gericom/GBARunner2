#include <nds.h>
#include <string.h>
#include <sd_access.h>

extern FN_MEDIUM_READSECTORS _DLDI_readSectors_ptr;
extern FN_MEDIUM_WRITESECTORS _DLDI_writeSectors_ptr;

sd_info_t gSDInfo;

//simple means without any caching and therefore slow, but that doesn't matter for the functions that use this
static uint32_t get_cluster_fat_value_simple(uint32_t cluster)
{
	uint32_t fat_offset = cluster * 4;
	uint32_t fat_sector = gSDInfo.first_fat_sector + (fat_offset >> 9); //sector_size);
	uint32_t ent_offset = fat_offset & 0x1FF;//% sector_size;
	void* tmp_buf = (void*)0x06000000;
	_DLDI_readSectors_ptr(fat_sector, 1, tmp_buf);
	return *((uint32_t*)(((uint8_t*)tmp_buf) + ent_offset)) & 0x0FFFFFFF;
}

static inline uint32_t get_sector_from_cluster(uint32_t cluster)
{
	return gSDInfo.first_cluster_sector + (cluster - 2) * gSDInfo.nr_sectors_per_cluster;
}

void initialize_cache()
{
	gSDInfo.access_counter = 0;
	vram_cd_t* vram_cd = (vram_cd_t*)0x06000000;
	memset(&vram_cd->cluster_cache, 0, sizeof(vram_cd->cluster_cache));
	memset(&vram_cd->gba_rom_is_cluster_cached_table, 0xFF, sizeof(vram_cd->gba_rom_is_cluster_cached_table));
	memset(&vram_cd->reserved[0], 0, sizeof(vram_cd->reserved));
	vram_cd->cluster_cache_info.total_nr_cacheblocks = sizeof(vram_cd->cluster_cache) >> gSDInfo.cluster_shift;
}

//to be called after dldi has been initialized (with the appropriate init function)
extern "C" void sd_init()
{
	void* tmp_buf = (void*)0x06000000;//vram block d, mapped to the arm 7
	_DLDI_readSectors_ptr(0, 1, tmp_buf);//read mbr
	mbr_t* mbr = (mbr_t*)tmp_buf;
	if(mbr->signature != 0xAA55)
	{
		while(1);
	}
	sec_t boot_sect = 0;
	if(mbr->non_usefull_stuff[2] != 0x90)
	{
		if(mbr->partitions[0].partition_type != MBR_PARTITION_TYPE_FAT32)
		{
			while(1);
		}
		boot_sect = mbr->partitions[0].lba_partition_start;
		_DLDI_readSectors_ptr(boot_sect, 1, tmp_buf);//read boot sector
	}
	bootsect_t* bootsect = (bootsect_t*)tmp_buf;
	//we need to calculate some stuff and save that for later use
	gSDInfo.nr_sectors_per_cluster = bootsect->nr_sector_per_cluster;
	gSDInfo.first_fat_sector = boot_sect + bootsect->nr_reserved_sectors;
	gSDInfo.first_cluster_sector = boot_sect + bootsect->nr_reserved_sectors + (bootsect->nr_fats * bootsect->fat32_nr_sectors_per_fat);
	gSDInfo.root_directory_cluster = bootsect->fat32_root_dir_cluster;

	gSDInfo.cluster_shift = 31 - __builtin_clz(bootsect->nr_sector_per_cluster * 512);
	gSDInfo.cluster_mask = (1 << gSDInfo.cluster_shift) - 1;
	//we'll search for runner.gba
	uint32_t root_dir_sector = get_sector_from_cluster(gSDInfo.root_directory_cluster);
	_DLDI_readSectors_ptr(root_dir_sector, gSDInfo.nr_sectors_per_cluster, tmp_buf + 512);
	bool name_found = false;
	bool found = false;
	dir_entry_t* gba_file_entry = 0;
	uint32_t cur_cluster = gSDInfo.root_directory_cluster;
	while(true)
	{
		dir_entry_t* dir_entries = (dir_entry_t*)(tmp_buf + 512);
		for(int i = 0; i < gSDInfo.nr_sectors_per_cluster * 512 / 32; i++)
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
				*((vu32*)0x04000188) = 0x5453414C;
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
					*((vu32*)0x04000188) = 0x444E4946;
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
					*((vu32*)0x04000188) = 0x444E4946;
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
			*((vu32*)0x04000188) = 0x5453414C;
			while(1);//last
		}
		cur_cluster = next;
		_DLDI_readSectors_ptr(get_sector_from_cluster(cur_cluster), gSDInfo.nr_sectors_per_cluster, tmp_buf + 512);
	}
	gSDInfo.gba_rom_size = gba_file_entry->regular_entry.file_size;
	//build the cluster table
	vram_cd_t* vram_cd = (vram_cd_t*)0x06000000;
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
	while(cur_cluster < 0x0FFFFFF8 && (data_read + gSDInfo.nr_sectors_per_cluster * 512) < data_max)
	{
		*((vu32*)0x04000188) = 0x59504F43;
		_DLDI_readSectors_ptr(get_sector_from_cluster(cur_cluster), gSDInfo.nr_sectors_per_cluster, (void*)(0x02040000 + data_read));//tmp_buf + 512);
		data_read += gSDInfo.nr_sectors_per_cluster * 512;
		cur_cluster = *cluster_table++;//get_cluster_fat_value_simple(cur_cluster);
	}
	*((vu32*)0x04000188) = 0x20204B4F;
	initialize_cache();
}

//gets an empty one or wipes the oldest
int get_new_cache_block()
{
	vram_cd_t* vram_cd = (vram_cd_t*)0x06000000;
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
	vram_cd->gba_rom_is_cluster_cached_table[vram_cd->cluster_cache_info.cache_block_info[least_used].cluster_index] = 0xFF;
	vram_cd->cluster_cache_info.cache_block_info[least_used].in_use = 0;
	vram_cd->cluster_cache_info.cache_block_info[least_used].counter = 0;
	return least_used;
}

int ensure_cluster_cached(uint32_t cluster_index)
{
	vram_cd_t* vram_cd = (vram_cd_t*)0x06000000;
	int block = vram_cd->gba_rom_is_cluster_cached_table[cluster_index];
	if(block == 0xFF)
	{
		//load it
		block = get_new_cache_block();
		vram_cd->gba_rom_is_cluster_cached_table[cluster_index] = block;
		vram_cd->cluster_cache_info.cache_block_info[block].in_use = 1;
		vram_cd->cluster_cache_info.cache_block_info[block].cluster_index = cluster_index;
		vram_cd->cluster_cache_info.cache_block_info[block].counter = gSDInfo.access_counter;
		_DLDI_readSectors_ptr(get_sector_from_cluster(vram_cd->gba_rom_cluster_table[cluster_index]), gSDInfo.nr_sectors_per_cluster, &vram_cd->cluster_cache[block << gSDInfo.cluster_shift]);
	}
	//it is in cache now
	return block;
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

void* get_cluster_data(uint32_t cluster_index)
{
	int block = ensure_cluster_cached(cluster_index);
	vram_cd_t* vram_cd = (vram_cd_t*)0x06000000;
	//int block = vram_cd->gba_rom_is_cluster_cached_table[cluster_index];
	vram_cd->cluster_cache_info.cache_block_info[block].counter = gSDInfo.access_counter;
	gSDInfo.access_counter++;
	//increase_cluster_cache_counters();
	//vram_cd->cluster_cache_info.cache_block_info[block].counter = 0;
	return (void*)&vram_cd->cluster_cache[block << gSDInfo.cluster_shift];
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

extern "C" void read_gba_rom(uint32_t address, uint32_t size)
{
	vram_cd_t* vram_cd = (vram_cd_t*)0x06000000;
	if(size > sizeof(vram_cd->arm9_transfer_region) || address >= gSDInfo.gba_rom_size)
		return;
	/*if(size <= 4 && size != 3)
	{
		read_gba_rom_small(address, size);
		return;
	}*/
	uint8_t* dst = vram_cd->arm9_transfer_region;
	uint32_t cluster = address >> gSDInfo.cluster_shift;
	uint32_t cluster_offset = address & gSDInfo.cluster_mask;
	uint32_t size_left = size;
	//read the part of the data that's in this cluster
	uint32_t left_in_this_cluster = (1 << gSDInfo.cluster_shift) - cluster_offset;
	if(left_in_this_cluster > size)
		left_in_this_cluster = size;
	void* cluster_data = get_cluster_data(cluster);
	memcpy(dst, cluster_data + cluster_offset, left_in_this_cluster);
	size_left -= left_in_this_cluster;
	if(size_left <= 0) return;
	dst += left_in_this_cluster;
	cluster++;
	//read whole clusters
	while(size_left >= (1 << gSDInfo.cluster_shift))
	{
		cluster_data = get_cluster_data(cluster++);
		dmaCopyWords(3, cluster_data, dst, 1 << gSDInfo.cluster_shift);
		//memcpy(dst, cluster_data, 1 << gSDInfo.cluster_shift);
		size_left -= 1 << gSDInfo.cluster_shift;
		dst += 1 << gSDInfo.cluster_shift;
	}
	if(size_left <= 0) return;
	//read data that's left
	cluster_data = get_cluster_data(cluster);
	memcpy(dst, cluster_data, size_left);
}