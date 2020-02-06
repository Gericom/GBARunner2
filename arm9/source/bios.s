.section .vram

.global gGbaBios
gGbaBios:

.global bios_swiVeneer
bios_swiVeneer:
	b swiPatch

.space 0x4000 - (. - gGbaBios)

swiPatch:
	push {r0,r1}
	ldr r0,= gBiosOp
	ldr r1,= 0xE3A02004
	str r1, [r0]
	pop {r0, r1}
	b (gGbaBios + 0x140)

.pool