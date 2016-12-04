.section .itcm
.altmacro

//#define DEBUG_ABORT_ADDRESS

//data_abort_handler_new:
//	push {lr}
//	mrs lr, spsr
//	tst lr, #0x20 //thumb bit
//	bne data_abort_handler_new_thumb
//data_abort_handler_new_arm:
//	ldr lr, [sp]
//	ldr lr, [lr]
//	and lr, lr, #(7 << 25)
//	add pc, lr, lsr #23

//	nop
//	b data_abort_handler_new_arm_half_load_store
//	b data_abort_handler_new_arm_unk
//	b data_abort_handler_new_arm_single_load_store
//	b data_abort_handler_new_arm_single_load_store
//	b data_abort_handler_new_arm_block_load_store
//	b data_abort_handler_new_arm_unk
//	b data_abort_handler_new_arm_unk
//	b data_abort_handler_new_arm_unk

//data_abort_handler_new_arm_half_load_store:
//	pop {lr}
//	b data_abort_handler
	
//data_abort_handler_new_arm_single_load_store:
	//change the instruction to quickly get the address with the processor
//	ldr lr, [sp]
//	ldr lr, [lr]
//	bic lr, lr, #0xF0000000 //condition
//	bic lr, lr, #0x0000F000 //dst register
//	tst lr, #(1 << 24)
//	biceq lr, lr, #0xFFF00FFF //#0xF3000FFF is enough
//	orreq lr, lr, #0x05000000
//	orr lr, lr, #0xE0000000 //condition
//	orr lr, lr, #0x00300000 //writeback and load
//	str lr, [pc, #0x28]
//	mov lr, lr, lsr #16
//	and lr, lr, #0xF
//	orr lr, lr, #0xE1000000
//	orr lr, lr, #0x00A00000	//mov r0, rb
//	str lr, [pc, #0x20]

//	push {r0-r2}
//	mrs lr, spsr
//	ands lr, lr, #0xF
//	moveq lr, #0xF
//	orr lr, lr, #0x90
//	msr cpsr_c, lr
//	nop //move instruction here
//	nop //move instruction here
//	nop	//load instruction will be placed here
//	nop //move instruction will be placed here
//	msr cpsr_c, #0x97
	//address is in r0 now
//	mov lr, r0, lsr #24
//	cmp lr, #0xE

//	pop {r0-r2}

//data_abort_handler_new_arm_block_load_store:
//	pop {lr}
//	b data_abort_handler

//data_abort_handler_new_arm_unk:
//	pop {lr}
//	b data_abort_handler
	//b data_abort_handler_new_arm_unk


//data_abort_handler_new_thumb:
//	pop {lr}

reg_table = 0x10000000

//when r15 is used as destination, problems will arise, as it's currently not supported

//BUG: problems arise when trying to read further than what's loaded from the rom in main memory with main memory addresses (for example when pc relative addressing is used)
//This bug makes it impossible to load the iwram in spongebob video pak = deadlock

.global data_abort_handler
data_abort_handler:
	mrs sp, spsr
	tst sp, #0x20 //thumb bit
	bne data_abort_handler_thumb
data_abort_handler_arm:
	ands sp, sp, #0xF
	cmpne sp, #0xF
	ldr sp,= reg_table
	stmeqia sp, {r0-r14}^
	stmneia sp!, {r0-r12}
	mov r5, lr
	beq data_abort_handler_cont
	mov r12, sp
	mrs sp, spsr
	orr sp, sp, #0x80
	msr cpsr_c, sp
	stmia r12, {sp,lr}

data_abort_handler_cont:
	msr cpsr_c, #0x91
	
