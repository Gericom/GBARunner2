.org 0x8000
.global main
main:
	ldr r0,= 0x04000500
	ldr r1,= 0x807F
	str r1, [r0]
	ldr r0,= 0x04000504
	ldr r1,= 0x200
	str r1, [r0]

	ldr r0,= 0x04000404
	ldr r1,= (0x02400000 - 1536)
	str r1, [r0]
	ldr r0,= 0x04000408
	ldr r1,= -1594
	strh r1, [r0]
	ldr r0,= 0x0400040A
	mov r1, #0
	strh r1, [r0]
	ldr r0,= 0x0400040C
	mov r1, #384 //#4
	str r1, [r0]
	ldr r0,= 0x04000400
	ldr r1,= 0x8840007F
	str r1, [r0]
main_loop:
	b main_loop