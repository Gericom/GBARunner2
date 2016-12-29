.section .vram

.include "consts.s"

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

.global count_bits_initialize
count_bits_initialize:
	ldr r0,= 0x10000040
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
	ldr r2,= 0x10000040
	and r1, r0, #0xFF
	ldrb r1, [r2, r1]
	ldrb r0, [r2, r0, lsr #8]
	add r0, r0, r1
	bx lr

count_bits_set_8_lookup:
	ldr r1,= 0x10000040
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

.global nibble_to_char
nibble_to_char:
	.byte	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46

.global undef_inst_handler_vram
undef_inst_handler_vram:
	mrc p15, 0, r0, c1, c0, 0
	bic r0, #(1 | (1 << 2))	//disable pu and data cache
	bic r0, #(1 << 12) //and cache
	mcr p15, 0, r0, c1, c0, 0

	ldr r0,= 0x06202000
	ldr r1,= 0x46444E55
	str r1, [r0]

	mov r0, lr
	ldr r1,= nibble_to_char
	ldr r4,= (0x06202000 + 32 * 8)
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

	ldr r0, [lr]
	ldr r1,= nibble_to_char
	ldr r4,= (0x06202000 + 32 * 9)
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

	mrs r0, spsr
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

	msr cpsr_c, #0x9F
	mov r0, lr
	msr cpsr_c, #0x9B
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

undef_inst_handler_loop:
	b undef_inst_handler_loop