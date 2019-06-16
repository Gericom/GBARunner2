.section .dtcm2
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
	.word 0 //spsr
.rept 12
	.word 0
.endr
	.word 0 //usr
	.word address_calc_unknown //fiq
	.word 0 //irq
	.word 0 //svc
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
	.word 0

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

.global arm_table
arm_table:
.macro listSingleTrans type, p, b, w, l, s, h
	.word aabt_singleTrans_\type\p\b\w\l\s\h
.endm

.macro listLdrhStrh p, arg=0
.if (\arg % 4) > 0
	listSingleTrans 0,\p,0,%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.else
	.word address_calc_unknown
.endif
.if \arg<0x3F
	listLdrhStrh \p,%(\arg+1)
.endif
.endm

listLdrhStrh 0
listLdrhStrh 1
listLdrhStrh 0
listLdrhStrh 1

.macro listLdrStr p, arg=0
	listSingleTrans 1,\p,%((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),0,0
.if \arg<0x3F
	listLdrStr \p,%(\arg+1)
.endif
.endm

listLdrStr 0
listLdrStr 1
listLdrStr 0
listLdrStr 1

.word 0

.global arm_ldm_table
arm_ldm_table:
.macro list_ldm_stm_variant a,b,c,d,e
	.word ldm_stm_address_calc_\a\b\c\d\e
.endm

.macro list_all_ldm_stm_variants arg=0
	list_ldm_stm_variant %((\arg>>4)&1),%((\arg>>3)&1),%((\arg>>2)&1),%((\arg>>1)&1),%((\arg>>0)&1)
.if \arg<0x1F
	list_all_ldm_stm_variants %(\arg+1)
.endif
.endm
	list_all_ldm_stm_variants
.rept 96
	.word address_calc_unknown
.endr

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

.global timer_shadow_regs_dtcm
timer_shadow_regs_dtcm:
.rept 4
	.short 0 //reload value
.endr

//for some reason the file is ignored without this nop here
nop
nop