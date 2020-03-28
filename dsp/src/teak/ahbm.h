#pragma once

#define AHBM_CHAN_CONFIG1_BURST_SINGLE      0
#define AHBM_CHAN_CONFIG1_BURST_INCR        1
#define AHBM_CHAN_CONFIG1_BURST_WRAP4       2
#define AHBM_CHAN_CONFIG1_BURST_INCR4       3
#define AHBM_CHAN_CONFIG1_BURST_WRAP8       4
#define AHBM_CHAN_CONFIG1_BURST_INCR8       5
//WRAP16 and INCR16 are not supported or so and act like INCR mode

//bit 3 hangs if no burst is used

#define AHBM_CHAN_CONFIG1_BURST(x)          (x)

#define AHBM_CHAN_CONFIG1_SIZE_8BIT         0
#define AHBM_CHAN_CONFIG1_SIZE_16BIT        1
#define AHBM_CHAN_CONFIG1_SIZE_32BIT        2
//bit 6 is probably the msb of the HSIZE signal, but since the bus
//is only 32 bit, that bit is ignored

#define AHBM_CHAN_CONFIG1_SIZE(x)           ((x) << 4)

//bits 8-11 may be HPROT, which is most likely unused on the dsi

void ahbm_configChannel(int channel, u16 a, u16 b, u16 dmaChannelMask);
void ahbm_resetChannel(int channel);