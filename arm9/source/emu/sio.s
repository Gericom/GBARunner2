.section .itcm

#include "consts.s"

.global read_address_sio_cnt_32
read_address_sio_cnt_32:
	//store register value into shadow register
	ldr r13,= ((sio_work + 8) | 0x00800000) //uncached start of shadow registers - register base
	ldr r10, [r13]
	bic r10, #0xFC
	ldrh r11, [r13, #6]
	orr r10, r11
	bx lr

.global read_address_sio_cnt_16
read_address_sio_cnt_16:
	//store register value into shadow register
	ldr r13,= ((sio_work + 8) | 0x00800000) //uncached start of shadow registers - register base
	ldrh r10, [r13]
	bic r10, #0xFC
	ldrh r11, [r13, #6]
	orr r10, r11
	bx lr

.global read_address_sio_8
read_address_sio_8:
	//store register value into shadow register
	ldr r13,= ((sio_work - 0x120) | 0x00800000) //uncached start of shadow registers - register base
	and r12, r9, #0xFFFFFF
	ldrb r10, [r13, r12]
	bx lr

.global read_address_sio_16
read_address_sio_16:
	//store register value into shadow register
	ldr r13,= ((sio_work - 0x120) | 0x00800000) //uncached start of shadow registers - register base
	and r12, r9, #0xFFFFFF
	ldrh r10, [r13, r12]
	bx lr

.global read_address_sio_32
read_address_sio_32:
	//store register value into shadow register
	ldr r13,= ((sio_work - 0x120) | 0x00800000) //uncached start of shadow registers - register base
	and r12, r9, #0xFFFFFF
	ldr r10, [r13, r12]
	bx lr

.global write_address_sio_16
write_address_sio_16:
	//store register value into shadow register
	ldr r13,= ((sio_work - 0x120) | 0x00800000) //uncached start of shadow registers - register base
	and r12, r9, #0xFFFFFF

	cmp r12, #0x128
	bne 1f
	
	//tst r11, #0x80
	//ldrh r10, [r13, #6]
	//orr r10, #0x80
	//strh r10, [r13, #6]

1:
	strh r11, [r13, r12]


	ldr r13,= 0x04000188
2:
	ldr r12, [r13, #-4]
	tst r12, #1
	beq 2b
	str r9, [r13] //command
	str r11, [r13] //val
	bx lr

.global read_address_sio_rcnt_16
read_address_sio_rcnt_16:
	//store register value into shadow register
	ldr r13,= ((sio_work + 12) | 0x00800000) //uncached start of shadow registers - register base
	ldrh r10, [r13]
	bx lr

.global write_address_sio_rcnt_8_hi
write_address_sio_rcnt_8_hi:
	//store register value into shadow register
	ldr r13,= ((sio_work + 12) | 0x00800000) //uncached start of shadow registers - register base
	ldrh r12, [r13]
	bic r12, #0xFF00
	orr r11, r12, r11, lsl #8
	b write_address_sio_rcnt_16_cont

.global write_address_sio_rcnt_16
write_address_sio_rcnt_16:
	//store register value into shadow register
	ldr r13,= ((sio_work + 12) | 0x00800000) //uncached start of shadow registers - register base
write_address_sio_rcnt_16_cont:
	strh r11, [r13]

	ldr r13,= 0x04000188
1:
	ldr r12, [r13, #-4]
	tst r12, #1
	beq 1b
	bic r12, r9, #1
	str r12, [r13] //command
	str r11, [r13] //val
	bx lr