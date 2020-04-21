.section .itcm

#include "consts.s"

.global read_address_dispcontrol
read_address_dispcontrol:
	ldr r10,= DISPCNT_copy
	ldr r10, [r10]
	bx lr

.global read_address_dispcontrol_bottom8
read_address_dispcontrol_bottom8:
	ldr r10,= DISPCNT_copy
	ldrb r10, [r10]
	bx lr

.global read_address_dispcontrol_top8
read_address_dispcontrol_top8:
	ldr r10,= (DISPCNT_copy + 1)
	ldrb r10, [r10]
	bx lr

.global read_address_dispstat
read_address_dispstat:
	ldrh r10, [r9]
	bic r10, #0xFF00
	bic r10, #0x0080
	ldr r11,= shadow_dispstat
	ldrh r11, [r11]
	and r11, #0xFF00
	orr r10, r10, r11
	bx lr

.global read_address_vcount
read_address_vcount:
	ldrh r10, [r9]
	cmp r10, #160
	bxlt lr
	cmp r10, #192
	movlt r10, #159
	bxlt lr
	sub r10, #32
	cmp r10, #227
	movgt r10, #227
	bx lr

.global write_address_dispcontrol
write_address_dispcontrol:
	ldr r12,= DISPCNT_copy
	strh r11, [r12]
write_address_dispcontrol_cont:
	ldr r12,= 0xFF80
	and r12, r11, r12
	tst r11, #(1 << 5)//hblank free bit is moved on the ds
	orrne r12, #(1 << 23)
	tst r11, #(1 << 6)//obj mode bit is moved on the ds as well
	orrne r12, #(1 << 4)
	orr r12, #(1 << 16)//display mode, which did not exist on gba
	and r13, r11, #7
	mov r11, r13
	cmp r13, #1
	moveq r11, #2
	biceq r12, #(1 << 11)
	cmp r13, #1
		biceq r12, #0x800
	cmp r13, #2
		biceq r12, #0x300

	cmp r13, #3
	movge r11, #5
	orr r12, r11

	bicge r12, #0xB00	//clear all bg bits except bg2

	ldr r10, [r9]

	str r12, [r9]

	//move gba vram block b to either bg or obj
	ldr r11,= 0x04000245
	movlt r12, #0x82
	movge r12, #0x91 //#0x89
	strb r12, [r11]

	//and change the pu settings accordingly
	ldrlt r11,= (1 | (14 << 1) | 0x06010000)
	ldrge r11,= (1 | (13 << 1) | 0x06014000)
	mcr p15, 0, r11, c6, c3, 0

	ldr r11,= 0x04000000

	bge 1f

	and r10, #7
	cmp r10, #5
		bxne lr //only if the previous mode was bitmap restore the shadow registers

	//restore the bg2cnt register if not bitmap mode
	ldr r13,= BG2CNT_copy
	ldrh r12, [r13]
	strh r12, [r11, #0xC]

	ldr r12, [r13, #(address_BG2PA_copy - address_BG2CNT_copy)]
	str r12, [r11, #0x20]

	ldr r12, [r13, #(address_BG2PC_copy - address_BG2CNT_copy)]
	str r12, [r11, #0x24]

	ldr r12, [r13, #(address_BG2X_copy - address_BG2CNT_copy)]
	str r12, [r11, #0x28]

	ldr r12, [r13, #(address_BG2Y_copy - address_BG2CNT_copy)]
	str r12, [r11, #0x2C]

	bx lr

1:	
	mov r12, #256
	strh r12, [r11, #0x20]
	ldr r12,= -4096
	strh r12, [r11, #0x22]
	mov r12, #1
	strh r12, [r11, #0x24]
	mov r12, #240
	strh r12, [r11, #0x26]

	ldr r10,= DISPCNT_copy
	ldrh r10, [r10]

	cmp r13, #3

	tstne r10, #(1 << 4) //test page flip bit

	mov r12, #0
	str r12, [r11, #0x28]
	movne r12, #8192
	str r12, [r11, #0x2C]

	ldrh r12, [r11, #0xC]
	and r12, #0x43

	cmp r13, #4
	orrne r12, #0x84
	orreq r12, #0x80
	orr r12, #0x6000

	tst r10, #(1 << 4)
	orrne r12, #(2 << 8)

	strh r12, [r11, #0xC]
	bx lr
	

.global write_address_dispcontrol_bottom8
write_address_dispcontrol_bottom8:
	ldr r13,= DISPCNT_copy
	ldrh r12, [r13]
	and r12, #0xFF00
	orr r12, r12, r11
	strh r12, [r13]
	mov r11, r12
	b write_address_dispcontrol_cont

.global write_address_dispcontrol_top8
write_address_dispcontrol_top8:
	ldr r13,= DISPCNT_copy
	ldrh r12, [r13]
	and r12, r12, #0xFF
	orr r12, r12, r11, lsl #8
	strh r12, [r13]
	mov r11, r12
	b write_address_dispcontrol_cont
	//strb r11, [r10]
	//bx lr

.global write_address_dispstat
write_address_dispstat:
	mov r11, r11, lsl #16
	mov r11, r11, lsr #16
	ldr r10,= shadow_dispstat
	strh r11, [r10]
	mov r12, r11, lsr #8 //vcount
	cmp r12, #160
	blt write_address_dispstat_finish
	add r12, #32
	mov r12, r12, lsl #8
	tst r12, #(1 << 16)
	orrne r12, #(1 << 7)
	bic r12, r12, #(1 << 16)
	bic r11, #0xFFFFFF80
	orr r11, r11, r12
write_address_dispstat_finish:
	strh r11, [r9]
	bx lr

.global gfx_writeBg2Cnt
gfx_writeBg2Cnt:
	ldr r13,= BG2CNT_copy
	strh r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - address_BG2CNT_copy)]
	and r12, r13, #7
	cmp r12, #3
	strlth r11, [r9]
	bxlt lr

	ldrh r13, [r9]
	and r11, #0x43
	bic r13, #0x43
	orr r13, r11
	strh r13, [r9]
	bx lr

.global gfx_writeBg2PAPB
gfx_writeBg2PAPB:
	ldr r13,= BG2PA_copy
	str r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - address_BG2PA_copy)]
	and r12, r13, #7
	cmp r12, #3
		ldrge r11,= 0xF0000100
	str r11, [r9]
	bx lr

.global gfx_writeBg2PCPD
gfx_writeBg2PCPD:
	ldr r13,= BG2PC_copy
	str r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - address_BG2PC_copy)]
	and r12, r13, #7
	cmp r12, #3
		ldrge r11,= 0x00F00001
	str r11, [r9]
	bx lr

.global gfx_writeBg2PA
gfx_writeBg2PA:
	ldr r13,= BG2PA_copy
	strh r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - address_BG2PA_copy)]
	and r12, r13, #7
	cmp r12, #3
		movge r11, #256
	strh r11, [r9]
	bx lr

.global gfx_writeBg2PB
gfx_writeBg2PB:
	ldr r13,= BG2PB_copy
	strh r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - address_BG2PB_copy)]
	and r12, r13, #7
	cmp r12, #3
		ldrge r11,= -4096
	strh r11, [r9]
	bx lr

.global gfx_writeBg2PC
gfx_writeBg2PC:
	ldr r13,= BG2PC_copy
	strh r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - address_BG2PC_copy)]
	and r12, r13, #7
	cmp r12, #3
		movge r11, #1
	strh r11, [r9]
	bx lr

.global gfx_writeBg2PD
gfx_writeBg2PD:
	ldr r13,= BG2PD_copy
	strh r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - address_BG2PD_copy)]
	and r12, r13, #7
	cmp r12, #3
		movge r11, #240
	strh r11, [r9]
	bx lr

.global gfx_writeBg2X
gfx_writeBg2X:
	ldr r13,= BG2X_copy
	str r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - address_BG2X_copy)]
	and r12, r13, #7
	cmp r12, #3
		movge r11, #0
	str r11, [r9]
	bx lr

.global gfx_writeBg2X_L
gfx_writeBg2X_L:
	ldr r13,= BG2X_copy
	strh r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - address_BG2X_copy)]
	and r12, r13, #7
	cmp r12, #3
		movge r11, #0
	strh r11, [r9]
	bx lr

