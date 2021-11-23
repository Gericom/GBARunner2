#include "vram.h"
#include "sd_access.h"
#include "dsp.h"
#include "dsp_dsp1.h"
#include "DspProcess.h"

// static DspProcess* sDspProcess = NULL;

// void DspProcess::DspIrqHandler()
// {
//     if(sDspProcess)
//         sDspProcess->HandleDspIrq();
// }

// void DspProcess::HandleDspIrq()
// {    
//     while(true)
//     {
//         u32 sources = (REG_DSP_SEM | (((REG_DSP_PSTS >> DSP_PCFG_IE_REP_SHIFT) & 7) << 16)) & _callbackSources;
//         if(!sources)
//             break;
//         while(sources)
//         {
//             int idx = MATH_CountTrailingZeros(sources);
//             sources &= ~_callbackGroups[idx];
//             _callbackFuncs[idx](_callbackArgs[idx]);
//         }
//     }
// }

DspProcess::DspProcess(bool forceDspSyncLoad)
    : _slotB(0), _slotC(0), _codeSegs(0), _dataSegs(0), _flags(forceDspSyncLoad ? DSP_PROCESS_FLAG_SYNC_LOAD : 0)
{
    for(int i = 0; i < TWR_WRAM_BC_SLOT_COUNT; i++)
    {
        _codeSlots[i] = 0xFF;
        _dataSlots[i] = 0xFF;
    }
    // for(int i = 0; i < DSP_PROCESS_CALLBACK_COUNT; i++)
    // {
    //     _callbackFuncs[i] = NULL;
    //     _callbackArgs[i] = NULL;
    //     _callbackGroups[i] = 0;
    // }
}

bool DspProcess::SetMemoryMapping(bool isCode, u32 addr, u32 len, bool toDsp)
{
    addr = DSP_MEM_ADDR_TO_CPU(addr);
    len = DSP_MEM_ADDR_TO_CPU(len < 1 ? 1 : len);
    int segBits = isCode ? _codeSegs : _dataSegs;
    int start = addr >> TWR_WRAM_BC_SLOT_SHIFT;
    int end = (addr + len - 1) >> TWR_WRAM_BC_SLOT_SHIFT;
    for(int i = start; i <= end; i++)
    {
        if(!(segBits & (1 << i)))
            continue;
        int slot = isCode ? _codeSlots[i] : _dataSlots[i];
        if(isCode)
            twr_mapWramBSlot(slot, toDsp ? TWR_WRAM_B_SLOT_MASTER_DSP_CODE : TWR_WRAM_B_SLOT_MASTER_ARM9, toDsp ? i : slot, true);
        else
            twr_mapWramCSlot(slot, toDsp ? TWR_WRAM_C_SLOT_MASTER_DSP_DATA : TWR_WRAM_C_SLOT_MASTER_ARM9, toDsp ? i : slot, true);
    }
    return true;
}

bool DspProcess::Execute()
{
    //OSIntrMode irq = OS_DisableInterrupts();
	//{
        //sDspProcess = this;
        //dsp_initPipe();
        //OS_SetIrqFunction(OS_IE_DSP, DspProcess::DspIrqHandler);
        //SetCallback(DSP_PROCESS_CALLBACK_SEMAPHORE(15) | DSP_PROCESS_CALLBACK_REPLY(DSP_PIPE_CMD_REG), dsp_pipeIrqCallback, NULL);
        //OS_EnableIrqMask(OS_IE_DSP);
		dsp_powerOn();
		dsp_setCoreResetOff(0);//_callbackSources >> 16);
		dsp_setSemaphoreMask(0);
        //SetupCallbacks();
		//needed for some modules
		if(_flags & DSP_PROCESS_FLAG_SYNC_LOAD)
			for(int i = 0; i < 3; i++)
				while(dsp_receiveData(i) != 1);
        //DspProcess::DspIrqHandler();
	//}
	//OS_RestoreInterrupts(irq);
    return true;
}

bool DspProcess::ExecuteDsp1(const dsp_dsp1_t* dsp1)
{
    // if(sDspProcess)
    //     return false;

    if(dsp1->header.magic != DSP_DSP1_MAGIC)
        return false;

    _slotB = 0xFF;//dsp1->header.memoryLayout & 0xFF;
    _slotC = 0xFF;//(dsp1->header.memoryLayout >> 8) & 0xFF;

    _codeSegs = 0xFF;
    _dataSegs = 0xFF;

    for(int i = 0; i < TWR_WRAM_BC_SLOT_COUNT; i++)
    {
        _codeSlots[i] = i;
        _dataSlots[i] = i;
        twr_mapWramBSlot(i, TWR_WRAM_B_SLOT_MASTER_ARM9, i, true);
        u32* slot = (u32*)(twr_getBlockAddress(TWR_WRAM_BLOCK_B) + i * TWR_WRAM_BC_SLOT_SIZE);
        for(int j = 0; j < (TWR_WRAM_BC_SLOT_SIZE >> 2); j++)
            *slot++ = 0;
        twr_mapWramCSlot(i, TWR_WRAM_C_SLOT_MASTER_ARM9, i, true);
        slot = (u32*)(twr_getBlockAddress(TWR_WRAM_BLOCK_C) + i * TWR_WRAM_BC_SLOT_SIZE);
        for(int j = 0; j < (TWR_WRAM_BC_SLOT_SIZE >> 2); j++)
            *slot++ = 0;
    }

    if(dsp1->header.flags & DSP_DSP1_FLAG_SYNC_LOAD)
        _flags |= DSP_PROCESS_FLAG_SYNC_LOAD;

    for(int i = 0; i < dsp1->header.nrSegments; i++)
    {
        bool isCode = dsp1->segments[i].segmentType != DSP_DSP1_SEG_TYPE_DATA;
        arm9_memcpy16((u16*)DspToArm9Address(isCode, dsp1->segments[i].address), (u16*)(((u8*)dsp1) + dsp1->segments[i].offset), (dsp1->segments[i].size + 1) >> 1);
    }

    SetMemoryMapping(true, 0, (TWR_WRAM_BC_SLOT_SIZE * TWR_WRAM_BC_SLOT_COUNT) >> 1, true);
    SetMemoryMapping(false, 0, (TWR_WRAM_BC_SLOT_SIZE * TWR_WRAM_BC_SLOT_COUNT) >> 1, true);

    return Execute();
}

// void DspProcess::SetCallback(u32 sources, dsp_process_irq_callback_t func, void* arg)
// {
//     OSIntrMode irq = OS_DisableInterrupts();
// 	{
//         for(int i = 0; i < DSP_PROCESS_CALLBACK_COUNT; i++)
//         {
//             if(!(sources & (1 << i)))
//                 continue;
//             _callbackFuncs[i] = func;
//             _callbackArgs[i] = arg;
//             _callbackGroups[i] = sources;
//         }
//         if(func)
//         {
//             REG_DSP_PCFG |= ((sources >> 16) & 7) << DSP_PCFG_IE_REP_SHIFT;
//             REG_DSP_PMASK &= ~(sources & 0xFFFF);
//             _callbackSources |= sources;
//         }
//         else
//         {
//             REG_DSP_PCFG &= ~(((sources >> 16) & 7) << DSP_PCFG_IE_REP_SHIFT);
//             REG_DSP_PMASK |= sources & 0xFFFF;
//             _callbackSources &= ~sources;
//         }        
// 	}
// 	OS_RestoreInterrupts(irq);
// }