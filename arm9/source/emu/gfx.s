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

	bicge r12, #0xF00	//clear all bg bits
	orrge r12, #0x800	//display only bg3 (which goes unused on the gba)

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

	bxlt lr

	ldr r11,= 0x04000000
	mov r12, #256
	strh r12, [r11, #0x30]
	ldr r12,= -4096
	strh r12, [r11, #0x32]
	mov r12, #1
	strh r12, [r11, #0x34]
	mov r12, #240
	strh r12, [r11, #0x36]

	ldr r10,= DISPCNT_copy
	ldrh r10, [r10]
	tst r10, #(1 << 4)

	mov r12, #0
	str r12, [r11, #0x38]
	streq r12, [r11, #0x3C]
	ldrne r12,= 8192
	strne r12, [r11, #0x3C]

	ldrh r12, [r11, #0xC]
	bic r12, #0xFF00
	bic r12, #0x00BC

	cmp r13, #4
	orrne r12, #0x84
	orreq r12, #0x80
	orr r12, #0x6000

	tst r10, #(1 << 4)
	orrne r12, #(2 << 8)

	strh r12, [r11, #0xE]
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