//because libnds is stupid and doesn't work for whatever reason
//I had to write this irq code
.arm
#include "../../common/common_defs.s"

.global my_irq_handler
my_irq_handler:
	push {lr}

	mov r0, #0x04000000
	ldr r1, [r0, #0x214]

	//ands r2, r1, #(1 << 1)
	//bne my_irq_handler_hblank

	//ands r2, r1, #(1 << 2)
	//bne my_irq_handler_vcount

	//timer used for sound
	ands r2, r1, #(1 << 6)
	bne my_irq_handler_timer3

	//timer used for gb sound
	ands r2, r1, #(1 << 3)
	bne my_irq_handler_timer0

	//ands r2, r1, #1
	//bne my_irq_handler_vblank

#ifdef USE_3DS_32MB
	ands r2, r1, #(1 << 7)
	bne my_irq_handler_rtcom
#endif

	//timer used for saving
	ands r2, r1, #(1 << 4)
	bne my_irq_handler_timer1

	pop {lr}
	bx lr

//my_irq_handler_hblank:
//	str r2, [r0, #0x214]
//	bl hblank_irq
//	pop {lr}
//	bx lr

//my_irq_handler_vcount:
//	str r2, [r0, #0x214]
//	bl vcount_irq
//	pop {lr}
//	bx lr

@ my_irq_handler_vblank:
@ 	str r2, [r0, #0x214]
@ 	bl vblankHandler
@ 	pop {lr}
@ 	bx lr

#ifdef USE_3DS_32MB
my_irq_handler_rtcom:
	str r2, [r0, #0x214]
	bl rtcomIrq
	pop {lr}
	bx lr
#endif

my_irq_handler_timer0:
	str r2, [r0, #0x214]
	bl gbs_frameSeqTick
	pop {lr}
	bx lr

my_irq_handler_timer1:
	str r2, [r0, #0x214]
	bl timer1_overflow_irq
	pop {lr}
	bx lr

my_irq_handler_timer3:
	str r2, [r0, #0x214]
	bl timer3_overflow_irq
	pop {lr}
	bx lr