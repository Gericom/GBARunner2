.section .itcm
.altmacro

#include "consts.s"

.macro create_ldm_stm_variant p, u, s, w, l
.global ldm_stm_address_calc_\p\u\s\w\l
ldm_stm_address_calc_\p\u\s\w\l:
	mov r1, r10, lsl #16
	//count nr bits
	ldr r11,= count_bit_table_new
	and r13, r1, #0xFF0000
	ldrb r13, [r11, r13, lsr #16]
	ldrb r11, [r11, r1, lsr #24]
	bic r9, r9, #3
	add r11, r11, r13
.if \w
	.ifeq \u
		sub r13, r9, r11, lsl #2
	.else
		add r13, r9, r11, lsl #2
	.endif

	//mov r2, #1
	//mov r3, r8, lsr #16
	//mov r2, r2, lsl r3
	//tst r10, r2
	//subne r2, #1
	//tstne r10, r2

	str r13, [r12, r8, lsr #14]
.endif
.ifeq \u
	sub r9, r9, r11, lsl #2
.endif
.if \p == \u
	add r9, r9, #4
.endif
	mov r4, r12
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

.pool

//  |..3 ..................2 ..................1 ..................0|
//  |1_0_9_8_7_6_5_4_3_2_1_0_9_8_7_6_5_4_3_2_1_0_9_8_7_6_5_4_3_2_1_0|
//  |_Cond__|0_0_0|P|U|0|W|L|__Rn___|__Rd___|0_0_0_0|1|S|H|1|__Rm___| TransReg10
//  |_Cond__|0_0_0|P|U|1|W|L|__Rn___|__Rd___|OffsetH|1|S|H|1|OffsetL| TransImm10
//  |_Cond__|0_1_0|P|U|B|W|L|__Rn___|__Rd___|_________Offset________| TransImm9
//  |_Cond__|0_1_1|P|U|B|W|L|__Rn___|__Rd___|__Shift__|Typ|0|__Rm___| TransReg9

aabt_singleTransReturn:
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x17)

	ldr lr, [r13, #4] //pu_data_permissions
	mcr p15, 0, lr, c5, c0, 2

	//assume the dtcm is always accessible
	ldr lr, [r13], #(-4 * 15 - 1)

	subs pc, lr, #4

//to be done before entering below functions
	//assume instruction in r8, cpsr for reg read in spsr
//	mrc p15, 0, r14, c1, c0, 0
//	bic r12, r14, #(1 | (1 << 2))
//	mrs r13, spsr
	//get rn bits in r10
//	and r10, r8, #0x000F0000
//	mov r10, r10, lsr #12
	//get rd bits in r9
//	and r9, r8, #0x0000F000
//	mov r9, r9, lsr #8
	//patch instruction
//	bic r8, #0xF0000000 //clear condition bits
//	orr r8, #0xE0000000 //set condition to always

.macro aabt_singleTrans type, p, by, w, l, s, h
.global aabt_singleTrans_\type\p\by\w\l\s\h
.func aabt_singleTrans_\type\p\by\w\l\s\h
aabt_singleTrans_\type\p\by\w\l\s\h:
	cmp r9, r10
	eoreq r8, #0x1000
.if !\l && !\p
	//if store and post (and implicit writeback), set load
	orr r8, #(1 << 20)
.elseif \p && (!\w || !\l)
	//if pre and not writeback or store, set load and writeback
	orr r8, #((1 << 20) | (1 << 21))
.endif
	str r8, inst\@ //write the instruction for address resolving


.if !(\p && \w)
	//for backing up rn
	strb r10, (oldRnBackup\@ + 1) //replace r0 with rn
.endif
.if \p
	//for backing up new rn
	strb r10, (newRnBackup\@ + 1) //replace r0 with rn
.endif

.if \p && !\w
	//for restoring rn
	strb r10, (oldRnRestore\@ + 1) //replace r0 with rn
.endif

.if !\l
	eoreq r9, #0x10
	//for backing up rd
	strb r9, (rdBackup\@ + 1) //replace r0 with rd
	//for restoring rd
	strb r9, (rdRestore\@ + 1) //replace r0 with rd
.else
	//for writing the load result
	strb r9, (loadResult\@ + 1) //replace r0 with rd
.endif

	//disable pu and data cache
	mcr p15, 0, r12, c1, c0, 0

	//mode switch
	msr cpsr_c, r13

.if !(\p && \w)
oldRnBackup\@: //backup the original value of rn
	str r0, rnVal\@
.endif
.if !\l
rdBackup\@: //backup the original value of rd
	str r0, rdVal\@
.endif
inst\@: //modified original instruction
	.word 0
.if \p
newRnBackup\@: //backup new value of rn
	str r0, newAddress\@
.endif
.if \p && !\w
oldRnRestore\@: //restore the value of rn
	ldr r0, rnVal\@
.endif
.if !\l
rdRestore\@: //restore the value of rd
	ldr r0, rdVal\@
.endif

	//back to fiq mode
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x11)
	//restore pu and data cache enable
	mcr p15, 0, r14, c1, c0, 0
.if !\p
	//post, use old rn
	ldr r9, rnVal\@
.else
	//pre, use new rn
	ldr r9, newAddress\@
.endif

.if !\l
	//store
	.if !\type
		ldrh r11, rdVal\@
		bl write_address_from_handler_16bit
	.elseif \type && !\by
		ldr r11, rdVal\@
		bic r9, #3
		bl write_address_from_handler_32bit
	.elseif \type && \by
		ldrb r11, rdVal\@
		bl write_address_from_handler_8bit
	.endif
	b aabt_singleTransReturn
.else
	//load
	.if !\type && \h
		bl read_address_from_handler_16bit
	.elseif !\type && \s
		bl read_address_from_handler_8bit
	.elseif \type && !\by
		bl read_address_from_handler_32bit
	.elseif \type && \by
		bl read_address_from_handler_8bit
	.endif
	str r10, rnVal\@
	//mode switch
	mrs r11, spsr
	msr cpsr_c, r11
loadResult\@:
	.if !\type && \h
		.if \s
			ldrsh r0, rnVal\@
		.else
			ldrh r0, rnVal\@
		.endif
	.elseif !\type && \s
		ldrsb r0, rnVal\@
	.elseif \type && !\by
		ldr r0, rnVal\@
	.elseif \type && \by
		ldrb r0, rnVal\@
	.endif
	b aabt_singleTransReturn
.endif
rnVal\@:
	.word 0
newAddress\@:
	.word 0
rdVal\@:
	.word 0
.endfunc
.endm

.macro aabt_singleTransGenLdrStr arg=0
	aabt_singleTrans 1,%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1),0,0
.if \arg<0xF
	aabt_singleTransGenLdrStr %(\arg+1)
.endif
.endm

aabt_singleTransGenLdrStr

.pool

.macro aabt_singleTransGenLdrhStrh arg=0
.if (\arg % 4) > 0
	aabt_singleTrans 0,%((\arg>>4)&1),0,%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.endif
.if \arg<0x1F
	aabt_singleTransGenLdrhStrh %(\arg+1)
.endif
.endm

aabt_singleTransGenLdrhStrh

.pool

address_calc_ignore_arm:
	b data_abort_handler_cont_finish