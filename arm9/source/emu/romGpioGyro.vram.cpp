#include "sd_access.h"
#include "../../common/fifo.h"
#include "romGpio.h"
#include "romGpioGyro.h"

static u16 sGyroSample;
static u16 sGyroEdge;

void rio_gyroInit()
{
    sGyroSample = 0;
    sGyroEdge = 0;

#ifdef USE_3DS_32MB
    //enable gyro sampling
    REG_SEND_FIFO = 0xAA550101;
#endif
}

void rio_gyroUpdate()
{
#ifdef USE_3DS_32MB
    if(gRioGpioData & 1)
    {
        vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) + UNCACHED_OFFSET);
        sGyroSample = ((vramcd_uncached->gyroZ * 79) >> 12) + 0x6C0;//(vramcd_uncached->gyroZ >> 6) + 0x6C0;
    }

    if(sGyroEdge && !(gRioGpioData & 2))
    {
        u32 sampBit = sGyroSample >> 15;
        sGyroSample <<= 1;
        gRioGpioData = (gRioGpioData & gRioGpioDirection) | ((sampBit << 2) & ~gRioGpioDirection);
    }

    sGyroEdge = (gRioGpioData >> 1) & 1;
#endif
}