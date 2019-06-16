.section .itcm
.altmacro

#include "consts.s"

//#define DEBUG_ABORT_ADDRESS

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
	str lr, [r13, #0xC]
	movs lr, lr, lsl #27
	bcc data_abort_handler_arm

data_abort_handler_thumb:
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x11)
	ldr r12,= reg_table
	ldr r11, [r12, #(4 * 15)]
	ldrh r10, [r11, #-8]
	add r12, r12, #(address_thumb_table_dtcm - reg_table)
	ldr pc, [r12, r10, lsr #7]

data_abort_handler_arm:
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x11)
	ldr r12,= reg_table
	ldr r11, [r12, #(4 * 15)]
	ldr r10, [r12, #0x48]
	ldr r8, [r11, #-8]
	ands r10, #0xF
	orreq r10, #0xF
	orr r10, #(CPSR_IRQ_FIQ_BITS | 0x10)
	msr spsr_c, r10
	tst r8, #(1 << 27)	
	bne data_abort_handler_arm_ldm
	and r9, r8, #0x7F00000
	and r10, r8, #0x60
	orr r9, r10, lsl #13
	add r11, r12, r9, lsr #16

	mrc p15, 0, r14, c1, c0, 0
	bic r12, r14, #(1 | (1 << 2))
	mrs r13, spsr
	//get rn bits in r10
	and r10, r8, #0x000F0000
	mov r10, r10, lsr #12
	//get rd bits in r9
	and r9, r8, #0x0000F000
	mov r9, r9, lsr #8
	//patch instruction
	bic r8, #0xF0000000 //clear condition bits
	orr r8, #0xE0000000 //set condition to always

	ldr pc, [r11, #(address_arm_table_dtcm - reg_table)]

data_abort_handler_arm_ldm:
	str r0, [r12]
	add r0, r12, #4
	msr cpsr_c, r10
	stmia r0, {r1-r12,sp,lr}
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x11)
	add r11, r12, #(address_arm_ldm_table_dtcm - reg_table)
	and r10, r8, #0x07FFFFFF

	and r8, r10, #(0xF << 16)
	ldr r9, [r12, r8, lsr #14]

	ldr pc, [r11, r10, lsr #18]

.global data_abort_handler_cont_finish
data_abort_handler_cont_finish:
	//important! this should set the v flag to 0
	msr cpsr_fc, #(CPSR_IRQ_FIQ_BITS | 0x17)

	//lr still contains spsr << 27
	movs lr, lr, lsl #1

	bgt data_abort_handler_cont2
data_abort_handler_cont3:
	ldmdb r13, {r0-r14}^

	ldr lr, [r13, #4] //pu_data_permissions
	mcr p15, 0, lr, c5, c0, 2

	//assume the dtcm is always accessible
	ldr lr, [r13], #(-4 * 15 - 1)

	subs pc, lr, #4

data_abort_handler_cont2:
	sub r12, r13, #(4 * 15)
	mov lr, lr, lsr #28
	orr lr, lr, #(CPSR_IRQ_FIQ_BITS | 0x10)
	msr cpsr_c, lr
	ldmia r12, {r0-r14}
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x17)

	ldr lr, [r13, #4] //pu_data_permissions
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