#ifdef DEBUG_ABORT_ADDRESS
	sub r0, r5, #8
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
#endif

	ldr r11,= reg_table
	add r6, r5, #4	//pc+12
	str r6, [r11, #(4 * 15)]

	ldr r6,= 0x33333333
	mcr p15, 0, r6, c5, c0, 2

	//mrc p15, 0, r6, c1, c0, 0
	//bic r2, r6, #1
	//mcr p15, 0, r2, c1, c0, 0

	ldr r10, [r5, #-8]
	and r10, r10, #0x0FFFFFFF

	and r8, r10, #(0xF << 16)
	ldr r9, [r11, r8, lsr #14]

	//ldr pc, [pc, r10, lsr #23]
	//ldr pc, [pc, r10, lsr #21]
	ldr pc, [pc, r10, lsr #18]

	nop
.macro list_ldrh_strh_variant a,b,c,d,e
	.word ldrh_strh_address_calc_\a\b\c\d\e
.endm

.macro list_all_ldrh_strh_variants arg=0
	list_ldrh_strh_variant %((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg<0x1F
	list_all_ldrh_strh_variants %(\arg+1)
.endif
.endm
	list_all_ldrh_strh_variants

.rept 32
	.word address_calc_unknown
.endr

.macro list_ldr_str_variant a,b,c,d,e,f
	.word ldr_str_address_calc_\a\b\c\d\e\f
.endm

.altmacro
.macro list_all_ldr_str_variants arg=0
	list_ldr_str_variant %((\arg>>5)&1),%((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg<0x3F
	list_all_ldr_str_variants %(\arg+1)
.endif
.endm

	list_all_ldr_str_variants

.macro list_ldm_stm_variant a,b,c,d,e
	.word ldm_stm_address_calc_\a\b\c\d\e
.endm

.macro list_all_ldm_stm_variants arg=0
	list_ldm_stm_variant %((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg<0x1F
	list_all_ldm_stm_variants %(\arg+1)
.endif
.endm
	list_all_ldm_stm_variants
.rept 96
	.word address_calc_unknown
.endr

.global data_abort_handler_cont_finish
data_abort_handler_cont_finish:
	msr cpsr_c, #0x97

	ldr r6,= 0x33660003
	mcr p15, 0, r6, c5, c0, 2

	//mcr p15, 0, r6, c1, c0, 0

	//push {r5}	//lr
	mrs sp, spsr
	//ldr r12,= (reg_table + (4 * 13))
	ldr r12,= reg_table
	ands sp, sp, #0xF
	cmpne sp, #0xF
	//cmpne sp, #0xF
	//ldmeqia r12, {sp,lr}^	//write user bank registers
	beq data_abort_handler_cont3
	orr sp, sp, #0x90
	msr cpsr_c, sp
	ldmia r12, {r0-r14}
	//ldmia r12, {sp,lr}
	msr cpsr_c, #0x97
	
data_abort_handler_cont2:
	//ldr sp,= reg_table
	//ldmia sp, {r0-r12}	//non-banked registers
	//ldr lr, [lr, #(4 * 15)]
	//cmp lr, #0
	//bne data_abort_handler_r15_dst
	//pop {lr}

	subs pc, lr, #4

data_abort_handler_cont3:
	//sub r12, #(4 * 13)
	ldmia r12, {r0-r14}^
	//ldmia r12, {sp,lr}^	//write user bank registers
	//sub sp, r12, #(4 * 13)
	//ldmia sp, {r0-r12}	//non-banked registers
	//ldr lr, [lr, #(4 * 15)]
	//cmp lr, #0
	//bne data_abort_handler_r15_dst
	//pop {lr}
	nop
	subs pc, lr, #4

//data_abort_handler_r15_dst:
//	pop {lr}
//	mov r0, lr
//	bl print_address
//	b .

data_abort_handler_thumb:
	ldr sp,= reg_table
	//stmia sp!, {r0-r7}	//non-banked registers
	str lr, [sp, #(8 << 2)]

#ifdef DEBUG_ABORT_ADDRESS
	sub r0, lr, #8
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

#endif

	ldr sp,= 0x33333333
	mcr p15, 0, sp, c5, c0, 2

	//mrc p15, 0, sp, c1, c0, 0
	//bic sp, #1
	//mcr p15, 0, sp, c1, c0, 0
	
	msr cpsr_c, #0x91
	ldr r11,= reg_table
	ldr r10, [r11, #(8 << 2)]
	ldrh r10, [r10, #-8]

	ldr pc, [pc, r10, lsr #11]

	nop

	.word address_calc_unknown
	.word address_calc_unknown
	.word thumb7_8_address_calc
	.word thumb9_address_calc
	.word thumb10_address_calc
	.word address_calc_unknown
	.word thumb15_address_calc
	.word address_calc_unknown

address_calc_unknown:
	ldr r0,= 0x06202000
	ldr r1,= 0x4B4E5541
	str r1, [r0]
	b .


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