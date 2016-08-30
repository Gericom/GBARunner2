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
	ldr r1,= (0x02400000 - (1584 * 2))
	str r1, [r0]
	add r1, #1584
	str r1, [r0, #0x10]
	ldr r0,= 0x04000408
	ldr r1,= -1594
	strh r1, [r0]
	strh r1, [r0, #0x10]
	ldr r0,= 0x0400040A
	mov r1, #0
	strh r1, [r0]
	strh r1, [r0, #0x10]
	ldr r0,= 0x0400040C
	ldr r1,= 396
	str r1, [r0]
	str r1, [r0, #0x10]

	ldr r2,= 0x04000400
	ldr r1,= 0x0840007F
	str r2, [r1]
	str r2, [r1, #0x10]

	//enable the arm7-arm9 fifo
	ldr r0,= 0x04000184
	mov r1, #0x8000
	str r1, [r0]
	ldr r0,= 0x04100000
	ldr r3,= 0xAA55
fifo_loop:
	ldr r1, [r0]	//read word from fifo
	mov r2, r1, lsr #16
	cmp r2, r3
	beq fifo_got_command
	bne fifo_loop

fifo_got_command:
	bic r1, r2, lsl #16
	cmp r1, #0xC4
	beq fifo_start_sound_command
	cmp r1, #0xC5
	beq fifo_stop_sound_command
	cmp r1, #0xC6
	beq fifo_start_sound_command2
	cmp r1, #0xC7
	beq fifo_stop_sound_command2
	b fifo_loop

fifo_start_sound_command:
	ldr r2,= 0x04000400
	//ldr r1,= 0x8840007F
	ldr r1, [r2]
	orr r1, #0x80000000
	str r1, [r2]
	b fifo_loop

fifo_start_sound_command2:
	ldr r2,= 0x04000410
	//ldr r1,= 0x8840007F
	ldr r1, [r2]
	orr r1, #0x80000000
	str r1, [r2]
	b fifo_loop

fifo_stop_sound_command:
	ldr r2,= 0x04000400
	ldr r1,= 0x0840007F
	str r1, [r2]
	b fifo_loop

fifo_stop_sound_command2:
	ldr r2,= 0x04000410
	ldr r1,= 0x0840007F
	str r1, [r2]
	b fifo_loop

fifo_sndctrl_top_command:
	ldr r1, [r0]
	mov r2, r1, lsr #8
	and r2, #3
	ldr r3,= pan_table
	ldrb r3, [r3, r2]
	mov r3, r3, lsl #16
	ldr r2,= 0x04000400
	ldr r1,= 0x8840007F
	bic r1, #(0x7F << 16)
	orr r1, r3
	str r1, [r2]

	mov r2, r1, lsr #12
	and r2, #3
	ldr r3,= pan_table
	ldrb r3, [r3, r2]
	mov r3, r3, lsl #16
	ldr r2,= 0x04000410
	ldr r1,= 0x8840007F
	bic r1, #(0x7F << 16)
	orr r1, r3
	str r1, [r2]

	b fifo_loop

pan_table:
.byte 0
.byte 127
.byte 0
.byte 64