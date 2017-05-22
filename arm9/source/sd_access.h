#ifndef __SD_ACCESS_H__
#define __SD_ACCESS_H__

#include "../../common/sd_vram.h"
#include "consts.s"

#define READ_U16_SAFE(addr)		(((uint8_t*)(addr))[0] | (((uint8_t*)(addr))[1] << 8))
#define READ_U32_SAFE(addr)		(((uint8_t*)(addr))[0] | (((uint8_t*)(addr))[1] << 8) | (((uint8_t*)(addr))[2] << 16) | (((uint8_t*)(addr))[3] << 24))

#define vram_cd		((vram_cd_t*)sd_cluster_cache)

#define SCREEN_COLS 32
#define SCREEN_ROWS 24
#define ENTRIES_START_ROW 2
#define ENTRIES_PER_SCREEN (SCREEN_ROWS - ENTRIES_START_ROW)
#define SKIP_ENTRIES (ENTRIES_PER_SCREEN/2 - 1)
#define KEY_HOLD_DELAY 4
#define KEY_REPEAT_FREQ 6

extern uint8_t _io_dldi;

//FN_MEDIUM_READSECTORS _DLDI_readSectors_ptr = (FN_MEDIUM_READSECTORS)(*((uint32_t*)(&_io_dldi + 0x10)));
//extern FN_MEDIUM_WRITESECTORS _DLDI_writeSectors_ptr;

#ifdef ARM7_DLDI
extern "C" void read_sd_sectors_safe(sec_t sector, sec_t numSectors, void* buffer);
#else
#define _DLDI_readSectors_ptr ((FN_MEDIUM_READSECTORS)(*((uint32_t*)(&_io_dldi + 0x10))))
#define read_sd_sectors_safe(sector,numSectors,buffer)	_DLDI_readSectors_ptr((sector),(numSectors),(buffer))
#endif

#define write_sd_sectors_safe	((FN_MEDIUM_WRITESECTORS)(*((uint32_t*)(&_io_dldi + 0x14))))

//extern sd_info_t gSDInfo;

extern "C" uint16_t *arm9_memcpy16(uint16_t *_dst, uint16_t *_src, size_t _count);

uint32_t get_cluster_fat_value_simple(uint32_t cluster);

ITCM_CODE static inline uint32_t get_sector_from_cluster(uint32_t cluster)
{
	return vram_cd->sd_info.first_cluster_sector + (cluster - 2) * vram_cd->sd_info.nr_sectors_per_cluster;
}

#endif