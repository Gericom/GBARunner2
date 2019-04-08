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

	ands r2, r1, #(1 << 6)
	bne my_irq_handler_timer3

	ands r2, r1, #(1 << 4)
	bne my_irq_handler_timer1

	ands r2, r1, #(1 << 24)
	bne my_irq_handler_wifi

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

my_irq_handler_timer1:
	str r2, [r0, #0x214]
	bl gbs_frameSeqTick
	pop {lr}
	bx lr

my_irq_handler_timer3:
	str r2, [r0, #0x214]
	bl timer3_overflow_irq
	pop {lr}
	bx lr

my_irq_handler_wifi:
	str r2, [r0, #0x214]
	bl wifi_irq
	pop {lr}
	bx lr