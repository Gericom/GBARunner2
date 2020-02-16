.section .itcm

#include "consts.s"

//I hate repeating code

.global inp_readGbaKeyinput32
inp_readGbaKeyinput32:
    ldr r12,= extKeys_uncached
    ldrh r11, [r9]
    ldrh r12, [r12]
    and r10, r11, #0xF0 //dpad never remapped
    mov r10, r10, lsl #16
    orr r11, r12
    ldr r12,= gInputSettings
    orr r10, #1

    ldr r13, [r12]
    tst r11, r10, lsl r13
    ldr r13, [r12, #4]
    orrne r10, #(1 << (0 + 16))

    tst r11, r10, lsl r13
    ldr r13, [r12, #8]
    orrne r10, #(1 << (1 + 16))
    
    tst r11, r10, lsl r13
    ldr r13, [r12, #0xC]
    orrne r10, #(1 << (9 + 16))

    tst r11, r10, lsl r13
    ldr r13, [r12, #0x10]
    orrne r10, #(1 << (8 + 16))

    tst r11, r10, lsl r13
    ldr r13, [r12, #0x14]
    orrne r10, #(1 << (3 + 16))

    tst r11, r10, lsl r13
    orrne r10, #(1 << (2 + 16))

    mov r10, r10, lsr #16

    ldrh r11, [r9, #2]
    orr r10, r10, r11, lsl #16

    bx lr

.global inp_readGbaKeyinput16
inp_readGbaKeyinput16:
    ldr r12,= extKeys_uncached
    ldrh r11, [r9]
    ldrh r12, [r12]
    and r10, r11, #0xF0 //dpad never remapped
    mov r10, r10, lsl #16
    orr r11, r12
    ldr r12,= gInputSettings
    orr r10, #1

    ldr r13, [r12]
    tst r11, r10, lsl r13
    ldr r13, [r12, #4]
    orrne r10, #(1 << (0 + 16))

    tst r11, r10, lsl r13
    ldr r13, [r12, #8]
    orrne r10, #(1 << (1 + 16))
    
    tst r11, r10, lsl r13
    ldr r13, [r12, #0xC]
    orrne r10, #(1 << (9 + 16))

    tst r11, r10, lsl r13
    ldr r13, [r12, #0x10]
    orrne r10, #(1 << (8 + 16))

    tst r11, r10, lsl r13
    ldr r13, [r12, #0x14]
    orrne r10, #(1 << (3 + 16))

    tst r11, r10, lsl r13
    orrne r10, #(1 << (2 + 16))

    mov r10, r10, lsr #16

    bx lr


.global inp_readGbaKeyinput8lo
inp_readGbaKeyinput8lo:
    ldr r12,= extKeys_uncached
    ldrh r11, [r9]
    ldrh r12, [r12]
    and r10, r11, #0xF0 //dpad never remapped
    mov r10, r10, lsl #16
    orr r11, r12
    ldr r12,= gInputSettings
    orr r10, #1

    ldr r13, [r12]
    tst r11, r10, lsl r13
    ldr r13, [r12, #4]
    orrne r10, #(1 << (0 + 16))

    tst r11, r10, lsl r13
    ldr r13, [r12, #0x10]
    orrne r10, #(1 << (1 + 16))

    tst r11, r10, lsl r13
    ldr r13, [r12, #0x14]
    orrne r10, #(1 << (3 + 16))

    tst r11, r10, lsl r13
    orrne r10, #(1 << (2 + 16))

    mov r10, r10, lsr #16

    bx lr


.global inp_readGbaKeyinput8hi
inp_readGbaKeyinput8hi:
    ldr r12,= extKeys_uncached
    ldrh r11, [r9]
    ldrh r12, [r12]
    orr r11, r12
    ldr r12,= gInputSettings
    mov r10, #1

    ldr r13, [r12, #0xC]
    tst r11, r10, lsl r13
    ldr r13, [r12, #0x10]
    orrne r10, #(1 << (9 + 16))

    tst r11, r10, lsl r13
    orrne r10, #(1 << (8 + 16))

    mov r10, r10, lsr #24

    bx lr