.global gfx_writeBg2X_H
gfx_writeBg2X_H:
	ldr r13,= (BG2X_copy + 2)
	strh r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - (address_BG2X_copy + 2))]
	and r12, r13, #7
	cmp r12, #3
		movge r11, #0
	strh r11, [r9]
	bx lr

.global gfx_writeBg2Y
gfx_writeBg2Y:
	ldr r13,= BG2Y_copy
	str r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - address_BG2Y_copy)]
	and r12, r13, #7
	cmp r12, #3
	blt 1f
	tstne r13, #(1 << 4) //test page flip bit
	moveq r11, #0
	movne r11, #8192
1:
	str r11, [r9]
	bx lr

.global gfx_writeBg2Y_L
gfx_writeBg2Y_L:
	ldr r13,= BG2Y_copy
	strh r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - address_BG2Y_copy)]
	and r12, r13, #7
	cmp r12, #3
	blt 1f
	tstne r13, #(1 << 4) //test page flip bit
	moveq r11, #0
	movne r11, #8192
1:
	strh r11, [r9]
	bx lr

.global gfx_writeBg2Y_H
gfx_writeBg2Y_H:
	ldr r13,= (BG2Y_copy + 2)
	strh r11, [r13]

	ldrh r13, [r13, #(address_DISPCNT_copy - (address_BG2Y_copy + 2))]
	and r12, r13, #7
	cmp r12, #3
		movge r11, #0
	strh r11, [r9]
	bx lr