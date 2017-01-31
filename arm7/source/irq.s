//because libnds is stupid and doesn't work for whatever reason
//I had to write this irq code
.arm

.global my_irq_handler
my_irq_handler:
	push {lr}

	mov r0, #0x04000000
	ldr r1, [r0, #0x214]
	ands r1, r1, #(1 << 6)
	beq my_irq_handler_end
	str r1, [r0, #0x214]
	bl timer3_overflow_irq

my_irq_handler_end:
	pop {lr}
	bx lr