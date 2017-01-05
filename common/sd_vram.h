#ifndef __SD_VRAM_H__
#define __SD_VRAM_H__

typedef uint32_t sec_t;

typedef struct
{
	uint32_t gba_rom_size;
	uint32_t cluster_shift;
	uint32_t cluster_mask;
	uint32_t access_counter;

	uint8_t nr_sectors_per_cluster;
	sec_t first_fat_sector;
	sec_t first_cluster_sector;
	uint32_t root_directory_cluster;
} sd_info_t;

typedef struct
{
	uint32_t counter	: 24;
	uint32_t reserved	: 7;
	uint32_t in_use		: 1;
	uint32_t cluster_index;
} cluster_cache_block_info_t;

typedef struct
{
	cluster_cache_block_info_t cache_block_info[256];//128];//128 blocks at max seems reasonable
	uint32_t total_nr_cacheblocks;
} cluster_cache_info_t;

//vram config
typedef struct
{
	//vram c
	uint8_t cluster_cache[128 * 1024];//96 * 1024];
	/*uint8_t gba_rom_is_cluster_cached_table[16 * 1024];	//allows roms up to 64MB
	union
	{
		uint8_t reserved[16 * 1024];//will be used for memory management of the cached clusters
		struct
		{
			cluster_cache_info_t cluster_cache_info;
			sd_info_t sd_info;
		};
	};*/
	//vram d
	uint32_t gba_rom_cluster_table[32 * 1024 / 4];//allows roms up to 32MB
	uint8_t arm9_transfer_region[64 * 1024];//contains data requested by the arm9
	//uint8_t dldi_region[32 * 1024];//contains the dldi code
	uint8_t gba_rom_is_cluster_cached_table[16 * 1024];	//allows roms up to 64MB
	union
	{
		uint8_t reserved[16 * 1024];//will be used for memory management of the cached clusters
		struct
		{
			cluster_cache_info_t cluster_cache_info;
			sd_info_t sd_info;
		};
	};
} vram_cd_t;

#endif