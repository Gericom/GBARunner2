.section .itcm

#include "consts.s"

.global write_address_snd_fifo_A
write_address_snd_fifo_A:
	ldr r12,= 0x04000188
	str r9, [r12]
	str r11, [r12]
	bx lr

.global read_address_snd_32
read_address_snd_32:
	//store register value into shadow register
	ldr r13,= (sound_sound_emu_work_uncached + 0x508 - 0x60) //uncached start of shadow registers - register base
	and r12, r9, #0xFF
	ldr r10, [r13, r12]
	//todo: mask out the unreadable parts somehow
	bx lr

.global read_address_snd_16
read_address_snd_16:
	//store register value into shadow register
	ldr r13,= (sound_sound_emu_work_uncached + 0x508 - 0x60) //uncached start of shadow registers - register base
	and r12, r9, #0xFF
	ldrh r10, [r13, r12]
	//todo: mask out the unreadable parts somehow
	bx lr

.global read_address_snd_8
read_address_snd_8:
	//store register value into shadow register
	ldr r13,= (sound_sound_emu_work_uncached + 0x508 - 0x60) //uncached start of shadow registers - register base
	and r12, r9, #0xFF
	ldrb r10, [r13, r12]
	//todo: mask out the unreadable parts somehow
	bx lr

.global write_address_snd_32
write_address_snd_32:
	//store register value into shadow register
	ldr r13,= (sound_sound_emu_work_uncached + 0x508 - 0x60) //uncached start of shadow registers - register base
	and r12, r9, #0xFF

	//clear reset bit for NRx4 registers
	cmp r12, #0x64
	cmpne r12, #0x6C
	cmpne r12, #0x74
	cmpne r12, #0x7C
	biceq r10, r11, #0x8000
	movne r10, r11

	str r10, [r13, r12]

	orr r12, #(4 << 8) //4 bytes

	ldr r13,= 0x04000188
1:
	ldr r10, [r13, #-4]
	tst r10, #1
	beq 1b
	ldr r10,= 0xAA5500FA //update gb sound reg command
	str r10, [r13] //command
	str r12, [r13] //reg + len
	str r11, [r13] //val
	bx lr

.global write_address_snd_16
write_address_snd_16:
	//store register value into shadow register
	ldr r13,= (sound_sound_emu_work_uncached + 0x508 - 0x60) //uncached start of shadow registers - register base
	and r12, r9, #0xFF

	//clear reset bit for NRx4 registers
	cmp r12, #0x64
	cmpne r12, #0x6C
	cmpne r12, #0x74
	cmpne r12, #0x7C
	biceq r10, r11, #0x8000
	movne r10, r11

	strh r10, [r13, r12]

	orr r12, #(2 << 8) //2 bytes

	ldr r13,= 0x04000188
1:
	ldr r10, [r13, #-4]
	tst r10, #1
	beq 1b
	ldr r10,= 0xAA5500FA //update gb sound reg command
	str r10, [r13] //command
	str r12, [r13]  //reg + len
	str r11, [r13] //val
	bx lr

.global write_address_snd_8
write_address_snd_8:
	//store register value into shadow register
	ldr r13,= (sound_sound_emu_work_uncached + 0x508 - 0x60) //uncached start of shadow registers - register base
	and r12, r9, #0xFF

	//clear reset bit for NRx4 registers
	cmp r12, #0x65
	cmpne r12, #0x6D
	cmpne r12, #0x75
	cmpne r12, #0x7D
	biceq r10, r11, #0x80
	movne r10, r11

	strb r10, [r13, r12]

	orr r12, #(1 << 8) //1 byte

	ldr r13,= 0x04000188
1:
	ldr r10, [r13, #-4]
	tst r10, #1
	beq 1b
	ldr r10,= 0xAA5500FA //update gb sound reg command
	str r10, [r13] //command
	str r12, [r13]  //reg + len
	str r11, [r13] //val
	bx lr

.global write_address_snd_waveram_32
write_address_snd_waveram_32:
	and r12, r9, #0xFF

	orr r12, #(4 << 8) //4 bytes

	ldr r13,= 0x04000188
1:
	ldr r10, [r13, #-4]
	tst r10, #1
	beq 1b
	ldr r10,= 0xAA5500FA //update gb sound reg command
	str r10, [r13] //command
	str r12, [r13] //reg + len
	str r11, [r13] //val
	bx lr

.global write_address_snd_waveram_16
write_address_snd_waveram_16:
	and r12, r9, #0xFF

	orr r12, #(2 << 8) //2 bytes

	ldr r13,= 0x04000188
1:
	ldr r10, [r13, #-4]
	tst r10, #1
	beq 1b
	ldr r10,= 0xAA5500FA //update gb sound reg command
	str r10, [r13] //command
	str r12, [r13] //reg + len
	str r11, [r13] //val
	bx lr

.global write_address_snd_waveram_8
write_address_snd_waveram_8:
	and r12, r9, #0xFF

	orr r12, #(1 << 8) //1 byte

	ldr r13,= 0x04000188
1:
	ldr r10, [r13, #-4]
	tst r10, #1
	beq 1b
	ldr r10,= 0xAA5500FA //update gb sound reg command
	str r10, [r13] //command
	str r12, [r13] //reg + len
	str r11, [r13] //val
	bx lr
