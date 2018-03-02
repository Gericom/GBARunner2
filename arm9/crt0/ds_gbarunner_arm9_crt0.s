.arch	armv5te
.cpu	arm946e-s

.extern gba_setup

.section ".init"
.global _start
.align	4
.arm
_start:
	mov	r0, #0x04000000
	str	r0, [r0, #0x208]
	b gba_setup
	
.align
.pool
.end