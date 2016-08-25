.section .itcm

card_interface_fnt_offset = 0x10000F98
card_interface_fnt_size = 0x10000F9C
card_interface_fat_offset = 0x10000FA0
card_interface_fat_size = 0x10000FA4
card_interface_gba_rom_start = 0x10000FA8
card_interface_gba_rom_end = 0x10000FAC

.global card_interface_init
card_interface_init:
	//copy fnt and fat info to the dtcm
	push {r4, lr}
	ldr r0,= 0x04000204
	ldrh r1, [r0]
	bic r1, #(1 << 11)
	strh r1, [r0]

	ldr r0,= 0x027FFE40
	ldr r2,= card_interface_fnt_offset
	ldr r1, [r0] //fnt offset
	str r1, [r2]
	ldr r1, [r0, #4] //fnt size
	str r1, [r2, #4]
	ldr r1, [r0, #8] //fat offset
	str r1, [r2, #8]
	mov r3, r1
	ldr r1, [r0, #0xC] //fat size
	str r1, [r2, #0xC]
	mov r0, r3
	bl print_address2
	sub sp, #512
	mov r4, sp
	ldr r0, [r2, #8]
	mov r1, r4
	bl card_interface_read_rom_page
	//get rom file address
	ldr r0,= card_interface_gba_rom_start
	ldr r1, [r4]
	str r1, [r0]
	mov r3, r1
	ldr r1, [r4, #4]
	str r1, [r0, #4]
	mov r0, r3
	bl print_address
	add sp, #512
	pop {r4, lr}
	bx lr

.global card_interface_read_rom_page
card_interface_read_rom_page:
	//r0=address,r1=dst
	push {r4, lr}
	mov r4, r1
	bl card_interface_send_read_rom_page_command
	ldr r0,= 0xA1586000
	ldr r1,= 0x040001A4
	str r0, [r1]
	ldr r2,= 0x04100010
card_interface_read_rom_page_loop:
	ldr r0, [r1]
	tst r0, #0x00800000 //data ready
	ldrne r3, [r2]
	strne r3, [r4], #4
	tst r0, #0x80000000 //card start
	bne card_interface_read_rom_page_loop
card_interface_read_rom_page_finish:
	pop {r4, lr}
	bx lr
	
card_interface_send_read_rom_page_command:
	//r0=src
	mov r1, r0, lsl #24
	mov r0, r0, lsr #8
	orr r0, r0, #(0xB7 << 24)

card_interface_send_command:
	//r0=command1,r1=command2
	ldr r3,= 0x40001A0
	ldr r2,= 0xC000
	strh r2, [r3]
	mov r2, r0, lsr #24
	strb r2, [r3, #8]
	mov r2, r0, lsr #16
	strb r2, [r3, #9]
	mov r2, r0, lsr #8
	strb r2, [r3, #0xA]
	strb r0, [r3, #0xB]
	mov r2, r1, lsr #24
	strb r2, [r3, #0xC]
	mov r2, r1, lsr #16
	strb r2, [r3, #0xD]
	mov r2, r1, lsr #8
	strb r2, [r3, #0xE]
	strb r1, [r3, #0xF]
	bx lr