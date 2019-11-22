#include "vram.h"
#include "consts.s"
#include "romGpioRtc.h"
#include "romGpio.h"

struct game_hw_info_t
{
	u32 gameCode;
	RomGpioHwMask hardware;
};

#define GAMECODE(x)		((((x) & 0xFF) << 24) | ((((x) >> 8) & 0xFF) << 16) | ((((x) >> 16) & 0xFF) << 8) | ((x) >> 24))

static game_hw_info_t sGameHardwareTable[] =
{
	// Boktai: The Sun is in Your Hand
	{ GAMECODE('U3IJ'), RIO_RTC | RIO_LIGHT },
	{ GAMECODE('U3IE'), RIO_RTC | RIO_LIGHT },
	{ GAMECODE('U3IP'), RIO_RTC | RIO_LIGHT },

	// Boktai 2: Solar Boy Django
	{ GAMECODE('U32J'), RIO_RTC | RIO_LIGHT },
	{ GAMECODE('U32E'), RIO_RTC | RIO_LIGHT },
	{ GAMECODE('U32P'), RIO_RTC | RIO_LIGHT },

	// Pokemon Ruby
	{ GAMECODE('AXVJ'), RIO_RTC },
	{ GAMECODE('AXVE'), RIO_RTC },
	{ GAMECODE('AXVP'), RIO_RTC },
	{ GAMECODE('AXVI'), RIO_RTC },
	{ GAMECODE('AXVS'), RIO_RTC },
	{ GAMECODE('AXVD'), RIO_RTC },
	{ GAMECODE('AXVF'), RIO_RTC },

	// Pokemon Sapphire
	{ GAMECODE('AXPJ'), RIO_RTC },
	{ GAMECODE('AXPE'), RIO_RTC },
	{ GAMECODE('AXPP'), RIO_RTC },
	{ GAMECODE('AXPI'), RIO_RTC },
	{ GAMECODE('AXPS'), RIO_RTC },
	{ GAMECODE('AXPD'), RIO_RTC },
	{ GAMECODE('AXPF'), RIO_RTC },

	// Pokemon Emerald
	{ GAMECODE('BPEJ'), RIO_RTC },
	{ GAMECODE('BPEE'), RIO_RTC },
	{ GAMECODE('BPEP'), RIO_RTC },
	{ GAMECODE('BPEI'), RIO_RTC },
	{ GAMECODE('BPES'), RIO_RTC },
	{ GAMECODE('BPED'), RIO_RTC },
	{ GAMECODE('BPEF'), RIO_RTC },

	// RockMan EXE 4.5 - Real Operation
	{ GAMECODE('BR4J'), RIO_RTC },

	// Sennen Kazoku
	{ GAMECODE('BKAJ'), RIO_RTC },

	// Shin Bokura no Taiyou: Gyakushuu no Sabata
	{ GAMECODE('U33J'), RIO_RTC | RIO_LIGHT },
};

#define RIO_REG_DATA        0xC4
#define RIO_REG_DIRECTION   0xC6
#define RIO_REG_CONTROL     0xC8

static RomGpioHwMask sGpioHwMask;

u16 gRioGpioData;
u16 gRioGpioDirection;
u16 gRioGpioControl;

void rio_init(RomGpioHwMask forceHwMask)
{
    sGpioHwMask = forceHwMask;
	u32 gameCode = *(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xAC);
	for(int i = 0; i < sizeof(sGameHardwareTable) / sizeof(sGameHardwareTable[0]); i++)
	{
		if(sGameHardwareTable[i].gameCode == gameCode)
		{
			sGpioHwMask = sGameHardwareTable[i].hardware | forceHwMask;
			break;
		}
	}
    gRioGpioData = 0;
    gRioGpioDirection = 0;
    gRioGpioControl = 0;
    if(sGpioHwMask & RIO_RTC)
        rio_rtcInit();
}

static void updateHardware()
{
    if(sGpioHwMask & RIO_RTC)
        rio_rtcUpdate();
}

extern "C" void rio_write(u32 addr, u16 val)
{
    switch(addr - 0x08000000)
    {
        case RIO_REG_DATA:
            gRioGpioData = (gRioGpioData & ~gRioGpioDirection) | (val & gRioGpioDirection);
            updateHardware();
            break;
        case RIO_REG_DIRECTION:
            gRioGpioDirection = val & 0xF;
            break;
        case RIO_REG_CONTROL:
            gRioGpioControl = val & 1;
            break;
    }  
    rio_invalidate();
}

void rio_invalidate()
{
    if(gRioGpioControl)
    {
        *(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + RIO_REG_DATA) = gRioGpioData;
        *(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + RIO_REG_DIRECTION) = gRioGpioDirection;
        *(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + RIO_REG_CONTROL) = gRioGpioControl;
    }
    else
    {
        *(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + RIO_REG_DATA) = 0;
        *(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + RIO_REG_DIRECTION) = 0;
        *(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + RIO_REG_CONTROL) = 0;
    }
}