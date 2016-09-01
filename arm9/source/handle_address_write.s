.section .itcm

address_write_table_32bit_dtcm = 0x10000140
address_write_table_16bit_dtcm = 0x10000248
address_write_table_8bit_dtcm = 0x10000454

.global write_address_from_handler
write_address_from_handler:
//r10=address, r11=value, r12=nr bytes
	cmp r10, #0x06000000
	bge write_address_from_handler_sprites
	bic r13, r10, #0x04000000
	cmp r13, #0x20C
	bxge lr

	add pc, r12, lsl #4

	nop

	nop
	nop
	nop
	nop

write_address_from_handler_1:
	mov r13, r13, lsl #1
	ldr r12,= address_write_table_8bit_dtcm
	ldrh r13, [r12, r13]
	orr pc, r13, #0x01000000	//itcm

write_address_from_handler_2:
	ldr r12,= address_write_table_16bit_dtcm
	ldrh r13, [r12, r13]
	orr pc, r13, #0x01000000	//itcm
	nop

	nop
	nop
	nop
	nop

write_address_from_handler_4:
	mov r13, r13, lsr #1
	ldr r12,= address_write_table_32bit_dtcm
	ldrh r13, [r12, r13]
	orr pc, r13, #0x01000000	//itcm

write_address_from_handler_sprites:
	add r10, #0x3F0000
	cmp r12, #1
	orreq r11, r11, r11, lsl #8
	moveq r12, #2
	cmp r12, #4
	streq r11, [r10]
	strneh r11, [r10]
	bx lr

.global address_write_table_32bit_dtcm_setup
address_write_table_32bit_dtcm_setup:
	ldr r10,= address_write_table_32bit_dtcm
	ldr r11,= address_write_table_32bit
	mov r12, #0x20C
address_write_table_32bit_dtcm_setup_loop:
	ldr r13, [r11], #4
	strh r13, [r10], #2
	subs r12, #4
	bne address_write_table_32bit_dtcm_setup_loop
	bx lr

.global address_write_table_16bit_dtcm_setup
address_write_table_16bit_dtcm_setup:
	ldr r10,= address_write_table_16bit_dtcm
	ldr r11,= address_write_table_16bit
	mov r12, #0x20C
address_write_table_16bit_dtcm_setup_loop:
	ldr r13, [r11], #4
	strh r13, [r10], #2
	subs r12, #2
	bne address_write_table_16bit_dtcm_setup_loop
	bx lr

.global address_write_table_8bit_dtcm_setup
address_write_table_8bit_dtcm_setup:
	ldr r10,= address_write_table_8bit_dtcm
	ldr r11,= address_write_table_8bit
	mov r12, #0x20C
address_write_table_8bit_dtcm_setup_loop:
	ldr r13, [r11], #4
	strh r13, [r10], #2
	subs r12, #1
	bne address_write_table_8bit_dtcm_setup_loop
	bx lr

.global write_address_nomod_8
write_address_nomod_8:
	strb r11, [r10]
	bx lr

.global write_address_nomod_16
write_address_nomod_16:
	strh r11, [r10]
	bx lr

