.section ".dtcm2","ax"
.altmacro

#include "consts.s"

.global reg_table_dtcm
reg_table_dtcm:
.rept 16
	.word 0
.endr

.global cpu_mode_switch_dtcm
cpu_mode_switch_dtcm:
	.word pu_data_permissions
	.word data_abort_handler_cont_finish
	.word 0x08088008 //arm low instruction mask
.global dbgDatarightsTmp
dbgDatarightsTmp:
	.word 0
 //for arml_instLdrhStrh
	.word 0x000F000F //0x50
	.word 0xE0800000 //0x54
	.word 0xE0400000 //0x58
	.word 0xE0809000 //0x5C
	.word 0xE0409000 //0x60
//for arml_instLdrStr	
	.word 0x000F0FFF //0x64
.rept 5
	.word 0
.endr
	.word data_abort_handler_arm_usr_sys //usr
	.word address_calc_unknown //fiq
	.word data_abort_handler_arm_irq //irq
	.word data_abort_handler_arm_svc //svc
	.word address_calc_unknown
	.word address_calc_unknown
	.word address_calc_unknown
	.word address_calc_unknown //abt
	.word address_calc_unknown
	.word address_calc_unknown
	.word address_calc_unknown
	.word address_calc_unknown //und
	.word address_calc_unknown
	.word address_calc_unknown
	.word address_calc_unknown
	.word data_abort_handler_arm_usr_sys

.global thumb_table
thumb_table:
.rept 36
	.word address_calc_unknown
.endr
	.word thumb6_address_calc
	.word thumb6_address_calc
	.word thumb6_address_calc
	.word thumb6_address_calc
	.word thumb7_address_calc_00
	.word thumb8_address_calc_00
	.word thumb7_address_calc_01
	.word thumb8_address_calc_01
	.word thumb7_address_calc_10
	.word thumb8_address_calc_10
	.word thumb7_address_calc_11
	.word thumb8_address_calc_11
	.word thumb9_address_calc_00
	.word thumb9_address_calc_00
	.word thumb9_address_calc_00
	.word thumb9_address_calc_00
	.word thumb9_address_calc_01
	.word thumb9_address_calc_01
	.word thumb9_address_calc_01
	.word thumb9_address_calc_01
	.word thumb9_address_calc_10
	.word thumb9_address_calc_10
	.word thumb9_address_calc_10
	.word thumb9_address_calc_10
	.word thumb9_address_calc_11
	.word thumb9_address_calc_11
	.word thumb9_address_calc_11
	.word thumb9_address_calc_11
	.word thumb10_address_calc_0
	.word thumb10_address_calc_0
	.word thumb10_address_calc_0
	.word thumb10_address_calc_0
	.word thumb10_address_calc_1
	.word thumb10_address_calc_1
	.word thumb10_address_calc_1
	.word thumb10_address_calc_1
	//this may not be right, isn't this thumb11?
	.word thumb10_address_calc_0
	.word thumb10_address_calc_0
	.word thumb10_address_calc_0
	.word thumb10_address_calc_0
	.word thumb10_address_calc_1
	.word thumb10_address_calc_1
	.word thumb10_address_calc_1
	.word thumb10_address_calc_1
.rept 16
	.word address_calc_unknown
.endr
	.word thumb15_address_calc_0
	.word thumb15_address_calc_0
	.word thumb15_address_calc_0
	.word thumb15_address_calc_0
	.word thumb15_address_calc_1
	.word thumb15_address_calc_1
	.word thumb15_address_calc_1
	.word thumb15_address_calc_1
.rept 24
	.word address_calc_unknown
.endr

.word 0
.word 0

.global arm_table
arm_table:
.rept 96
	.word address_calc_unknown
.endr

.macro list_ldm_stm_variant a,b,c,d,e
	.word ldm_stm_address_calc_\a\b\c\d\e
.endm

