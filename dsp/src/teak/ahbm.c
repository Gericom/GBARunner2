#include "teak.h"
#include "ahbm.h"

void ahbm_configChannel(int channel, u16 a, u16 b, u16 dmaChannelMask)
{
    *(vu16*)(0x80E2 + channel * 6) = a;
    *(vu16*)(0x80E4 + channel * 6) = b;
    *(vu16*)(0x80E6 + channel * 6) = dmaChannelMask;
}

void ahbm_resetChannel(int channel)
{
    *(vu16*)(0x80E6 + channel * 6) = 0;
}