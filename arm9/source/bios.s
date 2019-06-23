.section .vram

.global gGbaBios
gGbaBios:

.global bios_swiVeneer
bios_swiVeneer:
	ldr pc,= (gGbaBios + 0x140)

.pool

.space 0x4000 - (. - gGbaBios)