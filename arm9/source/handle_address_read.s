.section .itcm

address_read_table_32bit_dtcm = 0x1000086C
address_read_table_16bit_dtcm = 0x10000974
address_read_table_8bit_dtcm = 0x10000B80

.global read_address_from_handler
read_address_from_handler:
//r10=address, r11=nr bytes
	cmp r9, #0x06000000
	bge read_address_from_handler_sprites
	subs r12, r9, #0x04000000
	blt read_address_ignore
	cmp r12, #0x20C
	bge read_address_ignore

	add pc, r11, lsl #4

	nop

	nop
	nop
	nop
	nop

read_address_from_handler_1:
	mov r12, r12, lsl #1
	ldr r13,= address_read_table_8bit_dtcm
	ldrh r13, [r13, r12]
	orr pc, r13, #0x01000000	//itcm

read_address_from_handler_2:
	ldr r13,= address_read_table_16bit_dtcm
	ldrh r13, [r13, r12]
	orr pc, r13, #0x01000000	//itcm
	nop

	nop
	nop
	nop
	nop

read_address_from_handler_4:
	mov r12, r12, lsr #1
	ldr r13,= address_read_table_32bit_dtcm
	ldrh r13, [r13, r12]
	orr pc, r13, #0x01000000	//itcm

read_address_from_handler_sprites:
	cmp r9, #0x08000000
	bge read_address_from_handler_rom
	add r10, r9, #0x3F0000
	cmp r11, #2
	ldrltb r10, [r10]
	ldreqh r10, [r10]
	ldrgt r10, [r10]
	bx lr

read_address_from_handler_rom:
	cmp r9, #0x0D000000
	bge read_address_from_handler_eeprom
	ldr r12,= 0x083B0000
	cmp r9, r12
	blt read_address_from_handler_rom_in_mem
	//bic r12, r10, #0x06000000
	//ldr r13,= 0x083B0000
	//cmp r12, r13
	//sublt r10, r12, #0x05000000
	//sublt r10, r10, #0x00FC0000
	//blt read_address_from_handler_rom_cont
	

	//ldr r12,= nibble_to_char
	//mov r13, r10
	//ldrb r13, [r12, r13, lsr #28]
	//ldr r12,= (0x06202000 + 32 * 11)
	//strh r13, [r12]
	
	//ldr r12,= nibble_to_char
	//mov r13, r10, lsl #4
	//ldrb r13, [r12, r13, lsr #28]
	//ldr r12,= (0x06202000 + 32 * 11)
	//strh r13, [r12, #2]

	//ldr r12,= nibble_to_char
	//mov r13, r10, lsl #8
	//ldrb r13, [r12, r13, lsr #28]
	//ldr r12,= (0x06202000 + 32 * 11)
	//strh r13, [r12, #4]

	//ldr r12,= nibble_to_char
	//mov r13, r10, lsl #12
	//ldrb r13, [r12, r13, lsr #28]
	//ldr r12,= (0x06202000 + 32 * 11)
	//strh r13, [r12, #6]

	//ldr r12,= nibble_to_char
	//mov r13, r10, lsl #16
	//ldrb r13, [r12, r13, lsr #28]
	//ldr r12,= (0x06202000 + 32 * 11)
	//strh r13, [r12, #8]

	//ldr r12,= nibble_to_char
	//mov r13, r10, lsl #20
	//ldrb r13, [r12, r13, lsr #28]
	//ldr r12,= (0x06202000 + 32 * 11)
	//strh r13, [r12, #10]

	//ldr r12,= nibble_to_char
	//mov r13, r10, lsl #24
	//ldrb r13, [r12, r13, lsr #28]
	//ldr r12,= (0x06202000 + 32 * 11)
	//strh r13, [r12, #12]

	//ldr r12,= nibble_to_char
	//mov r13, r10, lsl #28
	//ldrb r13, [r12, r13, lsr #28]
	//ldr r12,= (0x06202000 + 32 * 11)
	//strh r13, [r12, #14]

	bic r10, r9, #0x0E000000
	//ensure block d is mapped to the arm7
	ldr r12,= 0x4000243
	mov r13, #0x8A
	strb r13, [r12]
	//send read command
	ldr r12,= 0x04000188
	ldr r13,= 0xAA5500C8
	str r13, [r12]
	str r10, [r12]	//address
	str r11, [r12]	//size

	//wait for the arm7 sync command
	ldr r13,= 0x55AAC8AC
	ldr r12,= 0x04000184
read_address_from_handler_rom_fifo_loop:
	ldr r10, [r12]
	tst r10, #(1 << 8)
	bne read_address_from_handler_rom_fifo_loop
	ldr r10,= 0x04100000
	ldr r10, [r10]	//read word from fifo
	cmp r10, r13
	bne read_address_from_handler_rom_fifo_loop

	//block d to arm9 lcdc
	ldr r12,= 0x4000243
	mov r13, #0x80
	strb r13, [r12]

	ldr r10,= 0x06868000
