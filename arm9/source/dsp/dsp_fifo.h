#pragma once
#include "dsp.h"

#define REG_DSP_PDATA           (*(vu16*)0x4004300)
#define REG_DSP_PADR            (*(vu16*)0x4004304)

void dsp_fifoSend(DspPcfgMemsel mem, const u16* src, bool fixedSrc, u16 dst, bool fixedDst, int length);

static inline void dsp_fifoWriteData(const u16* src, u16 dst, int length)
{
    dsp_fifoSend(DSP_PCFG_MEMSEL_DATA, src, false, dst, false, length);
}

void dsp_fifoRecv(DspPcfgMemsel mem, u16 src, bool fixedSrc, u16* dst, bool fixedDst, int length, DspPcfgRLen lengthMode = DSP_PCFG_RLEN_FREE);

static inline void dsp_fifoReadData(u16 src, u16* dst, int length)
{
    dsp_fifoRecv(DSP_PCFG_MEMSEL_DATA, src, false, dst, false, length);
}