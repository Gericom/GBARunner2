.section .itcm

#include "consts.s"

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