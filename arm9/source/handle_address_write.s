.section .itcm

.include "consts.s"

.global write_address_from_handler_32bit
write_address_from_handler_32bit:
	cmp r9, #0x06000000
	bge write_address_from_handler_sprites_32bit
	subs r13, r9, #0x04000000
	bxlt lr
	cmp r13, #0x20C
	bxge lr
	mov r13, r13, lsr #1
	ldr r12,= address_write_table_32bit_dtcm
	ldrh r13, [r12, r13]
	orr pc, r13, #0x01000000	//itcm

write_address_from_handler_sprites_32bit:
	cmp r9, #0x0E000000
	bge write_address_from_handler_sram_32bit
	ldr r12,= 0x06018000
	cmp r9, r12
	bxge lr
	add r10, r9, #0x3F0000
	str r11, [r10]
	bx lr

write_address_from_handler_sram_32bit:
	cmp r9, #0x0F000000
	bxge lr
	ldr r12,= 0x01FF8000
	bic r10, r9, r12
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	//sub r10, r10, #0x0B800000
	//sub r10, r10, #0x00008C00
	//sub r10, r10, #0x00000060
	str r11, [r10]
	bx lr

.global write_address_from_handler_16bit
write_address_from_handler_16bit:
	cmp r9, #0x06000000
	bge write_address_from_handler_sprites_16bit
	subs r13, r9, #0x04000000
	bxlt lr
	cmp r13, #0x20C
	bxge lr
	ldr r12,= address_write_table_16bit_dtcm
	ldrh r13, [r12, r13]
	orr pc, r13, #0x01000000	//itcm

write_address_from_handler_sprites_16bit:
	cmp r9, #0x0E000000
	bge write_address_from_handler_sram_16bit
	ldr r12,= 0x06018000
	cmp r9, r12
	bxge lr
	add r10, r9, #0x3F0000
	strh r11, [r10]
	bx lr

write_address_from_handler_sram_16bit:
	cmp r9, #0x0F000000
	bxge lr
	ldr r12,= 0x01FF8000
	bic r10, r9, r12
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	//sub r10, r10, #0x0B800000
	//sub r10, r10, #0x00008C00
	//sub r10, r10, #0x00000060
	strh r11, [r10]
	bx lr

.global write_address_from_handler_8bit
write_address_from_handler_8bit:
	cmp r9, #0x06000000
	bge write_address_from_handler_sprites_8bit
	subs r13, r9, #0x04000000
	bxlt lr
	cmp r13, #0x20C
	bxge lr
	mov r13, r13, lsl #1
	ldr r12,= address_write_table_8bit_dtcm
	ldrh r13, [r12, r13]
	orr pc, r13, #0x01000000	//itcm

write_address_from_handler_sprites_8bit:
	cmp r9, #0x0E000000
	bge write_address_from_handler_sram_8bit
	//nothing written on 8 bit access
	bx lr

write_address_from_handler_sram_8bit:
	cmp r9, #0x0F000000
	bxge lr
	ldr r12,= 0x01FF8000
	bic r10, r9, r12
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	//sub r10, r10, #0x0B800000
	//sub r10, r10, #0x00008C00
	//sub r10, r10, #0x00000060
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

cur_snd_buffer:
.word 0