.macro list_all_ldm_stm_variants arg=0x1F
	list_ldm_stm_variant %((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg>0
	list_all_ldm_stm_variants %(\arg-1)
.endif
.endm
	list_all_ldm_stm_variants


.macro list_ldr_str_variant a,b,c,d,e,f
	.word ldr_str_address_calc_\a\b\c\d\e\f
.endm

.macro list_all_ldr_str_variants arg=0x3F
	list_ldr_str_variant %((\arg>>5)&1),%((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg>0
	list_all_ldr_str_variants %(\arg-1)
.endif
.endm

	list_all_ldr_str_variants

.rept 32
	.word address_calc_unknown
.endr

.macro list_ldrh_strh_variant a,b,c,d,e
	.word ldrh_strh_address_calc_\a\b\c\d\e
.endm

.macro list_all_ldrh_strh_variants arg=0x1F
	list_ldrh_strh_variant %((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg>1
	list_all_ldrh_strh_variants %(\arg-1)
.endif
.endm
	list_all_ldrh_strh_variants

.global jumptab_armLo
jumptab_armLo:

	.word ldrh_strh_address_calc_00000

.macro list_arml_instLdrhStrh pre, up, imm, wrback, load, sign, half
	.if (!\pre && \wrback) || (\load && !\sign && !\half) || (!\load && !(!\sign && \half)) 
		.word address_calc_unknown
	.else
		.word arml_instLdrhStrh_\pre\up\imm\wrback\load\sign\half
	.endif
.endm

.macro listAll_arml_instLdrhStrh pre, arg=0
	list_arml_instLdrhStrh \pre,%((\arg>>5)&1),%((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg<0x3F
	listAll_arml_instLdrhStrh \pre,%(\arg+1)
.endif
.endm

listAll_arml_instLdrhStrh 0,1
listAll_arml_instLdrhStrh 1

.rept 128
	.word address_calc_unknown
.endr

.macro list_arml_instLdrStr reg, pre, up, byte, wrback, load
	.if !\pre && \wrback
		.word address_calc_unknown
		.word address_calc_unknown
		.word address_calc_unknown
		.word address_calc_unknown
	.else
		.word arml_instLdrStr_\reg\pre\up\byte\wrback\load
		.word arml_instLdrStr_\reg\pre\up\byte\wrback\load
		.word arml_instLdrStr_\reg\pre\up\byte\wrback\load
		.word arml_instLdrStr_\reg\pre\up\byte\wrback\load
	.endif
.endm

.macro listAll_arml_instLdrStr arg=0
	list_arml_instLdrStr %((\arg>>5)&1),%((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg<0x3F
	listAll_arml_instLdrStr %(\arg+1)
.endif
.endm

listAll_arml_instLdrStr


.global count_bit_table_new
count_bit_table_new:
.rept 256
	.byte 0
.endr

.global write_table_32bit_dtcm_new
write_table_32bit_dtcm_new:
.rept 132
	.short 0
.endr

.global write_table_16bit_dtcm_new
write_table_16bit_dtcm_new:
.rept 262
	.short 0
.endr

.global write_table_8bit_dtcm_new
write_table_8bit_dtcm_new:
.rept 524
	.short 0
.endr

.global read_table_32bit_dtcm_new
read_table_32bit_dtcm_new:
.rept 132
	.short 0
.endr

.global read_table_16bit_dtcm_new
read_table_16bit_dtcm_new:
.rept 262
	.short 0
.endr

.global read_table_8bit_dtcm_new
read_table_8bit_dtcm_new:
.rept 524
	.short 0
.endr

.global DISPCNT_copy
DISPCNT_copy:
	.word 0

.global BG2CNT_copy
BG2CNT_copy:
	.word 0

.global BG2PA_copy
BG2PA_copy:
	.2byte 0

.global BG2PB_copy
BG2PB_copy:
	.2byte 0

.global BG2PC_copy
BG2PC_copy:
	.2byte 0

.global BG2PD_copy
BG2PD_copy:
	.2byte 0

.global BG2X_copy
BG2X_copy:
	.word 0

.global BG2Y_copy
BG2Y_copy:
	.word 0

.global shadow_dispstat
shadow_dispstat:
	.word 0

.global WAITCNT_copy
WAITCNT_copy:
	.word 0

.global dma_shadow_regs_dtcm
dma_shadow_regs_dtcm:
.rept 4
	.word 0  //src
	.word 0  //dst
	.short 0 //count
	.short 0 //control
.endr

//dummy reload of fake timer before 0
.short 0

.global timer_shadow_regs_dtcm
timer_shadow_regs_dtcm:
.rept 4
	.short 0 //reload value
.endr

.global timer_shadow_regs_count_dtcm
timer_shadow_regs_count_dtcm:
.rept 4
	.short 0 //counter value
.endr

.align 2

//the current bios opcode that is returned if you do a protected read
//[00DCh+8] = 0xE129F000, after startup and softreset //before this address 0x27C is read
//[0134h+8] = 0xE25EF004, during irq execution
//[013Ch+8] = 0xE55EC002, after irq execution
//[0188h+8] = 0xE3A02004, after swi execution; reads between 0x1C8 and 0x274
.global gBiosOp
gBiosOp:
.word 0xE3A02004

//for some reason the file is ignored without this nop here
@ nop
@ nop