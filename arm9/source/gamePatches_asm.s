.section .vram
#include "consts.s"

.thumb
.global gptc_banjoPilotFix
gptc_banjoPilotFix:
    lsr r3, r3, #2
    ldr r0,= 0x84000000
    orr r3, r0
    ldr r0,= 0x040000BC
    stmia r0!, {r1-r3}
    sub r0, #0xC
    ldr r3,= 0x030075A4
    str r3, [r0]
    ldr r3,= 0x040000A0
    str r3, [r0, #4]
    ldr r0,= 0x0800213F
    bx r0