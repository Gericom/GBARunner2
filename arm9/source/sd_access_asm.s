.section .vram

/*.global read_sd_sectors_safe
read_sd_sectors_safe:
	push {r4,r5,lr}
	mrs r4, cpsr
	ldr r3,= 0x04000000
	ldr r5, [r3, #0x208]
	str r3, [r3, #0x208]
	ldr r12,= (_io_dldi + 0x10)
	ldr r12, [r12]
	blx r12
	msr cpsr_c, r4
	ldr r3,= 0x04000000
	str r5, [r3, #0x208]
	pop {r4,r5,lr}
	bx lr*/
