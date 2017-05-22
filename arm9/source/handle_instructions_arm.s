.section .itcm
.altmacro

#include "consts.s"

.macro create_ldrh_strh_variant p, u, i, w, l
.global ldrh_strh_address_calc_\p\u\i\w\l
ldrh_strh_address_calc_\p\u\i\w\l:
	//and r8, r10, #(0xF << 16)
	//ldr r9, [r11, r8, lsr #14]
	and r0, r10, #0xF
.ifeq \i
	ldr r0, [r11, r0, lsl #2]
.else
	and r1, r10, #0xF00
	orr r0, r1, lsr #4
.endif
.ifeq \p
	.ifeq \u
		rsb r0, r0, #0
	.endif
.else
	.ifeq \u
		sub r9, r9, r0
	.else
		add r9, r9, r0
	.endif
.endif
	b ldrh_strh_address_calc_cont_\p\w\l
.endm

.macro create_all_ldrh_strh_variants arg=0
	create_ldrh_strh_variant %((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg<0x1F
	create_all_ldrh_strh_variants %(\arg+1)
.endif
.endm

create_all_ldrh_strh_variants

ldrh_strh_address_calc_cont_000:
ldrh_strh_address_calc_cont_010:
	mov r7, r11
	and r12, r10, #(0xF << 12)
	mov r12, r12, lsr #10
	ldrh r11, [r11, r12]
	bl write_address_from_handler_16bit
	add r9, r0
	str r9, [r7, r8, lsr #14]
	b data_abort_handler_cont_finish

ldrh_strh_address_calc_cont_001:
ldrh_strh_address_calc_cont_011:
	mov r1, r10
	mov r7, r11
	and r4, r10, #(3 << 5)
	cmp r4, #(2 << 5)
	beq 2f
	tst r9, #1
	cmpne r4, #(1 << 5)
	bne 2f
	bl read_address_from_handler_16bit
	cmp r4, #(1 << 5)
	movne r10, r10, lsl #16
	movne r10, r10, asr #16
	and r11, r1, #(0xF << 12)
	str r10, [r7, r11, lsr #10]

	add r9, r0
	str r9, [r7, r8, lsr #14]
	b data_abort_handler_cont_finish
2:
	bl read_address_from_handler_8bit
	mov r10, r10, lsl #24
	mov r10, r10, asr #24
	and r11, r1, #(0xF << 12)
	str r10, [r7, r11, lsr #10]

	add r9, r0
	str r9, [r7, r8, lsr #14]
	b data_abort_handler_cont_finish


ldrh_strh_address_calc_cont_110:
	str r9, [r11, r8, lsr #14]
ldrh_strh_address_calc_cont_100:
	and r12, r10, #(0xF << 12)
	mov r12, r12, lsr #10
	ldrh r11, [r11, r12]
	bl write_address_from_handler_16bit
	b data_abort_handler_cont_finish

ldrh_strh_address_calc_cont_111:
	str r9, [r11, r8, lsr #14]
ldrh_strh_address_calc_cont_101:
	mov r1, r10
	mov r7, r11
	and r4, r10, #(3 << 5)
	cmp r4, #(2 << 5)
	beq 2f
	tst r9, #1
	cmpne r4, #(1 << 5)
	bne 2f
	bl read_address_from_handler_16bit
	cmp r4, #(1 << 5)
	movne r10, r10, lsl #16
	movne r10, r10, asr #16
	and r11, r1, #(0xF << 12)
	str r10, [r7, r11, lsr #10]
	b data_abort_handler_cont_finish
2:
	bl read_address_from_handler_8bit
	mov r10, r10, lsl #24
	mov r10, r10, asr #24
	and r11, r1, #(0xF << 12)
	str r10, [r7, r11, lsr #10]
	b data_abort_handler_cont_finish

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

.macro create_ldr_str_variant i, p, u, bw, w, l
.global ldr_str_address_calc_\i\p\u\bw\w\l
ldr_str_address_calc_\i\p\u\bw\w\l:
.ifeq \i
	//immediate
	//and r8, r10, #(0xF << 16)
	//ldr r9, [r11, r8, lsr #14]
	mov r0, r10, lsl #20
	mov r0, r0, lsr #20
.else
	//shifted register
	and r0, r10, #0xF
	ldr r0, [r11, r0, lsl #2]
	//construct shift (mov r0, r0, xxx #y)
	and r1, r10, #0xFE0
	strh r1, 1f
	//keep in mind that there should be enough instructions here for the pipeline not to read the instruction too early
	//fix c-flag
	//this is the wrong c-flag, since we're in fiq mode, not in abt mode
	//TODO: fix this!
	//mrs r12, spsr
	//msr cpsr_f, r12
	//and r8, r10, #(0xF << 16)
	//ldr r9, [r11, r8, lsr #14]
	//b ldr_str_address_calc_shift_instruction
	b 1f
1:
	mov r0, r0
.endif	
.ifeq \p
	.ifeq \u
		rsb r0, r0, #0
	.endif
.else
	.ifeq \u
		sub r9, r9, r0
	.else
		add r9, r9, r0
	.endif
.endif
//align if 32 bit write
.if !\bw && !\l
	bic r9, r9, #3
.endif

.if \p && \w //pre
	str r9, [r11, r8, lsr #14]
.endif
	and r1, r10, #(0xF << 12)
.if !\p || \l //post or read
	mov r7, r11
.endif
.ifeq \l //write
	.if \bw
		ldrb r11, [r11, r1, lsr #10]
		bl write_address_from_handler_8bit
	.else
		ldr r11, [r11, r1, lsr #10]
		bl write_address_from_handler_32bit
	.endif
.else
	.if \bw
		bl read_address_from_handler_8bit
	.else
		bl read_address_from_handler_32bit
	.endif
	str r10, [r7, r1, lsr #10]
.endif
.ifeq \p
	add r9, r0
	str r9, [r7, r8, lsr #14]
.endif
	b data_abort_handler_cont_finish
.endm

.macro create_all_ldr_str_variants arg=0
	create_ldr_str_variant %((\arg>>5)&1),%((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg<0x3F
	create_all_ldr_str_variants %(\arg+1)
.endif
.endm

create_all_ldr_str_variants

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

.macro create_ldm_stm_variant p, u, s, w, l
.global ldm_stm_address_calc_\p\u\s\w\l
ldm_stm_address_calc_\p\u\s\w\l:
	bic r9, r9, #3
	//and r8, r10, #(0xF << 16)
	//ldr r9, [r11, r8, lsr #14]
	mov r1, r10, lsl #16
	//count nr bits
	ldr r12,= address_count_bit_table
	and r13, r1, #0xFF0000
	ldrb r13, [r12, r13, lsr #16]
	ldrb r12, [r12, r1, lsr #24]
	add r12, r12, r13
.if \w
	.ifeq \u
		sub r13, r9, r12, lsl #2
	.else
		add r13, r9, r12, lsl #2
	.endif

	//mov r2, #1
	//mov r3, r8, lsr #16
	//mov r2, r2, lsl r3
	//tst r10, r2
	//subne r2, #1
	//tstne r10, r2

	str r13, [r11, r8, lsr #14]
.endif
.ifeq \u
	sub r9, r9, r12, lsl #2
.endif
.if \p == \u
	add r9, r9, #4
.endif
	mov r4, r11
	mov r1, r1, lsr #16
1:
	tst r1, #1
	beq 2f
.if \l
	bl read_address_from_handler_32bit
	str r10, [r4]
.else
	ldr r11, [r4]
	bl write_address_from_handler_32bit
.endif
	add r9, r9, #4
2:
	add r4, r4, #4
	movs r1, r1, lsr #1
	bne 1b
	b data_abort_handler_cont_finish
.endm

.macro create_all_ldm_stm_variants arg=0
	create_ldm_stm_variant %((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg<0x1F
	create_all_ldm_stm_variants %(\arg+1)
.endif
.endm

create_all_ldm_stm_variants

address_calc_ignore_arm:
	b data_abort_handler_cont_finish