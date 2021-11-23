.section .itcm

.global dsp_spinWait
dsp_spinWait:
    //the jump to here took at least 3 cycles, the jump back as well, 2 nops should do
    nop
    nop
    bx lr

//r10 = dsp address
//returns in r10
.global dsp_read16
dsp_read16:
    ldr r12,= 0x04004300
    strh r10, [r12, #4] //REG_DSP_PADR = address
    //burn 8 cycles
    nop
    b 1f
1:  nop
    b 2f
2:  
    ldrh r10, [r12, #8] //REG_DSP_PCFG
    bic r10, #0xF000 //DSP_PCFG_MEMSEL_MASK; source is data ram
    orr r10, #0x1E //DSP_PCFG_RSTART | DSP_PCFG_AUTOINC | DSP_PCFG_RLEN(DSP_PCFG_RLEN_FREE)
    strh r10, [r12, #8]

    //burn 8 cycles
    nop
    b 3f
3:  nop
    b 4f
4:  
    ldrh r10, [r12, #0xC]
    tst r10, #(1 << 6)  //DSP_PSTS_RD_FIFO_READY
    beq 4b
    ldrh r10, [r12]
    //burn 8 cycles
    nop
    b 5f
5:  nop
    b 6f
6:  
    ldrh r11, [r12, #8]
    bic r11, #0x1E   //DSP_PCFG_RSTART | DSP_PCFG_RLEN_MASK | DSP_PCFG_AUTOINC
    strh r11, [r12, #8]
    bx lr

//r10 = dsp address
//returns in r10
.global dsp_read32
dsp_read32:
    ldr r12,= 0x04004300
    strh r10, [r12, #4] //REG_DSP_PADR = address
    //burn 8 cycles
    nop
    b 1f
1:  nop
    b 2f
2:  
    ldrh r10, [r12, #8] //REG_DSP_PCFG
    bic r10, #0xF000 //DSP_PCFG_MEMSEL_MASK; source is data ram
    orr r10, #0x1E //DSP_PCFG_RSTART | DSP_PCFG_AUTOINC | DSP_PCFG_RLEN(DSP_PCFG_RLEN_FREE)
    strh r10, [r12, #8]

    //burn 8 cycles
    nop
    b 3f
3:  nop
    b 4f
4:  
    ldrh r10, [r12, #0xC]
    tst r10, #(1 << 6)  //DSP_PSTS_RD_FIFO_READY
    beq 4b
    ldrh r11, [r12]
    //burn 8 cycles
    nop
    b 5f
5:  nop
    b 6f
6:  
    ldrh r10, [r12, #0xC]
    tst r10, #(1 << 6)  //DSP_PSTS_RD_FIFO_READY
    beq 6b
    ldrh r10, [r12]
    //burn 8 cycles
    orr r10, r11, r10, lsl #16
    b 7f
7:  nop
    b 8f
8:  
    ldrh r11, [r12, #8]
    bic r11, #0x1E   //DSP_PCFG_RSTART | DSP_PCFG_RLEN_MASK | DSP_PCFG_AUTOINC
    strh r11, [r12, #8]
    bx lr

@ //r10 = dsp address
@ //r11 = value
@ //r13 = return address
@ .global dsp_writeWordR13
@ dsp_writeWordR13:
@     //burn 8 cycles
@     nop
@     b 1f
@ 1:  nop
@     b 2f
@ 2:  
@     ldr r12,= 0x04004300
@     strh r10, [r12, #4] //REG_DSP_PADR = address
@     ldrh r10, [r12, #8] //REG_DSP_PCFG
@     bic r10, #0xF000 //DSP_PCFG_MEMSEL_MASK; destination is data ram
@     orr r10, #(1 << 1) //DSP_PCFG_AUTOINC
@     strh r10, [r12, #8]

@     //burn 8 cycles
@     nop
@     b 3f
@ 3:  nop
@     b 4f
@ 4:  
@     ldrh r10, [r12, #0xC]
@     tst r10, #(1 << 7)  //DSP_PSTS_WR_FIFO_FULL
@     bne 4b
@     strh r11, [r12]
@     bx r13