//read_address_from_handler_rom_cont:
	cmp r11, #2
	ldrltb r10, [r10]
	ldreqh r10, [r10]
	ldrgt r10, [r10]
	bx lr

read_address_from_handler_eeprom:
	cmp r9, #0x0E000000
	bge read_address_from_handler_sram
	mov r10, #1
	bx lr

read_address_from_handler_sram:
	sub r10, r9, #0x0B800000
	sub r10, r10, #0x00008C00
	sub r10, r10, #0x00000060
	cmp r11, #2
	ldrltb r10, [r10]
	ldreqh r10, [r10]
	ldrgt r10, [r10]
	bx lr

read_address_from_handler_rom_in_mem:
	bic r10, r9, #0x06000000
	sub r10, r10, #0x05000000
	sub r10, r10, #0x00FC0000
	cmp r11, #2
	ldrltb r10, [r10]
	ldreqh r10, [r10]
	ldrgt r10, [r10]
	bx lr

//read_address_from_handler_highio:
//	ldr r12,= 0x02040000
//	sub r12, r10, r12
//	cmp r12, #0x3B0000
//	addge r10, r12, #0x08000000
//	bge read_address_from_handler_rom
//	mov r10, #0
//	bx lr

.global address_read_table_32bit_dtcm_setup
address_read_table_32bit_dtcm_setup:
	ldr r10,= address_read_table_32bit_dtcm
	ldr r11,= address_read_table_32bit
	mov r12, #0x20C
address_read_table_32bit_dtcm_setup_loop:
	ldr r13, [r11], #4
	strh r13, [r10], #2
	subs r12, #4
	bne address_read_table_32bit_dtcm_setup_loop
	bx lr

.global address_read_table_16bit_dtcm_setup
address_read_table_16bit_dtcm_setup:
	ldr r10,= address_read_table_16bit_dtcm
	ldr r11,= address_read_table_16bit
	mov r12, #0x20C
address_read_table_16bit_dtcm_setup_loop:
	ldr r13, [r11], #4
	strh r13, [r10], #2
	subs r12, #2
	bne address_read_table_16bit_dtcm_setup_loop
	bx lr

.global address_read_table_8bit_dtcm_setup
address_read_table_8bit_dtcm_setup:
	ldr r10,= address_read_table_8bit_dtcm
	ldr r11,= address_read_table_8bit
	mov r12, #0x20C
address_read_table_8bit_dtcm_setup_loop:
	ldr r13, [r11], #4
	strh r13, [r10], #2
	subs r12, #1
	bne address_read_table_8bit_dtcm_setup_loop
	bx lr

.global read_address_nomod_8
read_address_nomod_8:
	ldrb r10, [r9]
	bx lr

.global read_address_nomod_16
read_address_nomod_16:
	ldrh r10, [r9]
	bx lr

.global read_address_nomod_32
read_address_nomod_32:
	ldr r10, [r9]
	bx lr

.global read_address_ignore
read_address_ignore:
	mov r10, #0
	bx lr

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

.global read_address_vcount
read_address_vcount:
	ldrh r10, [r9]
	cmp r10, #227
	movgt r10, #227
	bx lr

.global read_address_timer_counter
read_address_timer_counter:
	ldrh r10, [r9]
	mov r10, r10, lsl #17
	mov r10, r10, lsr #16
	bx lr

.global read_address_timer
read_address_timer:
	ldr r10, [r9]
	mov r12, r10, lsr #16
	mov r12, r12, lsl #16
	mov r13, r10, lsl #17
	orr r10, r12, r13, lsr #16
	bx lr

.global read_address_ie
read_address_ie:
	ldr r13,= 0x4000210
	ldrh r10, [r13]
	bx lr

.global read_address_ie_bottom8
read_address_ie_bottom8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x210]
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
	bx lr

.global read_address_if_bottom8
read_address_if_bottom8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x214]
	bx lr

.global read_address_if_top8
read_address_if_top8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x215]
	bx lr

.global read_address_ie_if
read_address_ie_if:
	ldr r13,= 0x4000210
	ldrh r12, [r13]
	ldrh r13, [r13, #4]
	orr r10, r12, r13, lsl #16
	bx lr

.global read_address_wait_control
read_address_wait_control:
	ldr r13,= WAITCNT_copy
	ldr r10, [r13]
	bx lr

.global read_address_wait_control_bottom8
read_address_wait_control_bottom8:
	ldr r13,= WAITCNT_copy
	ldrb r10, [r13]
	bx lr

.global read_address_wait_control_top8
read_address_wait_control_top8:
	ldr r13,= (WAITCNT_copy + 1)
	ldrb r10, [r13]
	bx lr