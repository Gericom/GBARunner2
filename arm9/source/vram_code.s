.section .vram

#include "consts.s"

.global address_read_table_32bit_dtcm_setup
address_read_table_32bit_dtcm_setup:
	ldr r10,= read_table_32bit_dtcm_new
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
	ldr r10,= read_table_16bit_dtcm_new
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
	ldr r10,= read_table_8bit_dtcm_new
	ldr r11,= address_read_table_8bit
	mov r12, #0x20C
address_read_table_8bit_dtcm_setup_loop:
	ldr r13, [r11], #4
	strh r13, [r10], #2
	subs r12, #1
	bne address_read_table_8bit_dtcm_setup_loop
	bx lr

.global address_write_table_32bit_dtcm_setup
address_write_table_32bit_dtcm_setup:
	ldr r10,= write_table_32bit_dtcm_new
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
	ldr r10,= write_table_16bit_dtcm_new
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
	ldr r10,= write_table_8bit_dtcm_new
	ldr r11,= address_write_table_8bit
	mov r12, #0x20C
address_write_table_8bit_dtcm_setup_loop:
	ldr r13, [r11], #4
	strh r13, [r10], #2
	subs r12, #1
	bne address_write_table_8bit_dtcm_setup_loop
	bx lr

//.global thumb_table_dtcm_setup
//thumb_table_dtcm_setup:
//	ldr r10,= address_thumb_table_dtcm
//	ldr r11,= thumb_table
//	mov r12, #128
//1:
//	ldr r13, [r11], #4
//	str r13, [r10], #4
//	subs r12, #1
//	bne 1b
//	bx lr

.global count_bits_initialize
count_bits_initialize:
	ldr r0,= count_bit_table_new
	mov r1, #0
count_bits_initialize_loop:
	and	r3, r1, #0xAA
	sub	r2, r1, r3, lsr #1
		
	and	r3, r2, #0xCC
	and	r2, r2, #0x33
	add	r2, r2, r3, lsr #2
		
	add	r2, r2, r2, lsr #4
	and	r2, r2, #0xF
	strb r2, [r0], #1
	add r1, r1, #1
	cmp r1, #0x100
	bne count_bits_initialize_loop
	bx lr

