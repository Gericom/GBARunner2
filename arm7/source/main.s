//.org 0x8000
.global main
main:
	//enable the arm7-arm9 fifo
	ldr r0,= 0x04000184
	mov r1, #0x8000
	str r1, [r0]
	//wait for the arm9 sync command with the dldi data address
	ldr r3,= 0xAA5555AA
	ldr r4,= 0x04000184
	ldr r0,= 0x04100000
fifo_loop_1:
	ldr r1, [r4]
	tst r1, #(1 << 8)
	bne fifo_loop_1
	ldr r1, [r0]	//read word from fifo
	cmp r1, r3
	bne fifo_loop_1
	ldr r1, [r0]	//dldi data address

	ldr r0,= 32 * 1024
	ldr r2,= _dldi_start
dldi_copy_loop:
	ldr r3, [r1], #4
	str r3, [r2], #4
	subs r0, #4
	bne dldi_copy_loop

	//init dldi
	ldr r3,= _DLDI_startup_ptr
	ldr r3, [r3]
	bl call_by_r3

	cmp r0, #0
	ldreq r1,= 0x04000188
	ldreq r2,= 0x4C494146
	streq r2, [r1]
	beq .

	bl sd_init

	//send done command to arm9
	ldr r0,= 0x04000188
	ldr r1,= 0x55AAAA55
	str r1, [r0]

	ldr r0,= 0x04000500
	ldr r1,= 0x807F
	str r1, [r0]
	ldr r0,= 0x04000504
	ldr r1,= 0x200
	str r1, [r0]

	ldr r0,= 0x04000404
	ldr r1,= 0x23F8000 //(0x02400000 - (1584 * 2))
	str r1, [r0]
	add r1, #1584
	str r1, [r0, #0x10]
	ldr r0,= 0x04000408
	ldr r1,= -1594 //-1253
	strh r1, [r0]
	strh r1, [r0, #0x10]
	ldr r0,= 0x0400040A
	mov r1, #0
	strh r1, [r0]
	strh r1, [r0, #0x10]
	ldr r0,= 0x0400040C
	ldr r1,= (396 * 10)
	//ldr r1,= 396 * 2
	str r1, [r0]
	str r1, [r0, #0x10]

	ldr r2,= 0x04000400
	ldr r1,= 0x0840007F
	str r2, [r1]
	str r2, [r1, #0x10]

	//ldr r8,= 0x04000400
	//ldr r7,= 0x0840007F
	//str r7, [r8]
	//orr r7, #0x80000000
	//str r7, [r8]
	
	ldr r5,= 0x04100000
	ldr r4,= 0x04000184
	ldr r6,= 0xAA55
fifo_loop:
	ldr r7, [r4]
	tst r7, #(1 << 8)
	bne fifo_loop
	ldr r7, [r5]	//read word from fifo
	mov r8, r7, lsr #16
	cmp r8, r6
	beq fifo_got_command
	bne fifo_loop

fifo_got_command:
	bic r7, r8, lsl #16
	cmp r7, #0xCB
	bgt fifo_loop
	sub r7, r7, #0xC4
	ldr pc, [pc, r7, lsl #2]
	//cmp r7, #0xC8
	//beq fifo_read_rom_command
	//cmp r7, #0xC9
	//beq fifo_read_rom_32_command
	//cmp r7, #0xC4
	//beq fifo_start_sound_command
	//cmp r7, #0xC5
	//beq fifo_stop_sound_command
	//cmp r7, #0xC6
	//beq fifo_start_sound_command2
	//cmp r7, #0xC7
	//beq fifo_stop_sound_command2
	//b fifo_loop
	nop

fifo_command_list:
.word fifo_start_sound_command
.word fifo_stop_sound_command
.word fifo_start_sound_command2
.word fifo_stop_sound_command2
.word fifo_read_rom_command
.word fifo_read_rom_32_command
.word fifo_read_rom_16_command
.word fifo_read_rom_8_command

fifo_start_sound_command:
	ldr r8,= 0x04000400
	//ldr r1,= 0x8840007F
	ldr r7, [r8]
	orr r7, #0x80000000
	str r7, [r8]
	b fifo_loop

fifo_start_sound_command2:
	ldr r8,= 0x04000410
	//ldr r1,= 0x8840007F
	ldr r7, [r8]
	orr r7, #0x80000000
	str r7, [r8]
	b fifo_loop

fifo_stop_sound_command:
	ldr r8,= 0x04000400
	ldr r7,= 0x0840007F
	str r7, [r8]
	b fifo_loop

fifo_stop_sound_command2:
	ldr r8,= 0x04000410
	ldr r7,= 0x0840007F
	str r7, [r8]
	b fifo_loop

fifo_sndctrl_top_command:
	ldr r7, [r5]
	mov r8, r7, lsr #8
	and r8, #3
	ldr r6,= pan_table
	ldrb r6, [r6, r8]
	mov r6, r6, lsl #16
	ldr r8,= 0x04000400
	ldr r7,= 0x8840007F
	bic r7, #(0x7F << 16)
	orr r7, r6
	str r7, [r8]

	mov r8, r7, lsr #12
	and r8, #3
	ldr r6,= pan_table
	ldrb r6, [r6, r8]
	mov r6, r6, lsl #16
	ldr r8,= 0x04000410
	ldr r7,= 0x8840007F
	bic r7, #(0x7F << 16)
	orr r7, r6
	str r7, [r8]

	b fifo_loop

fifo_read_rom_command:
	ldr r7, [r4]
	tst r7, #(1 << 8)
	bne fifo_read_rom_command
	ldr r0, [r5]	//address

fifo_read_rom_command_loop2:
	ldr r7, [r4]
	tst r7, #(1 << 8)
	bne fifo_read_rom_command_loop2

	ldr r1, [r5]	//size
	bl read_gba_rom
	//send ready ack
	ldr r1,= 0x55AAC8AC
	str r1, [r4, #4]
	b fifo_loop

fifo_read_rom_32_command:
	ldr r7, [r4]
	tst r7, #(1 << 8)
	bne fifo_read_rom_32_command
	ldr r0, [r5]	//address
	bl sdread32
	str r0, [r4, #4]
	b fifo_loop

fifo_read_rom_16_command:
	ldr r7, [r4]
	tst r7, #(1 << 8)
	bne fifo_read_rom_16_command
	ldr r0, [r5]	//address
	bl sdread16
	str r0, [r4, #4]
	b fifo_loop

fifo_read_rom_8_command:
	ldr r7, [r4]
	tst r7, #(1 << 8)
	bne fifo_read_rom_8_command
	ldr r0, [r5]	//address
	bl sdread8
	str r0, [r4, #4]
	b fifo_loop

pan_table:
.byte 0
.byte 127
.byte 0
.byte 64

.align 4

//this is armv4, so there is no blx instruction
//by using this kind of functions it's easier to call a register
call_by_r3:
	bx r3