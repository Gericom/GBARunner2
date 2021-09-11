#ifdef SELF_MODIFYING_MIX
.section .itcm
.altmacro

#include "consts.s"

.macro arml_return
	msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x17)

	ldr lr, [r13, #4] //pu_data_permissions
	mcr p15, 0, lr, c5, c0, 2

	//assume the dtcm is always accessible
	ldr lr, [r13], #(-4 * 15 - 1)

	subs pc, lr, #4
.endm

.macro make_arml_instLdrStr reg, pre, up, byte, wrback, load
	.if !\pre && \wrback
		.exitm
	.endif
.global arml_instLdrStr_\reg\pre\up\byte\wrback\load
arml_instLdrStr_\reg\pre\up\byte\wrback\load:
	.if !\reg
		//immediate, make add (r9/rd), rn, r8, lsr #12 or sub (r9/rd), rn, r8, lsr #12
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
			orr r8, #0x0A //shift of 20
			strb r8, (1f + 1)
		.endif
		and r8, r10, #0x0000F000
		.if \load
			mov r8, r8, lsr #8
			strb r8, (4f + 1)
		.else
			mov r8, r8, lsr #12
			.if \byte
				strb r8, write_address_from_handler_8bit_selfmodify //4f
			.else
				strb r8, write_address_from_handler_32bit_selfmodify
			.endif
		.endif

		mov r8, r10, lsl #20	
		.if !\pre
		2:
			mov r9, r0
		.endif
	1:
		add r9, r0, r8, lsr #20
		.if \pre && \wrback
		3:
			mov r9, r0
		.endif
	.else
		//shifted register, convert opcode to add r9, rn, rm, xxx, #xx or sub r9, rn, rm, xxx, #xx
		ldr r8, [r12, #0x64] //0x000F0FFF
		.if !\pre || (\pre && \wrback)
			.if \up
				ldr r11, [r12, #0x54] //0xE0800000 add
			.else
				ldr r11, [r12, #0x58] //0xE0400000 sub
			.endif			
			and r8, r10, r8
			mov r9, r8, lsr #16
			orr r8, r9, lsl #12	//rd = base reg (rn)			
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
			.if \byte
				strb r8, write_address_from_handler_8bit_selfmodify
			.else
				strb r8, write_address_from_handler_32bit_selfmodify
			.endif
		.endif

		//todo: fix c-flag
		//msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x17)
		//mrs r8, spsr
		//msr cpsr_c, #(CPSR_IRQ_FIQ_BITS | 0x11)
		//msr cpsr_f, r8
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
		.if \byte
			bl read_address_from_handler_8bit
		.else
			bl read_address_from_handler_32bit
		.endif
	4:
		mov r0, r10
	.else
		.if \byte
			bl write_address_from_handler_8bit_selfmodify
		.else
			bl write_address_from_handler_32bit_selfmodify
		.endif
	.endif
	arml_return
.endm

.macro makeAll_arml_instLdrStr arg=0
	make_arml_instLdrStr %((\arg>>5)&1),%((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg<0x3F
	makeAll_arml_instLdrStr %(\arg+1)
.endif
.endm

makeAll_arml_instLdrStr

.pool
#endif