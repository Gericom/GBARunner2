#ifdef SELF_MODIFYING_MIX
.section .itcm
.altmacro

#include "consts.s"

.macro make_arml_instLdrhStrh pre, up, imm, wrback, load, sign, half
	.if (!\pre && \wrback) || (\load && !\sign && !\half) || (!\load && !(!\sign && \half)) 
		.exitm
	.endif
.global arml_instLdrhStrh_\pre\up\imm\wrback\load\sign\half
arml_instLdrhStrh_\pre\up\imm\wrback\load\sign\half:
	.if \imm
		//immediate, make add (r9/rd), rn, r8 or sub (r9/rd), rn, r8
		mov r8, r10, lsl #12
		mov r9, r8, lsr #28
		.if \up
			orr r8, r9, #0x80 //add
		.else
			orr r8, r9, #0x40 //sub
		.endif
		strb r8, (1f + 2)
		.if !\pre || (\pre && \wrback)
			.if !\pre //get the base address before writing back the new address for post
				strb r9, 2f
			.else //get the new base address from the writeback reg
				strb r9, 3f
			.endif
			mov r8, r9, lsl #4 //rd = base register
			strb r8, (1f + 1)
		.endif
		and r8, r10, #0x0000F000
		.if \load
			mov r8, r8, lsr #8
			strb r8, (4f + 1)
		.else
			mov r8, r8, lsr #12
			strb r8, write_address_from_handler_16bit_selfmodify
		.endif

		and r8, r10, #0xF
		and r9, r10, #0xF00
		orr r8, r9, lsr #4
		.if !\pre
		2:
			mov r9, r0
		.endif
	1:
		add r9, r0, r8
		.if \pre && \wrback
		3:
			mov r9, r0
		.endif
	.else
		//shifted register, convert opcode to add r9, rn, rm or sub r9, rn, rm
		ldr r8, [r12, #0x50] //0x000F000F
		.if !\pre || (\pre && \wrback)
			.if \up
				ldr r11, [r12, #0x54] //0xE0800000 add
			.else
				ldr r11, [r12, #0x58] //0xE0400000 sub
			.endif
			and r8, r10, r8
			mov r9, r8, lsr #16
			orr r8, r8, lsr #4	//rd = base reg (rn)			
			.if !\pre //get the base address before writing back the new address for post
				strb r9, 2f
			.else //get the new base address from the writeback reg
				strb r9, 3f
			.endif
		.else
			.if \up
				ldr r11, [r12, #0x5C] //0xE0809000 add
			.else
				ldr r11, [r12, #0x60] //0xE0409000 sub
			.endif
			and r8, r10, r8
		.endif		
		orr r8, r11
		str r8, 1f

		and r8, r10, #0x0000F000
		.if \load
			mov r8, r8, lsr #8
			strb r8, (4f + 1)
		.else
			mov r8, r8, lsr #12
			strb r8, write_address_from_handler_16bit_selfmodify
		.endif

		.if !\pre
		2:
			mov r9, r0
		.endif
	1:
		nop
		.if \pre && \wrback
		3:
			mov r9, r0
		.endif
	.endif
	.if \load
		.if !\sign && \half
			bl read_address_from_handler_16bit
		.elseif \sign && !\half
			bl read_address_from_handler_8bit
			mov r10, r10, lsl #24
			mov r10, r10, asr #24
		.else
			bl read_address_from_handler_16bit
			tst r9, #1
			movne r10, r10, lsl #8
			mov r10, r10, lsl #16
			mov r10, r10, asr #16
			movne r10, r10, asr #8
		.endif
		4:
			mov r0, r10
	.else
		bl write_address_from_handler_16bit_selfmodify
	.endif
	arml_return
.endm

.macro makeAll_arml_instLdrhStrh load, arg=0
	make_arml_instLdrhStrh %((\arg>>5)&1),%((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),\load,%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg<0x3F
	makeAll_arml_instLdrhStrh \load,%(\arg+1)
.endif
.endm

makeAll_arml_instLdrhStrh 0

.pool

makeAll_arml_instLdrhStrh 1

.pool
#endif