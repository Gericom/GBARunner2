.section .itcm

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

	//since pc points to pc+8, use 2 nops padding
//	nop
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

.global data_abort_handler
data_abort_handler:
	push {lr}
	mrs lr, spsr
	tst lr, #0x20 //thumb bit
	bne data_abort_handler_thumb
data_abort_handler_arm:
	ldr lr,= reg_table
	stmia lr!, {r0-r12}	//non-banked registers
	mov r12, lr
	mrs lr, spsr
	ands lr, lr, #0xF
	cmpne lr, #0xF
	stmeqia r12, {sp,lr}^	//read user bank registers
	beq data_abort_handler_cont
	orr lr, lr, #0x90
	msr cpsr_c, lr
	stmia r12, {sp,lr}
	msr cpsr_c, #0x97

data_abort_handler_cont:
	pop {r5}	//lr
	ldr r1,= reg_table
	sub r0, r5, #8

	mrc p15, 0, r4, c1, c0, 0
	bic r2, r4, #1	//disable pu
	bic r2, #(1 << 12) //and cache
	mcr p15, 0, r2, c1, c0, 0

	mrs r2, spsr
	mov r2, r2, lsr #29
	and r2, r2, #1
	
	bl DataAbortHandler

	mcr p15, 0, r4, c1, c0, 0

	cmp r0, #0
	addeq r5, #4

	push {r5}	//lr
	ldr r12,= (reg_table + (4 * 13))
	mrs lr, spsr
	ands lr, lr, #0xF
	cmpne lr, #0xF
	ldmeqia r12, {sp,lr}^	//write user bank registers
	beq data_abort_handler_cont2
	orr lr, lr, #0x90
	msr cpsr_c, lr
	ldmia r12, {sp,lr}
	msr cpsr_c, #0x97
	
data_abort_handler_cont2:
	ldr lr,= reg_table
	ldmia lr, {r0-r12}	//non-banked registers
	pop {lr}

	subs pc, lr, #8

data_abort_handler_thumb:
	ldr lr,= reg_table
	stmia lr, {r0-r7}	//non-banked registers
	pop {r5}	//lr
	ldr r1,= reg_table
	sub r0, r5, #8

	mrc p15, 0, r4, c1, c0, 0
	bic r2, r4, #1	//disable pu
	bic r2, #(1 << 12) //and cache
	mcr p15, 0, r2, c1, c0, 0
	
	push {r12}
	bl DataAbortHandlerThumb
	pop {r12}

	mcr p15, 0, r4, c1, c0, 0

	cmp r0, #0
	addeq lr, r5, #2
	movne lr, r5

	ldr r5,= reg_table
	ldmia r5, {r0-r7}	//non-banked registers

	subs pc, lr, #8



//calc address for ldr/str
ldr_str_address_calc:
	//r0 = instruction (and with 0x0FFFFFFF), r1 = register table
	and r8, r0, #(0xF << 16)
	ldr r9, [r1, r8, lsr #14]
	tst r0, #(1 << 25)
	moveq r10, r0, lsl #20
	moveq r10, r10, lsr #20
	beq ldr_str_address_calc_cont
	and r10, r0, #0xF
	ldr r10, [r1, r10, lsl #2]
	//construct shift (mov r10, r10, xxx #y)
	and r11, r0, #0xFE0
	orr r11, r11, #0xE000000A
	orr r11, r11, #0x01A00000
	orr r11, r11, #0x0000A000
	str r11, [pc]
	//to flush the pipeline
	b ldr_str_address_calc_shift
ldr_str_address_calc_shift:
	nop

ldr_str_address_calc_cont:
	tst r0, #(1 << 23)
	sbceq r10, r10, #0
	tst r0, #(1 << 24)
	addne r9, r10

