.section .itcm

#include "consts.s"

.global fake_irq_flags
fake_irq_flags:
	.word 0

.global read_address_ie
read_address_ie:
	ldr r13,= 0x4000210
	ldrh r10, [r13]
	//ldrb r11, [r13, #2]
	//tst r11, #1
	//orrne r10, #1
	bx lr

.global read_address_ie_bottom8
read_address_ie_bottom8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x210]
	//ldrb r11, [r13, #0x212]
	//tst r11, #1
	//orrne r10, #1
	bx lr

.global read_address_ie_top8
read_address_ie_top8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x211]
	bx lr

.global read_address_if
read_address_if:
	ldr r13,= 0x4000214
	ldrh r10, [r13]
	ldrh r11, fake_irq_flags
	orr r10, r10, r11
	bx lr

.global read_address_if_bottom8
read_address_if_bottom8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x214]
	ldrb r11, fake_irq_flags
	orr r10, r10, r11
	//ldrb r11, [r13, #0x216]
	//bic r10, #1
	//tst r11, #1
	//orrne r10, #1
	bx lr

.global read_address_if_top8
read_address_if_top8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x215]
	ldrb r11, (fake_irq_flags + 1)
	orr r10, r10, r11
	bx lr

.global read_address_ie_if
read_address_ie_if:
	ldr r13,= 0x4000210
	ldrh r12, [r13]
	//ldrb r11, [r13, #2]
	//tst r11, #1
	//orrne r12, #1
	//ldrb r11, [r13, #6]
	ldrh r13, [r13, #4]
	ldrh r11, fake_irq_flags
	orr r13, r13, r11
	orr r10, r12, r13, lsl #16
	//bic r10, #(1 << 16)
	//tst r11, #1
	//orrne r10, #(1 << 16)
	bx lr

.global write_address_ie
write_address_ie:
	ldr r13,= 0x4000210
	//tst r11, #1
	//bic r11, #1
	//orrne r11, r11, #(1 << 16)	//fifo sync as early vblank
	strh r11, [r13]
	bx lr

.global write_address_ie_bottom8
write_address_ie_bottom8:
	ldr r13,= 0x4000210
	//tst r11, #1
	//bic r11, #1
	strb r11, [r13]
	//ldrb r11, [r13, #2]
	//biceq r11, r11, #1	//fifo sync as early vblank
	//orrne r11, r11, #1	//fifo sync as early vblank
	//strb r11, [r13, #2]
	bx lr

.global write_address_ie_top8
write_address_ie_top8:
	ldr r13,= 0x4000211
	strb r11, [r13]
	bx lr

.global write_address_if
write_address_if:
	ldr r13,= 0x4000214
	//tst r11, #1
	//orrne r11, #(1 << 16)
	orr r11, #0x3E0000
	//orr r11, #0x3F0000
	str r11, [r13]
	ldr r13,= fake_irq_flags
	ldr r12, [r13]
	bic r12, r11
	str r12, [r13]
	bx lr

.global write_address_if_bottom8
write_address_if_bottom8:
	ldr r13,= 0x4000214
	//tst r11, #1
	//orrne r11, #(1 << 16)
	orr r11, #0x3E0000
	//orr r11, #0x3F0000
	str r11, [r13]
	ldr r13,= fake_irq_flags
	ldr r12, [r13]
	bic r12, r11
	str r12, [r13]
	bx lr

.global write_address_if_top8
write_address_if_top8:
	ldr r13,= 0x4000214
	//orr r11, #0x3F00
	orr r11, #0x3E00
	mov r11, r11, lsl #8
	str r11, [r13]
	ldr r13,= fake_irq_flags
	ldr r12, [r13]
	bic r12, r11
	str r12, [r13]
	bx lr

.global write_address_ie_if
write_address_ie_if:
	ldr r13,= 0x4000210
	//tst r11, #1
	//bic r11, #1
	//biceq r12, r11, #(1 << 16)	//fifo sync as early vblank
	//orrne r12, r11, #(1 << 16)	//fifo sync as early vblank
	//bic r12, //#0x3E0000
	//str r12, [r13]
	strh r11, [r13]
	mov r11, r11, lsr #16
	//tst r11, #1
	//orrne r11, #(1 << 16)
	//orr r11, #0x3E0000
	//orr r11, #0x3F0000
	orr r11, #0x3E0000
	str r11, [r13, #4]
	ldr r13,= fake_irq_flags
	ldr r12, [r13]
	bic r12, r11
	str r12, [r13]
	bx lr

//some additional irq work for capture and sound
.global irq_handler
irq_handler:
	STMFD   SP!, {R0-R3,R12,LR}

	//check for arm7 interrupt

	//make use of the backwards compatible version
	//of the data rights register, so we can use 0xFFFFFFFF instead of 0x33333333
	mov r0, #0xFFFFFFFF
	mcr p15, 0, r0, c5, c0, 0

	mov r12, #0x04000000
	ldr r1, [r12, #0x64]
	tst r1, #0x80000000
	beq cap_control
irq_cont:
	ldr r1, [r12, #0x214]
	tst r1, #(1 << 16)
	bne irq_handler_arm7_irq

irq_cont_handle_gba:
	ldr r1,= pu_data_permissions
	mcr p15, 0, r1, c5, c0, 2

	ADR     LR, loc_138
	LDR     PC, [R12,#-4]
loc_138:
	LDMFD   SP!, {R0-R3,R12,LR}
	SUBS    PC, LR, #4

irq_handler_arm7_irq:
	ldr r12,= save_save_work_state_uncached
	ldrb r12, [r12]
	cmp r12, #3 //sdsave request
	beq sdsave_request

	ldr r12,= sound_sound_emu_work_uncached
1:
	ldrb r2, [r12, #(4 + (SOUND_EMU_QUEUE_LEN * 4) + 1)]
	cmp r2, #SOUND_EMU_QUEUE_LEN
	bge 4f

	ldrb r2, [r12, #1]
	cmp r2, #0
	beq 4f

	ldrb r2, [r12, #3]
	add r3, r2, #1
	cmp r3, #SOUND_EMU_QUEUE_LEN
	subge r3, #SOUND_EMU_QUEUE_LEN
	strb r3, [r12, #3]

	add r3, r12, r2, lsl #2
	ldr r1, [r3, #4]

	ldrb lr, [r12, #(4 + (SOUND_EMU_QUEUE_LEN * 4) + 2)]
	add r2, lr, #1
	cmp r2, #SOUND_EMU_QUEUE_LEN
	subge r2, #SOUND_EMU_QUEUE_LEN
	strb r2, [r12, #(4 + (SOUND_EMU_QUEUE_LEN * 4) + 2)]

	ldmia r1, {r0, r1, r2, r3}

	add lr, r12, lr, lsl #4
	add lr, #(4 + (SOUND_EMU_QUEUE_LEN * 4) + 4)
	stmia lr, {r0, r1, r2, r3}

	mov r1, #1
	add r3, r12, #(4 + (SOUND_EMU_QUEUE_LEN * 4))
2:
	swpb r1, r1, [r3]
	cmp r1, #0
	bne 2b
	ldrb r2, [r3, #1]
	add r2, #1
	strb r2, [r3, #1]
	strb r1, [r3]

	mov r1, #1
3:
	swpb r1, r1, [r12]
	cmp r1, #0
	bne 3b
	ldrb r2, [r12, #1]
	sub r2, #1
	strb r2, [r12, #1]
	strb r1, [r12]

	cmp r2, #0
	bgt 1b
4:
	mov r12, #0x04000000
	mov r1, #(1 << 16)
	str r1, [r12, #0x214]

	ldr r3, [r12, #0x210]

	ldr r2,= fake_irq_flags
	ldr r1, [r2]

	ldr r0,= dma_shadow_regs_dtcm
	ldrh r12, [r0, #0x16]
	tst r12, #(1 << 14)
	orrne r1, #(1 << 9) //dma 1

	ldrh r12, [r0, #0x22]
	tst r12, #(1 << 14)
	orrne r1, #(1 << 10) //dma 2

	str r1, [r2]

//enable icache by pressing the R button
#if defined(ENABLE_WRAM_ICACHE) && defined(POSTPONED_ICACHE)
	ldr r0, [r12, #0x130]
	tst r0, #(1 << 8)

	moveq r0, #((1 << 5) | (1 << 6) | (1 << 0) | (1 << 7))
	mcreq p15, 0, r0, c2, c0, 1	//instruction cache
#endif

	ands r1, r3

	bne irq_cont_handle_gba

	ldr r1,= pu_data_permissions
	mcr p15, 0, r1, c5, c0, 2
	LDMFD   SP!, {R0-R3,R12,LR}
	SUBS    PC, LR, #4

sdsave_request:
	mov r12, #0x04000000
	mov r1, #(1 << 16)
	str r1, [r12, #0x214]

	ldr r12,= sd_write_save
	blx r12

	ldr r1,= pu_data_permissions
	mcr p15, 0, r1, c5, c0, 2
	LDMFD   SP!, {R0-R3,R12,LR}
	SUBS    PC, LR, #4

cap_control:
	eor r1, #0x00010000
	tst r1, #0x00010000
	mov r2, #0x80
	streqb r2, [r12, #0x242]
	strneb r2, [r12, #0x243]
	mov r2, #0x84
	strneb r2, [r12, #0x242]
	streqb r2, [r12, #0x243]
	orr r1, #0x80000000
	str r1, [r12, #0x64]

	ldr r2,= 0x07000400
	ldr r3,= 0xC0080C10
	ldr r0, [r2]
	cmp r0, r3
	beq irq_cont

	//fix sub oams
	mov r1, #0
1:
	mov r0, #0
2:
	orr r3, r1, #0x0C00 //bitmap obj
	add r3, #16
	strh r3, [r2], #2
	orr r3, r0, #0xC000 //64x64
	add r3, #8
	strh r3, [r2], #2
	mov r3, r1, lsr #3
	mov r3, r3, lsl #5
	add r3, r3, r0, lsr #3
	orr r3, #0xF000 //fully visible
	strh r3, [r2], #4

	add r0, #64
	cmp r0, #256
	blt 2b

	add r1, #64
	cmp r1, #192
	blt 1b
	b irq_cont