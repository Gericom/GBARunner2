.section .vram

.global gGbaBios
gGbaBios:

.global bios_swiVeneer
bios_swiVeneer:
	b (gGbaBios + 0x140)
	//b swiPatch

.space 0x4000 - (. - gGbaBios)

.pool