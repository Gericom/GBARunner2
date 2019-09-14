.section .itcm
#include "consts.s"
#ifdef ENABLE_HICODE

#define PAGE_4K		(0b01011 << 1)

//r0: gba address
.global hic_mapCode
hic_mapCode:
    push {lr}
    mov r0, r0, lsr #12
    mov r1, r0, lsl #12
    //setup the pu region
    orr r12, r1, #(PAGE_4K | 1)
	mcr	p15, 0, r12, c6, c3, 0
    
    ldr r0,= (ROM_ADDRESS_MAX - 4096)
    cmp r1, r0
        ble hic_loadSectorFast

    //lock half the cache
    mov r12, #2
    mcr p15, 0, r12, c9, c0, 1

    bl hic_loadSector
    bl hic_loadSector
    bl hic_loadSector
    bl hic_loadSector
    bl hic_loadSector
    bl hic_loadSector
    bl hic_loadSector
    bl hic_loadSector
    pop {pc}

//r1: gba address
hic_loadSector:
    push {r4-r11, lr}
    mov r4, r1
    bic r0, r1, #0xFE000000
    mov r0, r0, lsr #9
    bl get_cluster_data_asm
    mov r0, r1
    and lr, r4, #0x800
    mov lr, lr, lsl #19
    mov r1, r4
    mov r2, #512
1:
    and r3, r1, #0x7E0 //index
    orr r3, lr //set
    mcr p15, 3, r3, c15, c0, 0 //set index
    //make tag
    orr r12, r1, #(1 << 4) //valid flag
    mcr p15, 3, r12, c15, c1, 0 //write tag
    ldmia r0!, {r4-r11}
    mcr p15, 3, r4, c15, c3, 0 //write data
    add r3, #(1 << 2)
    mcr p15, 3, r3, c15, c0, 0 //set index
    mcr p15, 3, r5, c15, c3, 0 //write data
    add r3, #(1 << 2)
    mcr p15, 3, r3, c15, c0, 0 //set index
    mcr p15, 3, r6, c15, c3, 0 //write data
    add r3, #(1 << 2)
    mcr p15, 3, r3, c15, c0, 0 //set index
    mcr p15, 3, r7, c15, c3, 0 //write data
    add r3, #(1 << 2)
    mcr p15, 3, r3, c15, c0, 0 //set index
    mcr p15, 3, r8, c15, c3, 0 //write data
    add r3, #(1 << 2)
    mcr p15, 3, r3, c15, c0, 0 //set index
    mcr p15, 3, r9, c15, c3, 0 //write data
    add r3, #(1 << 2)
    mcr p15, 3, r3, c15, c0, 0 //set index
    mcr p15, 3, r10, c15, c3, 0 //write data
    add r3, #(1 << 2)
    mcr p15, 3, r3, c15, c0, 0 //set index
    mcr p15, 3, r11, c15, c3, 0 //write data
    add r1, #32
    subs r2, #32
    bne 1b
    pop {r4-r11, pc}

//load from main memory
hic_loadSectorFast:
    bic r12, r1, #0x06000000
    sub r12, #0x05000000
	sub r12, #0x00FC0000
    orr r1, #(1 << 4) //valid flag

    mov r0, #0x80000000 //load bit + segment 0
    mcr p15, 0, r0, c9, c0, 1
.rept 64
    mcr p15, 0, r12, c7, c13, 1 //prefetch
    add r12, #32
    mcr p15, 3, r1, c15, c0, 0 //set index
    mcr p15, 3, r1, c15, c1, 0 //write tag
    add r1, #32
.endr
    mov r0, #0x80000001 //load bit + segment 1
    mcr p15, 0, r0, c9, c0, 1
.rept 64
    mcr p15, 0, r12, c7, c13, 1 //prefetch
    orr r3, r1, #(1 << 30)
    mcr p15, 3, r3, c15, c0, 0 //set index
    add r12, #32
    mcr p15, 3, r1, c15, c1, 0 //write tag
    add r1, #32
.endr
    mov r0, #0x00000002 //segment 2
    mcr p15, 0, r0, c9, c0, 1
    pop {pc}


#endif