.global write_address_nomod_32
write_address_nomod_32:
	str r11, [r10]
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
	tst r11, #(1 << 6)//obj mode bit is moved on the ds aswell
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

	str r12, [r10]

	//move gba vram block b to either bg or obj
	ldr r11,= 0x04000245
	movlt r12, #0x82
	movge r12, #0x89
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
	mov r12, #0
	str r12, [r11, #0x38]
	str r12, [r11, #0x3C]

	ldrh r12, [r11, #0xC]
	bic r12, #0xFF00
	bic r12, #0x00BC

	cmp r13, #4
	orrne r12, #0x84
	orreq r12, #0x80
	orr r12, #0x6000
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

.global write_address_dma_src
write_address_dma_src:
	cmp r11, #0x08000000
	blt write_address_dma_src_cont
	cmp r11, #0x0E000000
	bge write_address_dma_src_cont2
	bic r11, r11, #0x07000000
	sub r11, r11, #0x05000000
	sub r11, r11, #0x00FC0000
	str r11, [r10]
	bx lr
write_address_dma_src_cont:
	ldr r13,= 0x06010000
	cmp r11, r13
	blt write_address_dma_src_cont2
	ldr r13,= 0x06018000
	cmp r11, r13
	addlt r11, #0x3F0000
write_address_dma_src_cont2:
	str r11, [r10]
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
	strh r11, [r10]
	bx lr
write_address_dma_src_cont_top16:
	ldr r13,= 0x0601
	cmp r11, r13
	addeq r11, #0x3F
write_address_dma_src_cont_top16_2:
	strh r11, [r10]
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
	str r11, [r10]
	bx lr

.global write_address_dma_dst_top16
write_address_dma_dst_top16:
	ldr r13,= 0x0601
	cmp r11, r13
	addeq r11, #0x3F
	strh r11, [r10]
	bx lr

.global write_address_dma_size
write_address_dma_size:
	ldr r13,= 0x040000DC
	cmp r10, r13
	streqh r11, [r10]
	bxeq lr
	cmp r11, #0
	moveq r11, #0x4000
	strh r11, [r10]
	bx lr

.global write_address_dma_control
write_address_dma_control:
	bic r11, r11, #0x1F
	ldr r13,= 0x040000DE
	cmp r10, r13
	ldreqh r13, [r10, #-2]
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
	ldr r12,= 0x023F0000
	ldr r13, [r10, #-0xA]
	cmp r13, r12
	blt write_address_dma_control_cont2
	cmp r13, #0x03000000
	blt write_address_dma_control_rom_src
write_address_dma_control_cont2:
	strh r11, [r10]
	bx lr

write_address_dma_control_rom_src:
	//ensure block d is mapped to the arm7
	ldr r12,= 0x4000243
	mov r13, #0x8A
	strb r13, [r12]

	ldr r12,= 0x04000188
	ldr r13,= 0xAA5500C8
	str r13, [r12]

	ldr r13, [r10, #-0xA]
	sub r13, #0x02040000
	str r13, [r12]	//address

	ldrh r13, [r10, #-0x2]
	and r12, r11, #0x1F
	orr r13, r12, lsl #16
	tst r11, #(1 << 10)
	moveq r13, r13, lsl #1
	movne r13, r13, lsl #2

	ldr r12,= 0x04000188
	str r13, [r12]	//size

	//wait for the arm7 sync command
write_address_dma_control_rom_src_fifo_loop:
	ldr r12,= 0x04000184
	ldr r12, [r12]
	tst r12, #(1 << 8)
	bne write_address_dma_control_rom_src_fifo_loop
	ldr r12,= 0x04100000
	ldr r12, [r12]	//read word from fifo
	ldr r13,= 0x55AAC8AC
	cmp r12, r13
	bne write_address_dma_control_rom_src_fifo_loop

	//block d to arm9 lcdc
	ldr r12,= 0x4000243
	mov r13, #0x80
	strb r13, [r12]

	ldr r13,= 0x06868000
	str r13, [r10, #-0xA]

	strh r11, [r10]
	nop
	nop
write_address_dma_control_rom_src_wait_loop:
	ldrh r11, [r10]
	tst r11, #0x8000
	bne write_address_dma_control_rom_src_wait_loop

	bx lr

write_address_dma_control_cont:
	ldr r13,= 0x40000C6
	cmp r10, r13
	//ldr r13,= 0x40000D2
	//cmpne r10, r13
	bicne r11, r11, #0x8000
	strneh r11, [r10]
	bxne lr
	//cmp r10, r13
	//beq write_address_dma_control_cont_snd2
	ldr r13,= 0x40000C0
	ldr r12,= (0x02400000 - (1584 * 2))
	str r12, [r13]
	bic r11, r11, #0x3B00
	bic r11, r11, #0x00E0
	//orr r11, r11, #(3 << 5)
	bic r11, r11, #0x1F
	mov r12, #396
	strh r12, [r10, #-2]
	strh r11, [r10]

	ldr r10,= 0x04000188
	ldr r11,= 0xAA5500C5
	ldr r13,= 0xAA5500C4
	mov r12, #0
	str r11, [r10]
	str r13, [r10]
	str r12, [r10]

	bx lr

write_address_dma_control_cont_snd2:
	ldr r13,= 0x40000CC
	ldr r12,= (0x02400000 - 1584)
	str r12, [r13]
	bic r11, r11, #0x3B00
	bic r11, r11, #0x00E0
	//orr r11, r11, #(3 << 5)
	bic r11, r11, #0x1F
	mov r12, #396
	strh r12, [r10, #-2]
	strh r11, [r10]

	ldr r10,= 0x04000188
	ldr r11,= 0xAA5500C7
	ldr r13,= 0xAA5500C6
	mov r12, #0
	str r11, [r10]
	str r13, [r10]
	str r12, [r10]

	bx lr

.global write_address_dma_size_control
write_address_dma_size_control:
	mov r12, r11, lsl #16
	movs r12, r12, lsr #16
	bne write_address_dma_size_control_cont
	ldr r13,= 0x040000DC
	cmp r10, r13
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
	ldr r12,= 0x023F0000
	ldr r13, [r10, #-0x8]
	cmp r13, r12
	blt write_address_dma_size_control_cont3
	cmp r13, #0x03000000
	blt write_address_dma_size_control_rom_src
write_address_dma_size_control_cont3:
	str r11, [r10]
	bx lr


write_address_dma_size_control_rom_src:
	//ensure block d is mapped to the arm7
	ldr r12,= 0x4000243
	mov r13, #0x8A
	strb r13, [r12]

	ldr r12,= 0x04000188
	ldr r13,= 0xAA5500C8
	str r13, [r12]

	ldr r13, [r10, #-0x8]
	sub r13, #0x02040000
	str r13, [r12]	//address

	//ldrh r13, [r10, #-0x2]
	//and r12, r11, #0x1F
	//orr r13, r12, lsl #16
	ldr r12,= 0x1FFFFF
	and r13, r11, r12
	tst r11, #(1 << 26)
	moveq r13, r13, lsl #1
	movne r13, r13, lsl #2

	ldr r12,= 0x04000188
	str r13, [r12]	//size

	//wait for the arm7 sync command
write_address_dma_size_control_rom_src_fifo_loop:
	ldr r12,= 0x04000184
	ldr r12, [r12]
	tst r12, #(1 << 8)
	bne write_address_dma_size_control_rom_src_fifo_loop
	ldr r12,= 0x04100000
	ldr r12, [r12]	//read word from fifo
	ldr r13,= 0x55AAC8AC
	cmp r12, r13
	bne write_address_dma_size_control_rom_src_fifo_loop

	//block d to arm9 lcdc
	ldr r12,= 0x4000243
	mov r13, #0x80
	strb r13, [r12]

	ldr r13,= 0x06868000
	str r13, [r10, #-0x8]

	str r11, [r10]
	nop
	nop
write_address_dma_size_control_rom_src_wait_loop:
	ldr r11, [r10]
	tst r11, #0x80000000
	bne write_address_dma_size_control_rom_src_wait_loop

	bx lr

write_address_dma_size_control_cont2:
	ldr r13,= 0x040000C4
	cmp r10, r13
	//ldr r13,= 0x040000D0
	//cmpne r10, r13
	bicne r11, #0x80000000
	strne r11, [r10]
	bxne lr
	//cmp r10, r13
	//beq write_address_dma_size_control_cont2_snd2
	ldr r13,= 0x40000C0
	ldr r12,= (0x02400000 - (1584 * 2))
	str r12, [r13]
	bic r11, r11, #0x3B000000
	bic r11, r11, #0x00E00000
	//orr r11, r11, #(3 << (5 + 16))
	ldr r13,= 0x1FFFFF
	bic r11, r13
	orr r11, r11, #396
	str r11, [r10]
	
	ldr r10,= 0x04000188
	ldr r11,= 0xAA5500C5
	ldr r13,= 0xAA5500C4
	mov r12, #0
	str r11, [r10]
	str r13, [r10]
	str r12, [r10]

	bx lr

write_address_dma_size_control_cont2_snd2:
	ldr r13,= 0x40000CC
	ldr r12,= (0x02400000 - 1584)
	str r12, [r13]
	bic r11, r11, #0x3B000000
	bic r11, r11, #0x00E00000
	//orr r11, r11, #(3 << (5 + 16))
	ldr r13,= 0x1FFFFF
	bic r11, r13
	orr r11, r11, #396
	str r11, [r10]
	
	ldr r10,= 0x04000188
	ldr r11,= 0xAA5500C7
	ldr r13,= 0xAA5500C6
	mov r12, #0
	str r11, [r10]
	str r13, [r10]
	str r12, [r10]

	bx lr

.global write_address_timer_counter
write_address_timer_counter:
	mov r11, r11, lsl #17
	mov r11, r11, lsr #16
	strh r11, [r10]
	bx lr

.global write_address_timer
write_address_timer:
	mov r12, r11, lsr #16
	mov r13, r11, lsl #17
	mov r13, r13, lsr #16
	orr r12, r13, r12, lsl #16
	str r12, [r10]
	bx lr

.global write_address_ie
write_address_ie:
	ldr r13,= 0x4000210
	strh r11, [r13]
	bx lr

.global write_address_ie_bottom8
write_address_ie_bottom8:
	ldr r13,= 0x4000210
	strb r11, [r13]
	bx lr

.global write_address_ie_top8
write_address_ie_top8:
	ldr r13,= 0x4000211
	strb r11, [r13]
	bx lr

.global write_address_if
write_address_if:
	ldr r13,= 0x4000214
	strh r11, [r13]
	bx lr

.global write_address_if_bottom8
write_address_if_bottom8:
	ldr r13,= 0x4000214
	strb r11, [r13]
	bx lr

.global write_address_if_top8
write_address_if_top8:
	ldr r13,= 0x4000215
	strb r11, [r13]
	bx lr

.global write_address_ie_if
write_address_ie_if:
	ldr r13,= 0x4000210
	strh r11, [r13]
	mov r11, r11, lsr #16
	strh r11, [r13, #4]
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