.section .itcm

reg_table = 0x10000000

.macro finish_handler
	msr cpsr_c, #0x97
	//orr sp, #1
	//mcr p15, 0, sp, c1, c0, 0
	ldr sp,= 0x33660003
	mcr p15, 0, sp, c5, c0, 2

	ldr sp,= reg_table
	ldmia sp, {r0-r7}	//non-banked registers

	subs pc, lr, #8
.endm

.macro finish_handler_skip_op
	msr cpsr_c, #0x97
	//orr sp, #1
	//mcr p15, 0, sp, c1, c0, 0
	ldr sp,= 0x33660003
	mcr p15, 0, sp, c5, c0, 2

	ldr sp,= reg_table
	ldmia sp, {r0-r7}	//non-banked registers

	subs pc, lr, #6
.endm

.macro finish_handler_self_modifying
	msr cpsr_c, #0x97
	//orr sp, #1
	//mcr p15, 0, sp, c1, c0, 0
	ldr sp,= 0x33660003
	mcr p15, 0, sp, c5, c0, 2

	subs pc, lr, #8
.endm

.macro finish_handler_skip_op_self_modifying
	msr cpsr_c, #0x97
	//orr sp, #1
	//mcr p15, 0, sp, c1, c0, 0
	ldr sp,= 0x33660003
	mcr p15, 0, sp, c5, c0, 2

	subs pc, lr, #6
.endm

//we may not use r5 and r6 here
.global thumb7_8_address_calc
thumb7_8_address_calc:
	and r12, r10, #(7 << 3)
	and r13, r10, #(7 << 6)
	orr r8, r12, r13, lsl #13
	ldr r9,= 0xE0809000
	orr r8, r9, r8, lsr #3
	str r8, thumb7_8_address_calc_op1
	b thumb7_8_address_calc_op1
thumb7_8_address_calc_op1:
	.word 0

thumb7_8_address_calc_cont:
	tst r10, #(1 << 9)
	bne thumb8_address_calc
thumb7_address_calc:
	tst r10, #(1 << 11)
	beq thumb7_address_calc_write
	and r8, r10, #7
	mov r8, r8, lsl #4
	strb r8, (thumb7_address_calc_op1 + 1)
	tst r10, #(1 << 10)
	mov r11, #4
	movne r11, #1
	bl read_address_from_handler
thumb7_address_calc_op1:
	mov r0, r10
	finish_handler_skip_op_self_modifying

thumb7_address_calc_write:
	tst r10, #(1 << 10)
	and r10, r10, #7
	strb r10, thumb7_address_calc_write_op1
	b thumb7_address_calc_write_op1
thumb7_address_calc_write_op1:
	mov r11, r0
	mov r12, #4
	movne r12, #1
	andne r11, r11, #0xFF
	bl write_address_from_handler
	finish_handler_skip_op_self_modifying

thumb8_address_calc:
	ands r8, r10, #(3 << 10)
	beq thumb8_address_calc_write
thumb8_address_calc_read:
	and r13, r10, #7
	mov r13, r13, lsl #4
	strb r13, (thumb8_address_calc_read_cont_op1 + 1)
	cmp r8, #(1 << 10)
	mov r11, #2
	moveq r11, #1
	bl read_address_from_handler
	cmp r8, #(2 << 10)
	beq thumb8_address_calc_read_cont
	cmp r8, #(1 << 10)
	mov r10, r10, lsl #16
	moveq r10, r10, lsl #8
	mov r10, r10, asr #16
	moveq r10, r10, asr #8
thumb8_address_calc_read_cont:
thumb8_address_calc_read_cont_op1:
	mov r0, r10
	finish_handler_skip_op_self_modifying

thumb8_address_calc_write:
	and r10, r10, #7
	strb r10, thumb8_address_calc_write_op1
	b thumb8_address_calc_write_op1
thumb8_address_calc_write_op1:
	mov r11, r0, lsl #16
	mov r11, r11, lsr #16
	bl write_address_from_handler_16bit
	finish_handler_skip_op_self_modifying

