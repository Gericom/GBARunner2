//because libnds is stupid and doesn't work for whatever reason
//I had to write this irq code
.arm

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
	bne timer3

	//timer used for gb sound
	ands r2, r1, #(1 << 3)
	bne timer0

	//vblank
	ands r2, r1, #1
	bne vblank

	//timer used for saving
	ands r2, r1, #(1 << 4)
	bne timer1

	ands r2, r1, #(1 << 24)
	bne my_irq_handler_wifi

	pop {lr}
	bx lr

vblank:
	str r2, [r0, #0x214]
	bl irq_vblank
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

timer0:
	str r2, [r0, #0x214]
	bl gbs_frameSeqTick
	pop {lr}
	bx lr

timer1:
	str r2, [r0, #0x214]
	bl timer1_overflow_irq
	pop {lr}
	bx lr

timer3:
	str r2, [r0, #0x214]
	bl timer3_overflow_irq
	pop {lr}
	bx lr

my_irq_handler_wifi:
	str r2, [r0, #0x214]
	bl wifi_irq
	pop {lr}
	bx lr