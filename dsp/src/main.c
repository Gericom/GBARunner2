#include "teak/teak.h"
#include "gbaAudio.h"
#include "ipc.h"

int main()
{
    icu_init();
    dma_init();

    *(vu16*)0x0600 = 0;

    while(*(vu16*)0x80E0);

    gbaa_init();

    ipc_init();
    
    gbaa_start();

    //u32 addr = 0x02022F00;
    //int val = 0;
    while(1)
    {
        // if((REG_DMA_START & (1 << 5)) == 0)
        // {
        //     cpu_disableIrqs();
        //     gbaa_updateDma();
        //     cpu_enableIrqs();
        // }
        // if(REG_BTDMP_TRANSMIT_FIFO_STAT(0) & BTDMP_TRANSMIT_FIFO_STAT_FULL)
        //     continue;
        // cpu_disableIrqs();
        // gbaa_updateMixer();
        // cpu_enableIrqs();
        // //*(vu16*)0x0600 = /*REG_TMR_COUNTER_LO(0);//*/REG_ICU_IRQ_PENDING;//addr & 0xFFFF;
        // *(vu16*)0x0602 = REG_TMR_CONTROL(0);
        // *(vu16*)0x0603 = REG_TMR_COUNTER_LO(0);
        // *(vu16*)0x0604 = REG_TMR_COUNTER_HI(0);
        // if(!(addr & 1))
        // {
        //     dma_transferArm9ToDsp(addr, (void*)0x0601, 1);
        //     val = *(vs16*)0x0601;
        // }        
        // addr++;
        // REG_BTDMP_TRANSMIT_FIFO_DATA(0) = val << 8;
        // val >>= 8;
    }

    // u16 val = 0;
    // while(1)
    // {
    //     dma_transferArm9ToDsp(0x04000130, (void*)0x0601, 1);
    //     if(((*(vu16*)0x0601) & 1) == 0)
    //         setColor(val++);
    //     for(int i = 166666; i >= 0; i--)
    //         asm("nop");
    // }

    // while(1);

    // // while(((*(vu16*)0x818C) & 0x80) == 0);
    // // u16 color = 0;
    // // while(1)
    // // {
    // //     *(vu16*)0x0600 = color;
    // //     dma_transferDspToArm9((const void*)0x0600, 0x05000000, 1);
    // //     // for(int i = 1666666; i >= 0; i--)
    // //     // {
    // //     //     asm("nop");
    // //     // }
    // //     color++;
    // // }
    // //*(vu16*)0x0600 = 0x1F;
    // setColor(0x1F);
    // setColor(0x3F);
    // while(1);
    // //setColor(0x3E0);
    // *(vu16*)0x0600 = 123;
    // while(1);
    return 0;
}