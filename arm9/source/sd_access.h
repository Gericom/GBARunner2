#ifndef __SD_ACCESS_H__
#define __SD_ACCESS_H__

#include "vram.h"
#include "../../common/sd_vram.h"
#include "consts.s"

enum KeyPadBits
{
	KEY_A = BIT(0),//!< Keypad A button.
	KEY_B = BIT(1),//!< Keypad B button.
	KEY_SELECT = BIT(2),//!< Keypad SELECT button.
	KEY_START = BIT(3),//!< Keypad START button.
	KEY_RIGHT = BIT(4),//!< Keypad RIGHT button.
	KEY_LEFT = BIT(5),//!< Keypad LEFT button.
	KEY_UP = BIT(6),//!< Keypad UP button.
	KEY_DOWN = BIT(7),//!< Keypad DOWN button.
	KEY_R = BIT(8),//!< Right shoulder button.
	KEY_L = BIT(9),//!< Left shoulder button.
	KEY_X = BIT(10),//!< Keypad X button.
	KEY_Y = BIT(11),//!< Keypad Y button.
	KEY_TOUCH = BIT(12),//!< Touchscreen pendown.
	KEY_LID = BIT(13) //!< Lid state.
};

static inline u16 keys_read()
{
	vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) + UNCACHED_OFFSET);
	return *((vu16*)0x04000130) | vramcd_uncached->extKeys;
}

#define READ_U16_SAFE(addr)		(((uint8_t*)(addr))[0] | (((uint8_t*)(addr))[1] << 8))
#define READ_U32_SAFE(addr)		(((uint8_t*)(addr))[0] | (((uint8_t*)(addr))[1] << 8) | (((uint8_t*)(addr))[2] << 16) | (((uint8_t*)(addr))[3] << 24))

static inline void MI_WriteByte(void* address, uint8_t value)
{
	u16 val = *((vu16*)((u32)address & ~1));

	if ((u32)address & 1)
		*((vu16*)((u32)address & ~1)) = (u16)(((value & 0xff) << 8) | (val & 0xff));
	else
		*((vu16*)((u32)address & ~1)) = (u16)((val & 0xff00) | (value & 0xff));
}

#define KEY_HOLD_DELAY 4
#define KEY_REPEAT_FREQ 6

extern uint8_t _io_dldi;

#ifdef ARM7_DLDI
extern "C" void read_sd_sectors_safe(sec_t sector, sec_t numSectors, void* buffer);
extern "C" void write_sd_sectors_safe(sec_t sector, sec_t numSectors, const void* buffer);
#else
#define _DLDI_readSectors_ptr ((FN_MEDIUM_READSECTORS)(*((uint32_t*)(&_io_dldi + 0x10))))
#define read_sd_sectors_safe(sector,numSectors,buffer)	_DLDI_readSectors_ptr((sector),(numSectors),(buffer))
#define write_sd_sectors_safe	((FN_MEDIUM_WRITESECTORS)(*((uint32_t*)(&_io_dldi + 0x14))))
#endif


extern "C" uint16_t *arm9_memcpy16(uint16_t *_dst, uint16_t *_src, int _count);

#endif