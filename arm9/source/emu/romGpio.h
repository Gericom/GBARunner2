#pragma once

typedef u16 RomGpioHwMask;

#define RIO_NONE    0
#define RIO_RTC     (1 << 0)
#define RIO_LIGHT   (1 << 1)

extern u16 gRioGpioData;
extern u16 gRioGpioDirection;
extern u16 gRioGpioControl;

void rio_init(RomGpioHwMask forceHwMask);
extern "C" void rio_write(u32 addr, u16 val);
void rio_invalidate();