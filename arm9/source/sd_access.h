#ifndef __SD_ACCESS_H__
#define __SD_ACCESS_H__

#include "vram.h"
#include "../../common/sd_vram.h"
#include "consts.s"

#define READ_U16_SAFE(addr)		(((uint8_t*)(addr))[0] | (((uint8_t*)(addr))[1] << 8))
#define READ_U32_SAFE(addr)		(((uint8_t*)(addr))[0] | (((uint8_t*)(addr))[1] << 8) | (((uint8_t*)(addr))[2] << 16) | (((uint8_t*)(addr))[3] << 24))

//void MI_WriteByte(void *address, uint8_t value);

static inline void MI_WriteByte(void* address, uint8_t value)
{
	u16 val = *((vu16*)((u32)address & ~1));

	if ((u32)address & 1)
		*((vu16*)((u32)address & ~1)) = (u16)(((value & 0xff) << 8) | (val & 0xff));
	else
		*((vu16*)((u32)address & ~1)) = (u16)((val & 0xff00) | (value & 0xff));
}

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

extern "C" uint16_t *arm9_memcpy16(uint16_t *_dst, uint16_t *_src, int _count);

#endif