.section .dtcm2
.global reg_table_dtcm
reg_table_dtcm:
.rept 16
	.word 0
.endr

.global cpu_mode_switch_dtcm
cpu_mode_switch_dtcm:
.rept 15
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

//for some reason the file is ignored without this nop here
nop