count_bits_set_16_lookup:
	ldr r2,= count_bit_table_new
	and r1, r0, #0xFF
	ldrb r1, [r2, r1]
	ldrb r0, [r2, r0, lsr #8]
	add r0, r0, r1
	bx lr

count_bits_set_8_lookup:
	ldr r1,= count_bit_table_new
	ldrb r0, [r1, r0]
	bx lr

.global print_address
print_address:
	push {r0-r4}
	ldr r1,= nibble_to_char
	ldr r4,= (0x06202000 + 32 * 10)
	//print address to bottom screen
	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2
	pop {r0-r4}
	bx lr

.global print_address2
print_address2:
	push {r0-r4}
	ldr r1,= nibble_to_char
	ldr r4,= (0x06202000 + 32 * 11)
	//print address to bottom screen
	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2
	pop {r0-r4}
	bx lr

sp_tmp:
	.word 0

write_offset:
	.word 0

.global print_address_isnitro
print_address_isnitro:
	str r13, sp_tmp
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r6}
	mrc p15, 0, r5, c1, c0, 0
	bic r5, #1
	mcr p15, 0, r5, c1, c0, 0

	ldr r6,= 0x09F80000
	//ldr r4, [r6, #0x94]
	ldr r4, write_offset
	add r4, #0x8000
	add r4, r6

	ldr r1,= nibble_to_char
	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2

	ldrb r2, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	ldrb r3, [r1, r0, lsr #28]
	mov r0, r0, lsl #4
	orr r2, r2, r3, lsl #8
	strh r2, [r4], #2
	
	//write a space and a linebreak
	ldr r2,= 0x0A20
	strh r2, [r4], #2

	sub r4, r6
	sub r4, #0x8000
	str r4, write_offset
	str r4, [r6, #0x90]

	//wait till 0x94 == 0x90
//1:
//	ldr r3, [r6, #0x94]
//	cmp r3, r4
//	bne 1b

	orr r5, #1
	mcr p15, 0, r5, c1, c0, 0
	pop {r0-r6}
	ldr r13, sp_tmp
	bx lr

.global gba_start_bkpt_vram
gba_start_bkpt_vram:
	//fill main memory
	mov r0, #0
	ldr r1,= 0x02000000
	mov r2, #(256 * 1024)
1:
	str r0, [r1], #4
	subs r2, #4
	bne 1b

	//fill wram
	ldr r1,= 0x03000000
	mov r2, #(32 * 1024)
1:
	str r0, [r1], #4
	subs r2, #4
	bne 1b

	//set abort exception handlers
	ldr r0,= instruction_abort_handler
	sub r0, #0xC	//relative to source address
	sub r0, #8	//pc + 8 compensation
	mov r1, #0xEA000000
	orr r1, r0, lsr #2
	mov r0, #0xC
	str r1, [r0]

	ldr r0,= data_abort_handler
	sub r0, #0x10	//relative to source address
	sub r0, #8	//pc + 8 compensation
	mov r1, #0xEA000000
	orr r1, r0, lsr #2
	mov r0, #0x10
	str r1, [r0]

	ldr r0,= undef_inst_handler
	sub r0, #0x4	//relative to source address
	sub r0, #8	//pc + 8 compensation
	mov r1, #0xEA000000
	orr r1, r0, lsr #2
	mov r0, #0x4
	str r1, [r0]

//#ifdef DEBUG_ENABLED
	//for debugging
	ldr r0,= irq_handler
	sub r0, #0x18	//relative to source address
	sub r0, #8	//pc + 8 compensation
	mov r1, #0xEA000000
	orr r1, r0, lsr #2
	mov r0, #0x18
	str r1, [r0]
//#endif

	//fiq handler for is-nitro
	ldr r0,= fiq_hook
	sub r0, #0x1C	//relative to source address
	sub r0, #8	//pc + 8 compensation
	mov r1, #0xEA000000
	orr r1, r0, lsr #2
	mov r0, #0x1C
	str r1, [r0]

	//set the abort mode r13
	mrs r0, cpsr
	and r0, r0, #0xE0
	orr r1, r0, #0x17
	msr cpsr_c, r1
	ldr r13,= (address_dtcm - 1) //0x10004000
	orr r1, r0, #0x13
	msr cpsr_c, r1

	ldr r0,= count_bits_initialize
	blx r0

	ldr r0,= 0x05000000
	ldr r1,= 0x7FFF
	strh r1, [r0]

	ldr r2,= gEmuSettingMainMemICache
	ldr r2, [r2]
	cmp r2, #1
	beq 1f
	//disable main memory i-cache
	mrc p15, 0, r0, c2, c0, 1
	bic r0, #(1 << 5)
	mcr p15, 0, r0, c2, c0, 1
1:
	mrc p15, 0, r0, c1, c0, 0
	//orr r0, #(1<<15)
	orr r0, #(1 | (1 << 2))	//enable pu and data cache
	orr r0, #(1 << 12) //and cache
	orr r0, #(1 << 14) //round robin cache replacement improves worst case performance
	orr r0, #(1 << 15) //arm v4 thumb handling
	mcr p15, 0, r0, c1, c0, 0

	//invalidate instruction cache
	mov r0, #0
	mcr p15, 0, r0, c7, c5, 0

	//and data cache
	mcr p15, 0, r0, c7, c6, 0

	mcr	p15, 0, r0, c7, c10, 4

	ldr r2,= gEmuSettingSkipIntro
	ldr r2, [r2]
	cmp r2, #1
	ldrne r0,= (gGbaBios + 0x68) //with intro
	ldreq r0,= (gGbaBios + 0xB4) //without intro
	bx r0