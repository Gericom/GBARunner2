.section .itcm.addrhack1
.altmacro

#include "consts.s"

.space (0x3F4 - (0x20 + addrhack1 - data_abort_handler))

//when r15 is used as destination, problems will arise, as it's currently not supported

//BUG: problems arise when trying to read further than what's loaded from the rom in main memory with main memory addresses (for example when pc relative addressing is used)
//This bug makes it impossible to load the iwram in spongebob video pak = deadlock

.global data_abort_handler
data_abort_handler:
#ifdef ABT_NO_FIQ
	msr cpsr_c, #0xD7	//immediately disable fiqs
#endif

	//we assume r13_abt contains the address of the dtcm - 1 (0x04EFFFFF)
	//this makes it possible to use the bottom 16 bits for unlocking the memory protection

	//make use of the backwards compatible version
	//of the data rights register, so we can use 0xXXXXFFFF instead of 0x33333333
	mcr p15, 0, r13, c5, c0, 0

	//store the value of lr and update r13 to point to the top of the register list (place of r15)
	str lr, [r13, #(4 * 15 + 1)]!

	mrs lr, spsr
	movs lr, lr, lsl #27
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x11)
	ldr r12,= reg_table	
	ldr r11, [r12, #(4 * 15)]
	bcc (abt_handleArm - 0x01000000 + 0x180000)

data_abort_handler_thumb:
	ldrh r10, [r11, #-8]
	//todo: fill up this interlock
	add r13, r12, #(address_thumb_table_dtcm - reg_table)
	ldr pc, [r13, r10, lsr #7] //todo: this construct actually involves an interlock!

.global data_abort_handler_arm_irq
data_abort_handler_arm_irq:
	str r0, [r13, #(-4 * 15)]
	sub r0, r13, #(4 * 14)
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x12)
	stmia r0, {r1-r12,sp,lr}
	b data_abort_handler_cont

.global data_abort_handler_arm_svc
data_abort_handler_arm_svc:
	str r0, [r13, #(-4 * 15)]
	sub r0, r13, #(4 * 14)
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x13)
	stmia r0, {r1-r12,sp,lr}
	b data_abort_handler_cont

.global data_abort_handler_cont_finish
data_abort_handler_cont_finish:
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x17)

	//lr still contains spsr << 27
	cmp lr, #(0x12 << 27)

	ldr lr, [r13, #4] //pu_data_permissions

	beq data_abort_handler_cont2
data_abort_handler_cont3:
	ldmdb r13, {r0-r14}^
	
	mcr p15, 0, lr, c5, c0, 2

	//assume the dtcm is always accessible
	ldr lr, [r13], #(-4 * 15 - 1)

	subs pc, lr, #4

data_abort_handler_cont2:
	sub r12, r13, #(4 * 15)
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x12)
	ldmia r12, {r0-r14}
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x17)

	mcr p15, 0, lr, c5, c0, 2

	//assume the dtcm is always accessible
	ldr lr, [r13], #(-4 * 15 - 1)

	subs pc, lr, #4

//data_abort_handler_r15_dst:
//	pop {lr}
//	mov r0, lr
//	bl print_address
//	b .

//.global data_abort_handler_thumb_pc_tmp
//data_abort_handler_thumb_pc_tmp:
//	.word 0

.global address_calc_unknown
address_calc_unknown:
//	ldr r0,= 0x06202000
//	ldr r1,= 0x4B4E5541
//	str r1, [r0]
//
//	mov r0, r10
//	ldr r1,= nibble_to_char
//	ldr r12,= (0x06202000 + 32 * 10)
//	//print address to bottom screen
//	ldrb r2, [r1, r0, lsr #28]
//	mov r0, r0, lsl #4
//	ldrb r3, [r1, r0, lsr #28]
//	mov r0, r0, lsl #4
//	orr r2, r2, r3, lsl #8
//	strh r2, [r12], #2
//
//	ldrb r2, [r1, r0, lsr #28]
//	mov r0, r0, lsl #4
//	ldrb r3, [r1, r0, lsr #28]
//	mov r0, r0, lsl #4
//	orr r2, r2, r3, lsl #8
//	strh r2, [r12], #2
//
//	ldrb r2, [r1, r0, lsr #28]
//	mov r0, r0, lsl #4
//	ldrb r3, [r1, r0, lsr #28]
//	mov r0, r0, lsl #4
//	orr r2, r2, r3, lsl #8
//	strh r2, [r12], #2
//
//	ldrb r2, [r1, r0, lsr #28]
//	mov r0, r0, lsl #4
//	ldrb r3, [r1, r0, lsr #28]
//	mov r0, r0, lsl #4
//	orr r2, r2, r3, lsl #8
//	strh r2, [r12], #2

	b .

.global abt_handleArm
abt_handleArm:	
	ldr r10, [r11, #-8]
	ldr r14, [r12, #0x48] //0x08088008
.global addrhack1
addrhack1:
	and r13, r10, pc, ror #14 //specially crafted value of pc serves as mask, pc + 8 should be 0x1803FC
	tst r10, r14	
	add r14, r12, #(address_jumptab_armLo - reg_table)
    //conditional is faster than branching here
	orreq r13, r13, lsl #13
	ldreq pc, [r14, r13, lsr #16]
handleArmHi:
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x17)
	cmp lr, #(0x12 << 27)
	beq data_abort_handler_arm_irq

.global data_abort_handler_arm_usr_sys
data_abort_handler_arm_usr_sys:
	stmdb r13, {r0-r14}^

data_abort_handler_cont:
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x11)	

#ifdef HANDLER_STATISTICS
	mov r8, r10, lsr #19
	mov r8, r8, lsl #4
	tst r10, #(1 << 15)
	orrne r8, #(1 << 3)
	tst r10, #(1 << 3)
	orrne r8, #1
	and r9, r10, #(3 << 5)
	orr r8, r9, lsr #4
	ldr r9,= STATISTICS_ADDRESS
	ldr r8, [r9, r8, lsl #2]!
	add r8, #1
	str r8, [r9]
#endif

	and r8, r10, #(0xF << 16)
	ldr r9, [r12, r8, lsr #14]

	ldr pc, [r14, -r13, lsr #18]//todo: this construct actually involves an interlock!