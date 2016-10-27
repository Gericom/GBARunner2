.section .itcm

//calc address for arm ldrh/ldrsh/ldrsb/strh
.global ldrh_strh_address_calc
ldrh_strh_address_calc:
	and r8, r10, #(0xF << 16)
	ldr lr, [r11, r8, lsr #14]
	tst r10, #(1 << 22)
	and r0, r10, #0xF
	ldreq r0, [r11, r0, lsl #2]
	andne r1, r10, #0xF00
	orrne r0, r1, lsr #4

	tst r10, #(1 << 23)
	rsbeq r0, r0, #0
	tst r10, #(1 << 24)
	addne r9, lr, r0
	moveq r9, lr
	ldr pc, [pc, r9, lsr #22]

	nop
	.word address_calc_ignore_arm	//bios: ignore
	.word address_calc_ignore_arm	//itcm: ignore
	.word address_calc_ignore_arm	//main: ignore
	.word address_calc_ignore_arm	//wram: can't happen
	.word ldrh_strh_address_calc_cont	//io, manual execution
	.word address_calc_ignore_arm	//pal: can't happen
	.word ldrh_strh_address_calc_cont	//sprites vram, manual execution
	.word address_calc_ignore_arm	//oam: can't happen
	.word ldrh_strh_address_calc_fix_cartridge	//card: fix
	.word ldrh_strh_address_calc_fix_cartridge	//card: fix	
	.word ldrh_strh_address_calc_fix_cartridge	//card: fix
	.word ldrh_strh_address_calc_fix_cartridge	//card: fix
	.word ldrh_strh_address_calc_fix_cartridge	//card: fix
	.word ldrh_strh_address_calc_cont	//eeprom, manual execution
	.word ldrh_strh_address_calc_fix_sram	//sram: fix
	.word address_calc_ignore_arm	//nothing: shouldn't happen

ldrh_strh_address_calc_fix_cartridge:
	ldr r4,= 0x083B0000
	cmp r9, r4
	bge ldrh_strh_address_calc_cont
	mov r8, r8, lsr #16
	cmp lr, #0x08000000
	bge ldrh_strh_address_calc_fix_cartridge_cont
	tst r10, #(1 << 22)
	andeq r8, r10, #0xF
	ldreq lr, [r11, r8, lsl #2]
ldrh_strh_address_calc_fix_cartridge_cont:
	bic lr, lr, #0x06000000
	sub lr, lr, #0x05000000
	sub lr, lr, #0x00FC0000
	str lr, [r11, r8, lsl #2]
	b data_abort_handler_cont_finish

ldrh_strh_address_calc_fix_sram:
	mov r8, r8, lsr #16
	cmp lr, #0x0E000000
	bge ldrh_strh_address_calc_fix_sram_cont
	tst r10, #(1 << 22)
	andeq r8, r10, #0xF
	ldreq lr, [r11, r8, lsl #2]
ldrh_strh_address_calc_fix_sram_cont:
	sub lr, lr, #0x0B800000
	sub lr, lr, #0x00008C00
	sub lr, lr, #0x00000060
	str lr, [r11, r8, lsl #2]
	b data_abort_handler_cont_finish

ldrh_strh_address_calc_cont:
	tst r10, #(1 << 24)
	tstne r10, #(1 << 21)
	strne r9, [r11, r8, lsr #14]

	tst r10, #(1 << 20)
	mov r1, r10
	mov r7, r11
	bne ldrh_strh_address_calc_cont_load
	and r12, r10, #(0xF << 12)
	mov r12, r12, lsr #10
	ldrh r11, [r11, r12]
	mov r12, #2
	bl write_address_from_handler
	tst r1, #(1 << 24)
	addeq r9, r0
	streq r9, [r7, r8, lsr #14]
	add r5, #4
	b data_abort_handler_cont_finish

ldrh_strh_address_calc_cont_load:
	and r4, r10, #(3 << 5)
	cmp r4, #(2 << 5)
	mov r11, #2
	moveq r11, #1
	bl read_address_from_handler
	cmp r4, #(1 << 5)
	beq ldrh_strh_address_calc_cont2_ld
	cmp r4, #(2 << 5)
	mov r10, r10, lsl #16
	moveq r10, r10, lsl #8
	mov r10, r10, asr #16
	moveq r10, r10, asr #8
ldrh_strh_address_calc_cont2_ld:
	and r11, r1, #(0xF << 12)
	str r10, [r7, r11, lsr #10]
	tst r1, #(1 << 24)
	addeq r9, r0
	streq r9, [r7, r8, lsr #14]
	add r5, #4
	b data_abort_handler_cont_finish

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//calc address for ldr/str
.global ldr_str_address_calc
ldr_str_address_calc:
	//r10 = instruction (and with 0x0FFFFFFF), r11 = register table
	and r8, r10, #(0xF << 16)
	ldr lr, [r11, r8, lsr #14]
	tst r10, #(1 << 25)
	moveq r0, r10, lsl #20
	moveq r0, r0, lsr #20
	beq ldr_str_address_calc_cont
	and r0, r10, #0xF
	ldr r0, [r11, r0, lsl #2]
	//construct shift (mov r0, r0, xxx #y)
	and r1, r10, #0xFE0
	strh r1, ldr_str_address_calc_shift_instruction
	//keep in mind that there should be enough instructions here for the pipeline not to read the instruction too early
	//fix c-flag
	mrs r12, spsr
	msr cpsr_f, r12
	b ldr_str_address_calc_shift_instruction
ldr_str_address_calc_shift_instruction:
	.word 0xE1A00000

ldr_str_address_calc_cont:
	tst r10, #(1 << 23)
	rsbeq r0, r0, #0
	tst r10, #(1 << 24)
	addne r9, lr, r0
	moveq r9, lr
	ldr pc, [pc, r9, lsr #22]

	nop
	.word address_calc_ignore_arm	//bios: ignore
	.word address_calc_ignore_arm	//itcm: ignore
	.word address_calc_ignore_arm	//main: ignore
	.word address_calc_ignore_arm	//wram: can't happen
	.word ldr_str_address_calc_cont2	//io, manual execution
	.word address_calc_ignore_arm	//pal: can't happen
	.word ldr_str_address_calc_cont2	//sprites vram, manual execution
	.word address_calc_ignore_arm	//oam: can't happen
	.word ldr_str_address_calc_fix_cartridge	//card: fix
	.word ldr_str_address_calc_fix_cartridge	//card: fix	
	.word ldr_str_address_calc_fix_cartridge	//card: fix
	.word ldr_str_address_calc_fix_cartridge	//card: fix
	.word ldr_str_address_calc_fix_cartridge	//card: fix
	.word ldr_str_address_calc_cont2	//eeprom, manual execution
	.word ldr_str_address_calc_fix_sram	//sram: fix
	.word address_calc_ignore_arm	//nothing: shouldn't happen

ldr_str_address_calc_fix_cartridge:
	ldr r4,= 0x083B0000
	cmp r9, r4
	bge ldr_str_address_calc_cont2
	mov r8, r8, lsr #16
	cmp lr, #0x08000000
	bge ldr_str_address_calc_fix_cartridge_cont
	tst r10, #(1 << 25)
	andne r8, r10, #0xF
	ldrne lr, [r11, r8, lsl #2]
ldr_str_address_calc_fix_cartridge_cont:
	bic lr, lr, #0x06000000
	sub lr, lr, #0x05000000
	sub lr, lr, #0x00FC0000
	str lr, [r11, r8, lsl #2]
	b data_abort_handler_cont_finish

ldr_str_address_calc_fix_sram:
	mov r8, r8, lsr #16
	cmp lr, #0x0E000000
	beq ldr_str_address_calc_fix_sram_cont
	tst r10, #(1 << 25)
	andne r8, r10, #0xF
	ldrne lr, [r11, r8, lsl #2]
ldr_str_address_calc_fix_sram_cont:
	sub lr, lr, #0x0B800000
	sub lr, lr, #0x00008C00
	sub lr, lr, #0x00000060
	str lr, [r11, r8, lsl #2]
	b data_abort_handler_cont_finish

ldr_str_address_calc_cont2:
	tst r10, #(1 << 24)
	tstne r10, #(1 << 21)
	strne r9, [r11, r8, lsr #14]

	and r1, r10, #(0xF << 12)

	mov r4, r10
	mov r7, r11
	tst r10, #(1 << 20)
	beq ldr_str_address_calc_cont2_write
	tst r10, #(1 << 22)
	mov r11, #4
	movne r11, #1
	bl read_address_from_handler
	str r10, [r7, r1, lsr #10]	

	tst r4, #(1 << 24)
	addeq r9, r0
	streq r9, [r7, r8, lsr #14]
	add r5, #4
	b data_abort_handler_cont_finish

ldr_str_address_calc_cont2_write:
	tst r10, #(1 << 22)
	ldr r11, [r11, r1, lsr #10]
	mov r12, #4
	movne r12, #1
	andne r11, r11, #0xFF
	bl write_address_from_handler

	tst r4, #(1 << 24)
	addeq r9, r0
	streq r9, [r7, r8, lsr #14]
	add r5, #4
	b data_abort_handler_cont_finish

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

.global ldm_stm_address_calc
ldm_stm_address_calc:
	and r8, r10, #(0xF << 16)
	ldr lr, [r11, r8, lsr #14]
	mov r1, r10, lsl #16
	//count nr bits
	ldr r12,= 0x10000040
	and r13, r1, #0xFF0000
	ldrb r13, [r12, r13, lsr #16]
	ldrb r12, [r12, r1, lsr #24]
	add r12, r12, r13

	mov r13, r10, lsl #7
	and r13, r13, #0x80000000
	orr r13, r10, lsl #5
	msr cpsr_f, r13
	
	mov r9, lr
	subvc r9, r9, r12, lsl #2
	addge r9, r9, #4

	ldr pc, [pc, r9, lsr #22]

	nop
	.word address_calc_ignore_arm	//bios: ignore
	.word address_calc_ignore_arm	//itcm: ignore
	.word address_calc_ignore_arm	//main: ignore
	.word address_calc_ignore_arm	//wram: can't happen
	.word ldm_stm_address_calc_cont2	//io, manual execution
	.word address_calc_ignore_arm	//pal: can't happen
	.word ldm_stm_address_calc_cont2	//sprites vram, manual execution
	.word address_calc_ignore_arm	//oam: can't happen
	.word ldm_stm_address_calc_fix_cartridge	//card: fix
	.word ldm_stm_address_calc_fix_cartridge	//card: fix	
	.word ldm_stm_address_calc_fix_cartridge	//card: fix
	.word ldm_stm_address_calc_fix_cartridge	//card: fix
	.word ldm_stm_address_calc_fix_cartridge	//card: fix
	.word ldm_stm_address_calc_cont2	//eeprom, manual execution
	.word ldm_stm_address_calc_fix_sram	//sram: fix
	.word address_calc_ignore_arm	//nothing: shouldn't happen
	
ldm_stm_address_calc_fix_cartridge:
	ldr r4,= 0x083B0000
	cmp r9, r4
	bge ldm_stm_address_calc_cont2
	bic lr, lr, #0x06000000
	sub lr, lr, #0x05000000
	sub lr, lr, #0x00FC0000
	str lr, [r11, r8, lsr #14]
	b data_abort_handler_cont_finish

ldm_stm_address_calc_fix_sram:
	sub lr, lr, #0x0B800000
	sub lr, lr, #0x00008C00
	sub lr, lr, #0x00000060
	str lr, [r11, r8, lsr #14]
	b data_abort_handler_cont_finish

ldm_stm_address_calc_cont2:
	tst r10, #(1 << 23)
	rsbeq r12, r12, #0
	tst r10, #(1 << 21)
	ldrne r13, [r11, r8, lsr #14]
	addne r13, r13, r12, lsl #2
	strne r13, [r11, r8, lsr #14]

	tst r10, #(1 << 20)
	mov r4, r11
	mov r1, r1, lsr #16
	beq ldm_stm_address_calc_cont2_write_loop
ldm_stm_address_calc_cont2_load_loop:
	tst r1, #1
	beq ldm_stm_address_calc_cont2_load_loop_cont
	mov r11, #4
	bl read_address_from_handler
	str r10, [r4]
	add r9, r9, #4
ldm_stm_address_calc_cont2_load_loop_cont:
	add r4, r4, #4
	movs r1, r1, lsr #1
	bne ldm_stm_address_calc_cont2_load_loop
	add r5, #4
	b data_abort_handler_cont_finish

ldm_stm_address_calc_cont2_write_loop:
	tst r1, #1
	beq ldm_stm_address_calc_cont2_write_loop_cont
	ldr r11, [r4]
	mov r12, #4
	bl write_address_from_handler
	add r9, r9, #4
ldm_stm_address_calc_cont2_write_loop_cont:
	add r4, r4, #4
	movs r1, r1, lsr #1
	bne ldm_stm_address_calc_cont2_write_loop
	add r5, #4
	b data_abort_handler_cont_finish

address_calc_ignore_arm:
	add r5, #4
	b data_abort_handler_cont_finish