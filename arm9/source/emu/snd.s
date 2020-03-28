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
#ifdef USE_DSP_AUDIO
	ldr r11,= (0x602 - 0x30)
	and r10, r9, #0xFF
	add r10, r11, r10, lsr #1
	b dsp_read32
#else
	//store register value into shadow register
	ldr r13,= (sound_sound_emu_work_uncached + 0x508 - 0x60) //uncached start of shadow registers - register base
	and r12, r9, #0xFF
	ldr r10, [r13, r12]
	//todo: mask out the unreadable parts somehow
	bx lr
#endif

.global read_address_snd_16
read_address_snd_16:
#ifdef USE_DSP_AUDIO
	ldr r11,= (0x602 - 0x30)
	and r10, r9, #0xFF
	add r10, r11, r10, lsr #1
	b dsp_read16
#else
	//store register value into shadow register
	ldr r13,= (sound_sound_emu_work_uncached + 0x508 - 0x60) //uncached start of shadow registers - register base
	and r12, r9, #0xFF
	ldrh r10, [r13, r12]
	//todo: mask out the unreadable parts somehow
	bx lr
#endif

.global read_address_snd_8
read_address_snd_8:
#ifdef USE_DSP_AUDIO
	ldr r11,= (0x602 - 0x30)
	and r10, r9, #0xFF
	add r10, r11, r10, lsr #1
	mov r13, lr
	bl dsp_read16
	tst r9, #1
	andeq r10, #0xFF
	movne r10, r10, lsr #8
	bx r13
#else
	//store register value into shadow register
	ldr r13,= (sound_sound_emu_work_uncached + 0x508 - 0x60) //uncached start of shadow registers - register base
	and r12, r9, #0xFF
	ldrb r10, [r13, r12]
	//todo: mask out the unreadable parts somehow
	bx lr
#endif

.global write_address_snd_32
write_address_snd_32:
#ifdef USE_DSP_AUDIO
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r3,lr}
1:
	and r0, r9, #0xFF
	orr r0, #(4 << 16)
	mov r1, r11
	ldr r12,= dsp_sendIpcCommand
	blx r12
	cmp r0, #0
	beq 1b //loop if it failed
	pop {r0-r3,lr}
	bx lr
#else
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
#endif

.global write_address_snd_16
write_address_snd_16:
#ifdef USE_DSP_AUDIO
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r3,lr}
1:
	and r0, r9, #0xFF
	orr r0, #(2 << 16)
	mov r1, r11
	ldr r12,= dsp_sendIpcCommand
	blx r12
	cmp r0, #0
	beq 1b //loop if it failed
	pop {r0-r3,lr}
	bx lr
#else
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
#endif

.global write_address_snd_8
write_address_snd_8:
#ifdef USE_DSP_AUDIO
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r3,lr}
1:
	and r0, r9, #0xFF
	orr r0, #(1 << 16)
	mov r1, r11
	ldr r12,= dsp_sendIpcCommand
	blx r12
	cmp r0, #0
	beq 1b //loop if it failed
	pop {r0-r3,lr}
	bx lr
#else
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
#endif

#ifndef USE_DSP_AUDIO
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
#endif