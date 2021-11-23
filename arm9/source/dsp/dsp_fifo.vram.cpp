#include "vram.h"
#include "dsp.h"
#include "dsp_fifo.h"

/**
 * @brief  Sends data to the dsp memory using the fifo
 * @param  mem: Destination memory, not DSP_PCFG_MEMSEL_PROG!
 * @param  src: Source address
 * @param  fixedSrc: True if the source address is fixed
 * @param  dst: Destination address in the selected dsp memory (16 bit units!)
 * @param  fixedDst: True if the destination address is fixed
 * @param  length: Length of the transfer in 16 bit units
 * @retval None
 */
void dsp_fifoSend(DspPcfgMemsel mem, const u16* src, bool fixedSrc, u16 dst, bool fixedDst, int length)
{
    if(mem == DSP_PCFG_MEMSEL_PROG)
        return;
    dsp_spinWait();
    REG_DSP_PCFG = (REG_DSP_PCFG & ~(DSP_PCFG_MEMSEL_MASK | DSP_PCFG_AUTOINC)) | DSP_PCFG_MEMSEL(mem) | (fixedDst ? 0 : DSP_PCFG_AUTOINC);
    REG_DSP_PADR = dst;
    if(fixedSrc)
    {
        while(length-- > 0)
        {
            dsp_spinWait();
            while(REG_DSP_PSTS & DSP_PSTS_WR_FIFO_FULL);
            REG_DSP_PDATA = *src;
        }
    }
    else
    {
        while(length-- > 0)
        {
            dsp_spinWait();
            while(REG_DSP_PSTS & DSP_PSTS_WR_FIFO_FULL);
            REG_DSP_PDATA = *src++;
        }
    }
    dsp_spinWait();
    REG_DSP_PCFG &= ~(DSP_PCFG_RLEN_MASK | DSP_PCFG_AUTOINC);
}

/**
 * @brief  Receives data from dsp memory using the fifo
 * @param  mem: Source memory, not DSP_PCFG_MEMSEL_PROG!
 * @param  src: Source address in the selected dsp memory (16 bit units!)
 * @param  fixedSrc: True if the source address is fixed
 * @param  dst: Destination address
 * @param  fixedDst: True if the destination address is fixed
 * @param  length: Length of the transfer in 16 bit units
 * @param  lengthMode: Length mode of the transfer
 * @retval None
 */
void dsp_fifoRecv(DspPcfgMemsel mem, u16 src, bool fixedSrc, u16* dst, bool fixedDst, int length, DspPcfgRLen lengthMode)
{
    if(mem == DSP_PCFG_MEMSEL_PROG)
        return;
    switch(lengthMode)
    {
        case DSP_PCFG_RLEN_1:
            length = 1;
            break;
        case DSP_PCFG_RLEN_8:
            length = 8;
            break;
        case DSP_PCFG_RLEN_16:
            length = 16;
            break;
    }
    REG_DSP_PADR = src;
    dsp_spinWait();
    REG_DSP_PCFG = (REG_DSP_PCFG & ~(DSP_PCFG_MEMSEL_MASK | DSP_PCFG_RLEN_MASK | DSP_PCFG_AUTOINC)) | DSP_PCFG_MEMSEL(mem) | DSP_PCFG_RSTART | DSP_PCFG_RLEN(lengthMode) | (fixedSrc ? 0 : DSP_PCFG_AUTOINC);
    if(fixedDst)
    {
        while(length-- > 0)
        {
            dsp_spinWait();
            while(!(REG_DSP_PSTS & DSP_PSTS_RD_FIFO_READY));
            *dst = REG_DSP_PDATA;
        }
    }
    else
    {
        while(length-- > 0)
        {
            dsp_spinWait();
            while(!(REG_DSP_PSTS & DSP_PSTS_RD_FIFO_READY));
            *dst++ = REG_DSP_PDATA;
        }
    }
    dsp_spinWait();
    REG_DSP_PCFG &= ~(DSP_PCFG_RSTART | DSP_PCFG_RLEN_MASK | DSP_PCFG_AUTOINC);
}