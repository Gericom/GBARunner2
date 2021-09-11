#ifndef __SD_VRAM_H__
#define __SD_VRAM_H__

#include "common_defs.s"

#include "sound_emu.h"
#include "save_work.h"

#include "../arm9/source/fat/ff.h"

#define vram_cd		((vram_cd_t*)sd_cluster_cache)

typedef uint32_t sec_t;

typedef bool (* FN_MEDIUM_STARTUP)(void) ;
typedef bool (* FN_MEDIUM_ISINSERTED)(void) ;
typedef bool (* FN_MEDIUM_READSECTORS)(sec_t sector, sec_t numSectors, void* buffer) ;
typedef bool (* FN_MEDIUM_WRITESECTORS)(sec_t sector, sec_t numSectors, const void* buffer) ;
typedef bool (* FN_MEDIUM_CLEARSTATUS)(void) ;
typedef bool (* FN_MEDIUM_SHUTDOWN)(void) ;

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
	uint32_t sectors_per_fat;
} sd_info_t;

typedef struct
{
	uint32_t counter	: 24;
	//uint32_t reserved	: 7;
	uint32_t counter2	: 7;
	uint32_t in_use		: 1;
	uint32_t cluster_index;
} cluster_cache_block_info_t;

typedef struct
{
	uint16_t prev;
	uint16_t next;
} cluster_cache_block_link_t;

typedef struct
{
	cluster_cache_block_info_t cache_block_info[4096];//128];//128 blocks at max seems reasonable
	cluster_cache_block_link_t cache_linked_list[4096];
	uint16_t cache_list_tail;
	uint16_t cache_list_head;
	uint32_t total_nr_cacheblocks;
} cluster_cache_info_t;

//vram config
typedef struct
{
	//vram b and c
	uint8_t cluster_cache[SD_CACHE_SIZE];//96 * 1024];
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
	uint32_t gba_rom_cluster_table[32 * 1024 / 4];//allows roms up to 32MB at 4kb clusters
	uint16_t gba_rom_is_cluster_cached_table[64 * 1024];	//allows roms up to 32MB
	cluster_cache_info_t cluster_cache_info;
	sd_info_t sd_info;
	sound_emu_work_t sound_emu_work;
	save_work_t save_work;
	vu32 openMenuIrqFlag;
	vu16 extKeys;
#ifdef USE_LOW_LATENCY_IRQ_AUDIO
	volatile u8 gbaDsndChanIrqFlags[2];
	gba_dsnd_channel_t gbaDsndChans[2] __attribute__((aligned(32)));
#endif
#ifdef USE_MP2000_PATCH
	u8 mp2000SoundArea[8192] __attribute__((aligned(32)));
#endif
	u8 tmpSector[512] __attribute__((aligned(32)));
	FATFS fatFs;
	FIL fil;
	DIR dir;
} vram_cd_t;

static_assert(MAIN_MEMORY_ADDRESS_GBARUNNER_DATA + sizeof(vram_cd_t) <= MAIN_MEMORY_ADDRESS_SAVE_DATA, "GBARunner2 data overflow");

#endif