.global thumb9_address_calc
thumb9_address_calc:
	and r8, r10, #(7 << 3)
	mov r8, r8, lsr #3
	strb r8, thumb9_address_calc_op1
	b thumb9_address_calc_op1
thumb9_address_calc_op1:
	mov lr, r0
	and r12, r10, #(31 << 6)
	tst r10, #(1 << 12)
	addeq r9, lr, r12, lsr #4
	addne r9, lr, r12, lsr #6

thumb9_address_calc_cont:
	tst r10, #(1 << 11)
	beq thumb9_address_calc_write
	and r8, r10, #7
	mov r8, r8, lsl #4
	strb r8, (thumb9_address_calc_cont_op1 + 1)
	tst r10, #(1 << 12)
	mov r11, #4
	movne r11, #1
	bl read_address_from_handler
thumb9_address_calc_cont_op1:
	mov r0, r10
	finish_handler_skip_op_self_modifying

thumb9_address_calc_write:
	tst r10, #(1 << 12)
	and r10, r10, #7
	strb r10, thumb9_address_calc_write_op1
	b thumb9_address_calc_write_op1
thumb9_address_calc_write_op1:
	mov r11, r0
	mov r12, #4
	movne r12, #1
	andne r11, r11, #0xFF
	bl write_address_from_handler
	finish_handler_skip_op_self_modifying
	
.global thumb10_address_calc
thumb10_address_calc:
	and r8, r10, #(7 << 3)
	mov r8, r8, lsr #3
	strb r8, thumb10_address_calc_op1
	b thumb10_address_calc_op1
thumb10_address_calc_op1:
	mov lr, r0
	and r12, r10, #(31 << 6)
	add r9, lr, r12, lsr #5

thumb10_address_calc_cont:
	tst r10, #(1 << 11)
	beq thumb10_address_calc_write
thumb10_address_calc_read:
	and r8, r10, #7
	mov r8, r8, lsl #4
	strb r8, (thumb10_address_calc_cont_op1 + 1)
	bl read_address_from_handler_16bit
thumb10_address_calc_cont_op1:
	mov r0, r10
	finish_handler_skip_op_self_modifying

thumb10_address_calc_write:
	and r10, r10, #7
	strb r10, thumb10_address_calc_write_op1
	b thumb10_address_calc_write_op1
thumb10_address_calc_write_op1:
	mov r11, r0, lsl #16
	mov r11, r11, lsr #16
	bl write_address_from_handler_16bit
	finish_handler_skip_op_self_modifying

.global thumb15_address_calc
thumb15_address_calc:
	stmia r11, {r0-r7}	//non-banked registers
	and r8, r10, #(7 << 8)
	ldr r9, [r11, r8, lsr #6]
	bic r9, r9, #3

thumb15_address_calc_cont:
	and r1, r10, #0xFF
	ldr r12,= 0x10000040
	ldrb r13, [r12, r1]
	add lr, r9, r13, lsl #2
	str lr, [r11, r8, lsr #6]

	tst r10, #(1 << 11)
	mov r8, r11
	beq thumb15_address_calc_cont_write_loop
thumb15_address_calc_cont_load_loop:
	tst r1, #1
	beq thumb15_address_calc_cont_load_loop_cont
	bl read_address_from_handler_32bit
	str r10, [r8]
	add r9, r9, #4
thumb15_address_calc_cont_load_loop_cont:
	add r8, r8, #4
	movs r1, r1, lsr #1
	bne thumb15_address_calc_cont_load_loop
	finish_handler_skip_op

thumb15_address_calc_cont_write_loop:
	tst r1, #1
	beq thumb15_address_calc_cont_write_loop_cont
	ldr r11, [r8]
	bl write_address_from_handler_32bit
	add r9, r9, #4
thumb15_address_calc_cont_write_loop_cont:
	add r8, r8, #4
	movs r1, r1, lsr #1
	bne thumb15_address_calc_cont_write_loop
	finish_handler_skip_op

address_calc_ignore_thumb:
	finish_handler_skip_op_self_modifying