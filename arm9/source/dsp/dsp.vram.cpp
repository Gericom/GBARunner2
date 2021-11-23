#if 1//defined(USE_DSI_16MB) || defined(USE_3DS_32MB)
#include "vram.h"
#include "scfg.h"
#include "dsp.h"

void dsp_setBlockReset(bool reset)
{
    REG_SCFG_RST = reset ? SCFG_RST_APPLY : SCFG_RST_RELEASE;
}

void dsp_setClockEnabled(bool enabled)
{
    if(enabled)
        REG_SCFG_CLK |= SCFG_CLK_DSP_ENABLE;
    else
        REG_SCFG_CLK &= ~SCFG_CLK_DSP_ENABLE;
}

void dsp_resetInterface()
{
    dsp_spinWait();
    if(!(REG_DSP_PCFG & DSP_PCFG_RESET))
        return;
    REG_DSP_PCFG &= ~(DSP_PCFG_IE_REP0 | DSP_PCFG_IE_REP1 | DSP_PCFG_IE_REP2);
    REG_DSP_PSEM = 0;
    REG_DSP_PCLEAR = 0xFFFF;
    //clear them all
    vu16 tmp = REG_DSP_REP0;
    tmp = REG_DSP_REP1;
    tmp = REG_DSP_REP2;
}

void dsp_setCoreResetOn()
{
    dsp_spinWait();
    if(REG_DSP_PCFG & DSP_PCFG_RESET)
        return;
    REG_DSP_PCFG |= DSP_PCFG_RESET;
    dsp_spinWait();
    while(REG_DSP_PSTS & DSP_PSTS_PERI_RESET);   
}

void dsp_setCoreResetOff(u16 repIrqMask)
{
    dsp_spinWait();
    while(REG_DSP_PSTS & DSP_PSTS_PERI_RESET);
    dsp_resetInterface();
    dsp_spinWait();
    REG_DSP_PCFG |= (repIrqMask & 7) << DSP_PCFG_IE_REP_SHIFT;
    dsp_spinWait();
    REG_DSP_PCFG &= ~DSP_PCFG_RESET;
}

void dsp_powerOn()
{
    dsp_setBlockReset(true);
    dsp_setClockEnabled(true);
    dsp_spinWait();
    dsp_setBlockReset(false);
    dsp_setCoreResetOn();
}

void dsp_powerOff()
{
    dsp_setBlockReset(true);
    dsp_setClockEnabled(false);
}

void dsp_sendData(int id, u16 data)
{
    dsp_spinWait();
    while(REG_DSP_PSTS & (1 << (DSP_PSTS_CMD_UNREAD_SHIFT + id)));
    (&REG_DSP_CMD0)[4 * id] = data;
}

u16 dsp_receiveData(int id)
{
    dsp_spinWait();
    while(!(REG_DSP_PSTS & (1 << (DSP_PSTS_REP_NEW_SHIFT + id))));
    return (&REG_DSP_REP0)[4 * id];
}

bool dsp_receiveDataReady(int id)
{
    dsp_spinWait();
    return (REG_DSP_PSTS & (1 << (DSP_PSTS_REP_NEW_SHIFT + id))) ? true : false;
}
#endif