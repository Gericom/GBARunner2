#pragma once
#include "twlwram.h"
#include "dsp_mem.h"
#include "dsp_dsp1.h"

class DspProcess;

// typedef bool (*dsp_process_sec_callback_t)(DspProcess* process, const dsp_coff_header_t* header, const dsp_coff_section_t* section);

// #define DSP_PROCESS_CALLBACK_COUNT          (16 + 3)
// #define DSP_PROCESS_CALLBACK_SEMAPHORE(i)   (1 << (i))
// #define DSP_PROCESS_CALLBACK_REPLY(i)       (1 << ((i) + 16))
// typedef void (*dsp_process_irq_callback_t)(void* arg);

#define DSP_PROCESS_FLAG_SYNC_LOAD  1

class DspProcess
{
    u16 _slotB;
    u16 _slotC;
    int _codeSegs;
    int _dataSegs;
    u8 _codeSlots[TWR_WRAM_BC_SLOT_COUNT];
    u8 _dataSlots[TWR_WRAM_BC_SLOT_COUNT];
    u32 _flags;

    // u32 _callbackSources;
    // dsp_process_irq_callback_t _callbackFuncs[DSP_PROCESS_CALLBACK_COUNT];
    // void* _callbackArgs[DSP_PROCESS_CALLBACK_COUNT];
    // u32 _callbackGroups[DSP_PROCESS_CALLBACK_COUNT];
    
    // static void DspIrqHandler();
    // void HandleDspIrq();
    
    void* DspToArm9Address(bool isCodePtr, u32 addr)
    {
        addr = DSP_MEM_ADDR_TO_CPU(addr);
        int seg = addr >> TWR_WRAM_BC_SLOT_SHIFT;
        int offs = addr - (seg << TWR_WRAM_BC_SLOT_SHIFT);
        int slot = isCodePtr ? _codeSlots[seg] : _dataSlots[seg];
        return (u8*)twr_getBlockAddress(isCodePtr ? TWR_WRAM_BLOCK_B : TWR_WRAM_BLOCK_C) + slot * TWR_WRAM_BC_SLOT_SIZE + offs;
    }

    bool SetMemoryMapping(bool isCode, u32 addr, u32 len, bool toDsp);

    bool Execute();

protected:
    // void SetCallback(u32 sources, dsp_process_irq_callback_t func, void* arg);

    // virtual void SetupCallbacks() { }

public:
    DspProcess(bool forceDspSyncLoad = false);

    // bool ExecuteCoff(const char* path, u16 slotB, u16 slotC);
    bool ExecuteDsp1(const dsp_dsp1_t* dsp1);//const char* path);
};