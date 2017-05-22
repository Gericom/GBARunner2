#ifndef __COMMON_DEFS_H__
#define __COMMON_DEFS_H__

//#define ARM7_DLDI

#define SD_CACHE_SIZE	(1424 * 1024)

#define MAIN_MEMORY_ADDRESS_ROM_DATA		0x02040000
#define MAIN_MEMORY_ADDRESS_GBARUNNER_DATA	0x02240000
#define ROM_DATA_LENGTH						(MAIN_MEMORY_ADDRESS_GBARUNNER_DATA - MAIN_MEMORY_ADDRESS_ROM_DATA)
#define ROM_ADDRESS_MAX						(0x08000000 + ROM_DATA_LENGTH)

#define sd_cluster_cache  (MAIN_MEMORY_ADDRESS_GBARUNNER_DATA) //0x06820000

#define sd_data_base (sd_cluster_cache + SD_CACHE_SIZE) //0x06840000
#define sd_is_cluster_cached_table (sd_data_base + (32 * 1024)) //(96 * 1024))
#define sd_cluster_cache_info (sd_is_cluster_cached_table + (2 * 64 * 1024))
#define sd_cluster_cache_linked_list (sd_cluster_cache_info + (4096 * 8))
#define sd_sd_info (sd_cluster_cache_linked_list + (4096 * 4 + 4 + 4)) //0x0685C404

#define CACHE_LINKED_LIST_NIL	4096 //0x8000

#endif