already_playing:
.word 0

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

	ldr r12,= 0x023F0000
	ldr r13, [r9, #-0xA]
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
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r9,lr}
	ldr r0, [r9, #-0xA]
	sub r0, #0x02040000
	ldrh r10, [r9, #-0x2]
	and r12, r11, #0x1F
	orr r10, r12, lsl #16
	tst r11, #(1 << 10)
	moveq r1, r10, lsl #1
	movne r1, r10, lsl #2
	ldr r2, [r9, #-0x6]
	ldr r3,= read_gba_rom
	blx r3
	pop {r0-r9,lr}
	bx lr

write_address_dma_control_cont:
	ldr r13,= 0x40000C6
	cmp r9, r13
	//ldr r13,= 0x40000D2
	//cmpne r10, r13
	bicne r11, r11, #0x8000
	strneh r11, [r9]
	bxne lr

	ldr r13,= 0x040000BC
	ldr r13, [r13]

	ldr r10,= 0x04000188
	ldr r12,= 0xAA5500F8
	str r12, [r10]
	str r13, [r10]

	//cmp r10, r13
	//beq write_address_dma_control_cont_snd2
	//ldr r13,= cur_snd_buffer
	//ldr r12, [r13]
	//cmp r12, #0
	//moveq r12, #1
	//movne r12, #0
	//str r12, [r13]
	//ldreq r12,= (0x02400000 - (1584 * 2))
	//ldrne r12,= (0x02400000 - 1584)
	//ldr r13,= cur_snd_buffer
	//ldr r10, [r13]
	//mov r12, #1584
	//mul r12, r10, r12
	//cmp r10, #9
	//moveq r10, #0
	//addne r10, #1
	//str r10, [r13]
	//ldr r13,= 0x23F8000
	//add r12, r13, r12
@	ldr r12,= 0x23F8000
@	ldr r13,= 0x40000C0
@	str r12, [r13]
@	bic r11, r11, #0x3B00
	//bic r11, r11, #0x00E0
	//orr r11, r11, #(3 << 5)
	//bic r11, r11, #0x1F
@	bic r11, r11, #0x00FF
@	mov r12, #1024 //#(396 * 4)
@	strh r12, [r9, #-2]
@	strh r11, [r9]

//	nop
//	nop
//write_address_dma_size_control_cont_dma_wait_loop:
//	ldrh r11, [r9]
//	tst r11, #0x8000
//	bne write_address_dma_size_control_cont_dma_wait_loop

	//ldr r13,= cur_snd_buffer
	//ldr r12, [r13]
	//cmp r12, #1
	//bxeq lr
	//ldr r12, [r13, #4]
	//cmp r12, #1
	//bxeq lr
	//mov r12, #1
	//str r12, [r13, #4]

	//ldr r13,= cur_snd_buffer
	//ldr r12, [r13]
	//cmp r12, #5
	//bxne lr
	//ldr r12, [r13, #4]
	//cmp r12, #1
	//bxeq lr
	//mov r12, #1
	//str r12, [r13, #4]

@	ldr r10,= 0x04000188
	//ldr r11,= 0xAA5500C5
@	ldr r13,= 0xAA5500C4
	//mov r12, #0
	//str r11, [r10]
@	str r13, [r10]
	//str r12, [r10]

	bx lr

//write_address_dma_control_cont_snd2:
//	ldr r13,= 0x40000CC
//	ldr r12,= (0x02400000 - 1584)
//	str r12, [r13]
//	bic r11, r11, #0x3B00
//	bic r11, r11, #0x00E0
	//orr r11, r11, #(3 << 5)
//	bic r11, r11, #0x1F
//	mov r12, #396
//	strh r12, [r9, #-2]
//	strh r11, [r9]

//	ldr r10,= 0x04000188
//	ldr r11,= 0xAA5500C7
//	ldr r13,= 0xAA5500C6
//	mov r12, #0
//	str r11, [r10]
//	str r13, [r10]
//	str r12, [r10]

//	bx lr

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

	ldr r12,= 0x023F0000
	ldr r13, [r9, #-0x8]
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
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r9,lr}
	ldr r0, [r9, #-0x8]
	sub r0, #0x02040000
	ldr r12,= 0x1FFFFF
	and r10, r11, r12
	tst r11, #(1 << 26)
	moveq r1, r10, lsl #1
	movne r1, r10, lsl #2
	ldr r2, [r9, #-0x4]
	ldr r3,= read_gba_rom
	blx r3
	pop {r0-r9,lr}
	bx lr

write_address_dma_size_control_cont2:
	ldr r13,= 0x040000C4
	cmp r9, r13
	//ldr r13,= 0x040000D0
	//cmpne r10, r13
	bicne r11, #0x80000000
	strne r11, [r9]
	bxne lr

	ldr r13,= 0x040000BC
	ldr r13, [r13]

	ldr r10,= 0x04000188
	ldr r12,= 0xAA5500F8
	str r12, [r10]
	str r13, [r10]
	//cmp r10, r13
	//beq write_address_dma_size_control_cont2_snd2
	//ldr r13,= 0x40000C0
	//ldr r12,= (0x02400000 - (1584 * 2))
	//str r12, [r13]
	//ldr r13,= cur_snd_buffer
	//ldr r12, [r13]
	//cmp r12, #0
	//moveq r12, #1
	//movne r12, #0
	//str r12, [r13]
	//ldreq r12,= (0x02400000 - (1584 * 2))
	//ldrne r12,= (0x02400000 - 1584)
@	ldr r13,= 0x40000C0
@	ldr r12,= 0x23F8000 //(0x02400000 - (1584 * 2))
	//ldr r13,= cur_snd_buffer
	//ldr r10, [r13]
	//mov r12, #1584
	//mul r12, r10, r12
	//cmp r10, #9
	//moveq r10, #0
	//addne r10, #1
	//str r10, [r13]
	//ldr r13,= 0x23F8000
	//add r12, r13, r12
	//ldr r13,= 0x40000C0
@	str r12, [r13]
	//bic r11, r11, #0x3B000000
	//bic r11, r11, #0x00E00000
	//orr r11, r11, #(3 << (5 + 16))
@	ldr r13,= 0x3BFFFFFF
@	bic r11, r13
@	orr r11, r11, #1024 //(396 * 2)
@	str r11, [r9]
	
//	nop
//	nop
//write_address_dma_size_control_cont2_dma_wait_loop:
//	ldr r11, [r9]
//	tst r11, #0x80000000
//	bne write_address_dma_size_control_cont2_dma_wait_loop

	//ldr r13,= cur_snd_buffer
	//ldr r12, [r13]
	//cmp r12, #5
	//bxne lr
	//ldr r12, [r13, #4]
	//cmp r12, #1
	//bxeq lr
	//mov r12, #1
	//str r12, [r13, #4]
	
@	ldr r10,= 0x04000188
	//ldr r11,= 0xAA5500C5
@	ldr r13,= 0xAA5500C4
	//mov r12, #1
	//str r11, [r10]
@	str r13, [r10]
	//str r12, [r10]

	bx lr

//write_address_dma_size_control_cont2_snd2:
//	ldr r13,= 0x40000CC
//	ldr r12,= (0x02400000 - 1584)
//	str r12, [r13]
//	bic r11, r11, #0x3B000000
//	bic r11, r11, #0x00E00000
	//orr r11, r11, #(3 << (5 + 16))
//	ldr r13,= 0x1FFFFF
//	bic r11, r13
//	orr r11, r11, #396
//	str r11, [r9]
	
//	ldr r10,= 0x04000188
//	ldr r11,= 0xAA5500C7
//	ldr r13,= 0xAA5500C6
//	mov r12, #0
//	str r11, [r10]
//	str r13, [r10]
//	str r12, [r10]

//	bx lr

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
	//orr r11, #0x3E0000
	orr r11, #0x3F0000
	str r11, [r13]
	bx lr

.global write_address_if_bottom8
write_address_if_bottom8:
	ldr r13,= 0x4000214
	//tst r11, #1
	//orrne r11, #(1 << 16)
	//orr r11, #0x3E0000
	orr r11, #0x3F0000
	str r11, [r13]
	bx lr

.global write_address_if_top8
write_address_if_top8:
	ldr r13,= 0x4000214
	orr r11, #0x3F00
	mov r11, r11, lsl #8
	str r11, [r13]
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