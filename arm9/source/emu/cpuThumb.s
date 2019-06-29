.section .itcm
.altmacro

#include "consts.s"

.macro finish_handler_skip_op_self_modifying
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x17)

	ldr lr, [r13, #4] //pu_data_permissions
	mcr p15, 0, lr, c5, c0, 2

	//assume the dtcm is always accessible
	ldr lr, [r13], #(-4 * 15 - 1)

	subs pc, lr, #6
.endm

.global thumb6_address_calc
thumb6_address_calc:
	and r13, r10, #(7 << 8)
	mov r13, r13, lsr #4
	strb r13, (1f + 1)
	//ldr r9, [r11, #(8 << 2)]
	sub r9, r11, #4
	bic r9, #3
	and r12, r10, #0xFF
	add r9, r9, r12, lsl #2
	bl read_address_from_handler_32bit
1:
	mov r0, r10
	finish_handler_skip_op_self_modifying

.macro create_thumb7_variant l, bw
.global thumb7_address_calc_\l\bw
thumb7_address_calc_\l\bw:
	and r12, r10, #(7 << 3)
	and r13, r10, #(7 << 6)
	ldr r9,= 0xE0809000
	orr r8, r12, r13, lsl #13
	orr r8, r9, r8, lsr #3
	str r8, 1f
	and r8, r10, #7
.ifeq \l //write
	strb r8, 2f
.else
	mov r8, r8, lsl #4
	strb r8, (2f + 1)
.endif
	b 1f
1:
	.word 0
//align if 32 bit write
.if !\bw && !\l
	//bic r9, r9, #3
.endif
.ifeq \l //write
2:
	mov r11, r0
	.ifeq \bw
		bl write_address_from_handler_32bit
	.else
		and r11, r11, #0xFF
		bl write_address_from_handler_8bit
	.endif
.else
	.ifeq \bw
		bl read_address_from_handler_32bit
	.else
		bl read_address_from_handler_8bit
	.endif
2:
	mov r0, r10
.endif
	finish_handler_skip_op_self_modifying
.endm

create_thumb7_variant 0,0
create_thumb7_variant 0,1
create_thumb7_variant 1,0
create_thumb7_variant 1,1


.macro create_thumb8_variant x, y
.global thumb8_address_calc_\x\y
thumb8_address_calc_\x\y:
	and r12, r10, #(7 << 3)
	and r13, r10, #(7 << 6)
	ldr r9,= 0xE0809000
	orr r8, r12, r13, lsl #13
	orr r8, r9, r8, lsr #3
	str r8, 1f
	and r8, r10, #7
.if !\x && !\y //strh
	strb r8, 2f
.else
	mov r8, r8, lsl #4
	strb r8, (2f + 1)
.endif
	b 1f
1:
	.word 0
.if !\x && !\y //strh
2:
	mov r11, r0, lsl #16
	mov r11, r11, lsr #16
	bl write_address_from_handler_16bit
.else
	.if !\x && \y //ldrsb
		bl read_address_from_handler_8bit
		mov r10, r10, lsl #24
		mov r10, r10, asr #24
	.else
		bl read_address_from_handler_16bit
		.if \x && \y //ldrsh
			tst r9, #1
			movne r10, r10, lsl #8
			mov r10, r10, lsl #16
			mov r10, r10, asr #16
			movne r10, r10, asr #8
		.endif
	.endif
2:
	mov r0, r10
.endif
	finish_handler_skip_op_self_modifying
.endm

create_thumb8_variant 0,0
create_thumb8_variant 0,1
create_thumb8_variant 1,0
create_thumb8_variant 1,1

.macro create_thumb9_variant bw, l
.global thumb9_address_calc_\bw\l
thumb9_address_calc_\bw\l:
	and r8, r10, #(7 << 3)
	mov r8, r8, lsr #3
	strb r8, 1f
	and r8, r10, #7
.ifeq \l //write
	strb r8, 2f
.else
	mov r8, r8, lsl #4
	strb r8, (2f + 1)
.endif
	b 1f
1:
	mov lr, r0
	and r12, r10, #(31 << 6)
.ifeq \bw
	add r9, lr, r12, lsr #4
.else
	add r9, lr, r12, lsr #6
.endif
//align if 32 bit write
.if !\bw && !\l
	bic r9, r9, #3
.endif
.ifeq \l //write
2:
	mov r11, r0
	.ifeq \bw
		bl write_address_from_handler_32bit
	.else
		and r11, r11, #0xFF
		bl write_address_from_handler_8bit
	.endif
.else
	.ifeq \bw
		bl read_address_from_handler_32bit
	.else
		bl read_address_from_handler_8bit
	.endif
2:
	mov r0, r10
.endif
	finish_handler_skip_op_self_modifying
.endm

create_thumb9_variant 0,0
create_thumb9_variant 0,1
create_thumb9_variant 1,0
create_thumb9_variant 1,1

.macro create_thumb10_variant l
.global thumb10_address_calc_\l
thumb10_address_calc_\l:
	and r8, r10, #(7 << 3)
	mov r8, r8, lsr #3
	strb r8, 1f
	and r8, r10, #7
.ifeq \l //write
	strb r8, 2f
.else //read
	mov r8, r8, lsl #4
	strb r8, (2f + 1)
.endif
	and r12, r10, #(31 << 6)
	b 1f
1:
	mov lr, r0
	add r9, lr, r12, lsr #5
.ifeq \l //write
2:
	mov r11, r0, lsl #16
	mov r11, r11, lsr #16
	bl write_address_from_handler_16bit
.else //read
	bl read_address_from_handler_16bit
2:
	mov r0, r10
.endif
	finish_handler_skip_op_self_modifying
.endm

create_thumb10_variant 0
create_thumb10_variant 1

.global thumb15_address_calc_0
thumb15_address_calc_0:
	and r8, r10, #(7 << 8)
	mov r9, r8, lsr #8
	strb r9, 1f

	mov r8, #1
	mov r8, r8, lsl r9
	tst r10, r8
	subne r8, #1
	tstne r10, r8

	//I think this piece is broken, the rb in rlist edge case
	moveq r9, r9, lsl #4
	orreq r9, #1
	streqb r9, (2f + 1)
	b 1f
1:
	mov r9, r0
	bic r9, r9, #3

	ldreq r12,= count_bit_table_new
	andeq r8, r10, #0xFF
	ldreqb r13, [r12, r8]
	mov r8, r10
2:
	addeq r0, r9, r13, lsl #2

	sub r9, r9, #4

	tst r8, #1
	movne r11, r0
	addne r9, r9, #4
	blne write_address_from_handler_32bit

	tst r8, #2
	movne r11, r1
	addne r9, r9, #4
	blne write_address_from_handler_32bit

	tst r8, #4
	movne r11, r2
	addne r9, r9, #4
	blne write_address_from_handler_32bit

	tst r8, #8
	movne r11, r3
	addne r9, r9, #4
	blne write_address_from_handler_32bit
	
	tst r8, #16
	movne r11, r4
	addne r9, r9, #4
	blne write_address_from_handler_32bit

	tst r8, #32
	movne r11, r5
	addne r9, r9, #4
	blne write_address_from_handler_32bit

	tst r8, #64
	movne r11, r6
	addne r9, r9, #4
	blne write_address_from_handler_32bit

	tst r8, #128
	movne r11, r7
	addne r9, r9, #4
	blne write_address_from_handler_32bit

	add r9, r9, #4

	and r11, r8, #(7 << 8)
	mov r11, r11, lsr #8
	mov r10, #1
	mov r10, r10, lsl r11
	tst r8, r10
	beq 12f
	sub r10, #1
	tst r8, r10
	beq 12f

	mov r11, r11, lsl #4
	strb r11, (11f + 1)
	b 11f
11:
	mov r0, r9

12:
	finish_handler_skip_op_self_modifying

.global thumb15_address_calc_1
thumb15_address_calc_1:
	and r8, r10, #(7 << 8)
	mov r9, r8, lsr #8
	strb r9, 1f

	mov r9, r9, lsl #4
	orr r9, #1
	strb r9, (2f + 1)
	b 1f
1:
	mov r9, r0
	bic r9, r9, #3

	ldr r12,= count_bit_table_new
	and r8, r10, #0xFF
	ldrb r13, [r12, r8]
2:
	add r0, r9, r13, lsl #2

	tst r8, #1
	beq 3f
	bl read_address_from_handler_32bit
	mov r0, r10
	add r9, r9, #4
3:
	tst r8, #2
	beq 4f
	bl read_address_from_handler_32bit
	mov r1, r10
	add r9, r9, #4
4:
	tst r8, #4
	beq 5f
	bl read_address_from_handler_32bit
	mov r2, r10
	add r9, r9, #4
5:
	tst r8, #8
	beq 6f
	bl read_address_from_handler_32bit
	mov r3, r10
	add r9, r9, #4
6:
	tst r8, #16
	beq 7f
	bl read_address_from_handler_32bit
	mov r4, r10
	add r9, r9, #4
7:
	tst r8, #32
	beq 8f
	bl read_address_from_handler_32bit
	mov r5, r10
	add r9, r9, #4
8:
	tst r8, #64
	beq 9f
	bl read_address_from_handler_32bit
	mov r6, r10
	add r9, r9, #4
9:
	tst r8, #128
	beq 10f
	bl read_address_from_handler_32bit
	mov r7, r10
10:
	finish_handler_skip_op_self_modifying

address_calc_ignore_thumb:
	finish_handler_skip_op_self_modifying