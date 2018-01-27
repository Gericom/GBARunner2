.section .itcm

#include "consts.s"

.global write_address_from_handler_32bit
write_address_from_handler_32bit:
	cmp r9, #0x0F000000
	ldrlo pc, [pc, r9, lsr #22]
	bx lr

	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_from_handler_io_32
	.word write_address_ignore
	.word write_address_from_handler_vram_32
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore //write_address_from_handler_eeprom_32
	.word write_address_from_handler_sram_32

write_address_from_handler_io_32:
	sub r13, r9, #0x04000000
	cmp r13, #0x20C
	bxge lr
	mov r13, r13, lsr #1
	ldr r12,= write_table_32bit_dtcm_new
	ldrh r13, [r12, r13]
	orr pc, r13, #0x01000000	//itcm

write_address_from_handler_vram_32:
	ldr r13,= DISPCNT_copy
	ldrh r13, [r13]
	and r13, #7
	cmp r13, #3
	ldrlt r13,= 0x06010000
	ldrge r13,= 0x06014000
	cmp r9, r13
	strlt r11, [r9]
	bxlt lr

	ldr r12,= 0x06018000
	cmp r9, r12
	bxge lr

	add r10, r9, #0x3F0000
	str r11, [r10]
	bx lr

write_address_from_handler_sram_32:
	ldr r12,= 0x01FF0000
	bic r10, r9, r12
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	and r12, r9, #3
	mov r12, r12, lsl #3
	mov r11, r11, ror r12
	bic r10, r10, #3
	strb r11, [r10]
	bx lr

.global write_address_from_handler_16bit
write_address_from_handler_16bit:
	cmp r9, #0x0F000000
	ldrlo pc, [pc, r9, lsr #22]
	bx lr

	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_from_handler_io_16
	.word write_address_ignore
	.word write_address_from_handler_vram_16
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore //write_address_from_handler_eeprom_16
	.word write_address_from_handler_sram_16

write_address_from_handler_io_16:
	sub r13, r9, #0x04000000
	cmp r13, #0x20C
	bxge lr
	ldr r12,= write_table_16bit_dtcm_new
	ldrh r13, [r12, r13]
	orr pc, r13, #0x01000000	//itcm

write_address_from_handler_vram_16:
	ldr r13,= DISPCNT_copy
	ldrh r13, [r13]
	and r13, #7
	cmp r13, #3
	ldrlt r13,= 0x06010000
	ldrge r13,= 0x06014000
	cmp r9, r13
	strlth r11, [r9]
	bxlt lr

	ldr r12,= 0x06018000
	cmp r9, r12
	bxge lr

	add r10, r9, #0x3F0000
	strh r11, [r10]
	bx lr

write_address_from_handler_sram_16:
	ldr r12,= 0x01FF0000
	bic r10, r9, r12
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	tst r9, #1
	movne r11, r11, ror #8
	bic r10, r10, #1
	strb r11, [r10]
	bx lr

.global write_address_from_handler_8bit
write_address_from_handler_8bit:
	cmp r9, #0x0F000000
	ldrlo pc, [pc, r9, lsr #22]
	bx lr

	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_from_handler_io_8
	.word write_address_ignore
	.word write_address_from_handler_vram_8
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore //write_address_from_handler_eeprom_8
	.word write_address_from_handler_sram_8

write_address_from_handler_io_8:
	sub r13, r9, #0x04000000
	cmp r13, #0x20C
	bxge lr
	mov r13, r13, lsl #1
	ldr r12,= write_table_8bit_dtcm_new
	ldrh r13, [r12, r13]
	orr pc, r13, #0x01000000	//itcm

write_address_from_handler_vram_8:
	ldr r13,= DISPCNT_copy
	ldrh r13, [r13]
	and r13, #7
	cmp r13, #3
	ldrlt r13,= 0x06010000
	ldrge r13,= 0x06014000
	cmp r9, r13
	bxge lr //nothing written on 8 bit access

	orr r11, r11, lsl #8
	strh r11, [r9]
	bx lr

write_address_from_handler_sram_8:
	ldr r12,= 0x01FF0000
	bic r10, r9, r12
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	strb r11, [r10]
	bx lr

.global write_address_nomod_8
write_address_nomod_8:
	strb r11, [r9]
	bx lr

.global write_address_nomod_16
write_address_nomod_16:
	strh r11, [r9]
	bx lr

.global write_address_nomod_32
write_address_nomod_32:
	str r11, [r9]
	bx lr

.global write_address_ignore
write_address_ignore:
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

.global shadow_dispstat
shadow_dispstat:
	.word 0

.global write_address_dispstat
write_address_dispstat:
	strh r11, shadow_dispstat
	mov r12, r11, lsr #8 //vcount
	cmp r12, #160
	blt write_address_dispstat_finish
	add r12, #32
	mov r12, r12, lsl #8
	tst r12, #(1 << 16)
	orrne r12, #(1 << 7)
	bic r12, r12, #(1 << 16)
	bic r11, #0xFF00
	bic r11, #0x0080
	orr r11, r11, r12
write_address_dispstat_finish:
	strh r11, [r9]
	bx lr

.global write_address_sound_cnt_top16
write_address_sound_cnt_top16:
	//ldr r10,= 0x04000188
	//ldr r13,= 0xAA5500C8
	//mov r12, #0
	//str r13, [r10]
	//str r11, [r10]
	//str r12, [r10]
	bx lr

//	mov r12, #0
//	str r11, [r10]
//	tst r11, #0x300
//	beq write_address_sound_cnt_top16_disable_A
//	tst r11, #(1 << 11)
//	bne write_address_sound_cnt_top16_reset
//write_address_sound_cnt_top16_enable_A:
//	ldr r10,= 0x04000188
//	ldr r11,= 0xAA5500C4
//	mov r12, #0
//	str r11, [r10]
//	str r12, [r10]
//	bx lr

//write_address_sound_cnt_top16_disable_A:
//	ldr r10,= 0x04000188
//	ldr r11,= 0xAA5500C5
//	mov r12, #0
//	str r11, [r10]
//	str r12, [r10]
//	bx lr

//write_address_sound_cnt_top16_reset:
//	ldr r10,= 0x04000188
//	ldr r11,= 0xAA5500C5
//	ldr r13,= 0xAA5500C4
//	mov r12, #0
//	str r11, [r10]
//	str r13, [r10]
//	str r12, [r10]
//	bx lr

.global write_address_sound_cnt
write_address_sound_cnt:
	mov r11, r11, lsr #16
	b write_address_sound_cnt_top16

.global write_address_snd_fifo_A
write_address_snd_fifo_A:
	ldr r12,= 0x04000188
	str r9, [r12]
	str r11, [r12]
	bx lr

.global write_address_dma_src
write_address_dma_src:
	cmp r11, #0x08000000
	blt write_address_dma_src_cont
	cmp r11, #0x0E000000
	bge write_address_dma_src_cont2
	bic r11, r11, #0x07000000
	sub r11, r11, #0x05000000
	sub r11, r11, #0x00FC0000
	str r11, [r9]
	bx lr
write_address_dma_src_cont:
	ldr r13,= DISPCNT_copy
	ldrh r13, [r13]
	and r13, #7
	cmp r13, #3
	ldrlt r13,= 0x06010000
	ldrge r13,= 0x06014000
	cmp r11, r13
	blt write_address_dma_src_cont2
	ldr r13,= 0x06018000
	cmp r11, r13
	addlt r11, #0x3F0000
write_address_dma_src_cont2:
	str r11, [r9]
	bx lr

.global write_address_dma_src_top16
write_address_dma_src_top16:
	cmp r11, #0x0800
	blt write_address_dma_src_cont_top16
	cmp r11, #0x0E00
	bge write_address_dma_src_cont_top16_2
	bic r11, r11, #0x0700
	sub r11, r11, #0x0500
	sub r11, r11, #0x00FC
	strh r11, [r9]
	bx lr
write_address_dma_src_cont_top16:
	ldr r13,= 0x0601
	cmp r11, r13
	addeq r11, #0x3F
write_address_dma_src_cont_top16_2:
	strh r11, [r9]
	bx lr

.global write_address_dma_dst
write_address_dma_dst:
	ldr r13,= DISPCNT_copy
	ldrh r13, [r13]
	and r13, #7
	cmp r13, #3
	ldrlt r13,= 0x06010000
	ldrge r13,= 0x06014000
	cmp r11, r13
	blt write_address_dma_dst_cont
	ldr r13,= 0x06018000
	cmp r11, r13
	addlt r11, #0x3F0000
write_address_dma_dst_cont:
	str r11, [r9]
	bx lr

.global write_address_dma_dst_top16
write_address_dma_dst_top16:
	ldr r13,= 0x0601
	cmp r11, r13
	addeq r11, #0x3F
	strh r11, [r9]
	bx lr

.global write_address_dma_size
write_address_dma_size:
	ldr r13,= 0x040000DC
	cmp r9, r13
	streqh r11, [r9]
	bxeq lr
	cmp r11, #0
	moveq r11, #0x4000
	strh r11, [r9]
	bx lr

.global write_address_dma_control
write_address_dma_control:
	bic r11, r11, #0x1F
	ldr r13,= 0x040000DE
	cmp r9, r13
	ldreqh r13, [r9, #-2]
	cmpeq r13, #0
	orreq r11, #1
	mov r13, r11, lsr #12
	and r13, #3
	cmp r13, #3
	bge write_address_dma_control_cont
	bic r11, r11, #0x3800
	orr r11, r11, r13, lsl #11

	tst r11, #0x8000
	beq write_address_dma_control_cont2
	ldr r13, [r9, #-0x6]
	cmp r13, #0x02000000
	biclt r11, #0x8000
	blt write_address_dma_control_cont2

	ldr r12,= MAIN_MEMORY_ADDRESS_GBARUNNER_DATA

	ldrh r10, [r9, #-0x2]
	and r13, r11, #0x1F
	orr r10, r13, lsl #16
	tst r11, #(1 << 10)

	ldr r13, [r9, #-0xA]

	addeq r13, r13, r10, lsl #1
	addne r13, r13, r10, lsl #2

	cmp r13, r12
	blt write_address_dma_control_cont2
	cmp r13, #0x03000000
	blt write_address_dma_control_rom_src
	ldr r12,= 0x040000A4
	ldr r13, [r9, #-0x6]
	cmp r13, r12
	bxeq lr
write_address_dma_control_cont2:
	strh r11, [r9]
	bx lr

write_address_dma_control_rom_src:
	//still store the register, but not enabled
	bic r11, r11, #0x8000
	strh r11, [r9]
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r3,lr}
	ldr r0, [r9, #-0xA]
	sub r0, #MAIN_MEMORY_ADDRESS_ROM_DATA
	ldrh r10, [r9, #-0x2]
	and r12, r11, #0x1F
	orr r10, r12, lsl #16
	tst r11, #(1 << 10)
	moveq r1, r10, lsl #1
	movne r1, r10, lsl #2
	ldr r2, [r9, #-0x6]
	bl read_gba_rom_asm
	pop {r0-r3,lr}
	bx lr

write_address_dma_control_cont:
	ldr r13,= 0x40000C6
	cmp r9, r13

	bic r11, r11, #0x8000
	strh r11, [r9]
	bxne lr

	ldr r13,= 0x040000BC
	ldr r13, [r13]

	ldr r10,= 0x04000188
	ldr r12,= 0xAA5500F8
	str r12, [r10]
	str r13, [r10]

	bx lr

.global write_address_dma_size_control
write_address_dma_size_control:
	mov r12, r11, lsl #16
	movs r12, r12, lsr #16
	bne write_address_dma_size_control_cont
	ldr r13,= 0x040000DC
	cmp r9, r13
	movne r12, #0x4000
	moveq r12, #0x10000
write_address_dma_size_control_cont:
	ldr r13,= 0x1FFFFF
	bic r11, r13
	orr r11, r12
	mov r13, r11, lsr #28
	and r13, #3
	cmp r13, #3
	bge write_address_dma_size_control_cont2
	bic r11, r11, #0x38000000
	orr r11, r11, r13, lsl #27
	tst r11, #0x80000000
	beq write_address_dma_size_control_cont3
	ldr r13, [r9, #-0x4]
	cmp r13, #0x02000000
	biclt r11, #0x80000000
	blt write_address_dma_size_control_cont3

	ldr r12,= MAIN_MEMORY_ADDRESS_GBARUNNER_DATA

	ldr r13,= 0x1FFFFF
	and r10, r11, r13
	tst r11, #(1 << 26)

	ldr r13, [r9, #-0x8]

	addeq r13, r13, r10, lsl #1
	addne r13, r13, r10, lsl #2
	
	cmp r13, r12
	blt write_address_dma_size_control_cont3
	cmp r13, #0x03000000
	blt write_address_dma_size_control_rom_src
	ldr r12,= 0x040000A4
	ldr r13, [r9, #-0x4]
	cmp r13, r12
	bxeq lr
write_address_dma_size_control_cont3:
	str r11, [r9]
	bx lr


write_address_dma_size_control_rom_src:
	//still store the register
	bic r11, #0x80000000
	str r11, [r9]
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r3,lr}
	ldr r0, [r9, #-0x8]
	sub r0, #MAIN_MEMORY_ADDRESS_ROM_DATA
	ldr r12,= 0x1FFFFF
	and r10, r11, r12
	tst r11, #(1 << 26)
	moveq r1, r10, lsl #1
	movne r1, r10, lsl #2
	ldr r2, [r9, #-0x4]
	bl read_gba_rom_asm
	pop {r0-r3,lr}
	bx lr

write_address_dma_size_control_cont2:
	ldr r13,= 0x040000C4
	cmp r9, r13

	bic r11, #0x80000000
	str r11, [r9]
	bxne lr

	ldr r13,= 0x040000BC
	ldr r13, [r13]

	ldr r10,= 0x04000188
	ldr r12,= 0xAA5500F8
	str r12, [r10]
	str r13, [r10]

	bx lr

.global write_address_timer_counter
write_address_timer_counter:
	mov r12, r11, lsl #17
	mov r12, r12, lsr #16
	strh r12, [r9]
	ldr r12,= 0x04000188
	str r9, [r12]
	str r11, [r12]
	bx lr

.global write_address_timer
write_address_timer:
	mov r12, r11, lsr #16
	mov r13, r11, lsl #17
	mov r13, r13, lsr #16
	orr r12, r13, r12, lsl #16
	str r12, [r9]
	ldr r12,= 0x04000188
	str r9, [r12]
	str r11, [r12]
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

.global write_address_wait_control
write_address_wait_control:
	ldr r13,= WAITCNT_copy
	str r11, [r13]
	bx lr

.global write_address_wait_control_bottom8
write_address_wait_control_bottom8:
	ldr r13,= WAITCNT_copy
	strb r11, [r13]
	bx lr

.global write_address_wait_control_top8
write_address_wait_control_top8:
	ldr r13,= (WAITCNT_copy + 1)
	strb r11, [r13]
	bx lr