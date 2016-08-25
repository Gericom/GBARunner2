.section .itcm

//calc address for arm ldrh/ldrsh/ldrsb/strh
.global ldrh_strh_address_calc
ldrh_strh_address_calc:
	tst r0, #(1 << 22)
	and r10, r0, #0xF
	ldreq r10, [r1, r10, lsl #2]
	andne r11, r0, #0xF00
	orrne r10, r11, lsr #4

	tst r0, #(1 << 23)
	rsbeq r10, r10, #0
	tst r0, #(1 << 24)
	addne r9, r10
	and r2, r9, #0x0F000000
	add pc, r2, lsr #22

	nop
	b address_calc_ignore_arm	//bios: ignore
	b address_calc_ignore_arm	//itcm: ignore
	b address_calc_ignore_arm	//main: ignore
	b address_calc_ignore_arm	//wram: can't happen
	b ldrh_strh_address_calc_cont	//io, manual execution
	b address_calc_ignore_arm	//pal: can't happen
	b ldrh_strh_address_calc_cont	//sprites vram, manual execution
	b address_calc_ignore_arm	//oam: can't happen
	b ldrh_strh_address_calc_fix_cartridge	//card: fix
	b ldrh_strh_address_calc_fix_cartridge	//card: fix	
	b ldrh_strh_address_calc_fix_cartridge	//card: fix
	b ldrh_strh_address_calc_fix_cartridge	//card: fix
	b ldrh_strh_address_calc_fix_cartridge	//card: fix
	b ldrh_strh_address_calc_cont	//eeprom, manual execution
	b ldrh_strh_address_calc_fix_sram	//sram: fix
	b address_calc_ignore_arm	//nothing: shouldn't happen

ldrh_strh_address_calc_fix_cartridge:
	mov r8, r8, lsr #16
	ldr r9, [r1, r8, lsl #2]
	mov r10, r9, lsr #24
	cmp r10, #0x8
	bge ldrh_strh_address_calc_fix_cartridge_cont
	tst r0, #(1 << 22)
	andeq r8, r0, #0xF
	ldreq r9, [r1, r8, lsl #2]
ldrh_strh_address_calc_fix_cartridge_cont:
	bic r9, r9, #0x07000000
	sub r9, r9, #0x05000000
	sub r9, r9, #0x00FC0000
	str r9, [r1, r8, lsl #2]
	b data_abort_handler_cont_finish

ldrh_strh_address_calc_fix_sram:
	mov r8, r8, lsr #16
	ldr r9, [r1, r8, lsl #2]
	mov r10, r9, lsr #24
	cmp r10, #0xE
	beq ldrh_strh_address_calc_fix_sram_cont
	tst r0, #(1 << 22)
	andeq r8, r0, #0xF
	ldreq r9, [r1, r8, lsl #2]
ldrh_strh_address_calc_fix_sram_cont:
	sub r9, r9, #0x0B800000
	sub r9, r9, #0x00008600
	str r9, [r1, r8, lsl #2]
	b data_abort_handler_cont_finish

ldrh_strh_address_calc_cont:
	tst r0, #(1 << 24)
	tstne r0, #(1 << 21)
	strne r9, [r1, r8, lsr #14]

	tst r0, #(1 << 20)
	push {r0,r1}
	bne ldrh_strh_address_calc_cont_load
	and r2, r0, #(0xF << 12)
	mov r2, r2, lsr #10
	ldrh r1, [r1, r2]
	mov r0, r9
	mov r2, #2
	bl write_address_from_handler
	b ldrh_strh_address_calc_cont2
ldrh_strh_address_calc_cont_load:
	and r4, r0, #(3 << 5)
	cmp r4, #(2 << 5)
	mov r0, r9
	mov r1, #2
	moveq r1, #1
	bl read_address_from_handler
	cmp r4, #(1 << 5)
	beq ldrh_strh_address_calc_cont2_ld
	cmp r4, #(2 << 5)
	mov r0, r0, lsl #16
	moveq r0, r0, lsl #8
	mov r0, r0, asr #16
	moveq r0, r0, asr #8
ldrh_strh_address_calc_cont2_ld:
	ldr r1, [sp, #4]
	ldr r3, [sp]
	and r2, r3, #(0xF << 12)
	str r0, [r1, r2, lsr #10]
ldrh_strh_address_calc_cont2:
	pop {r0,r1}
	tst r0, #(1 << 24)
	addeq r9, r10
	streq r9, [r1, r8, lsr #14]
	add r5, #4
	b data_abort_handler_cont_finish

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//calc address for ldr/str
.global ldr_str_address_calc
ldr_str_address_calc:
	//r0 = instruction (and with 0x0FFFFFFF), r1 = register table
	tst r0, #(1 << 25)
	moveq r10, r0, lsl #20
	moveq r10, r10, lsr #20
	beq ldr_str_address_calc_cont
	and r10, r0, #0xF
	ldr r10, [r1, r10, lsl #2]
	//construct shift (mov r10, r10, xxx #y)
	and r11, r0, #0xFE0
	orr r11, r11, #0xA000
	orr r11, r11, #0x000A
	//fix c-flag
	mrs r2, spsr
	msr cpsr_f, r2
	strh r11, [pc, #-4]
	//to flush the pipeline
	b ldr_str_address_calc_shift
ldr_str_address_calc_shift:
	.word 0xE1A00000

ldr_str_address_calc_cont:
	tst r0, #(1 << 23)
	rsbeq r10, r10, #0
	tst r0, #(1 << 24)
	addne r9, r10
	and r2, r9, #0x0F000000
	add pc, r2, lsr #22

	nop
	b address_calc_ignore_arm	//bios: ignore
	b address_calc_ignore_arm	//itcm: ignore
	b address_calc_ignore_arm	//main: ignore
	b address_calc_ignore_arm	//wram: can't happen
	b ldr_str_address_calc_cont2	//io, manual execution
	b address_calc_ignore_arm	//pal: can't happen
	b ldr_str_address_calc_cont2	//sprites vram, manual execution
	b address_calc_ignore_arm	//oam: can't happen
	b ldr_str_address_calc_fix_cartridge	//card: fix
	b ldr_str_address_calc_fix_cartridge	//card: fix	
	b ldr_str_address_calc_fix_cartridge	//card: fix
	b ldr_str_address_calc_fix_cartridge	//card: fix
	b ldr_str_address_calc_fix_cartridge	//card: fix
	b ldr_str_address_calc_cont2	//eeprom, manual execution
	b ldr_str_address_calc_fix_sram	//sram: fix
	b address_calc_ignore_arm	//nothing: shouldn't happen

ldr_str_address_calc_fix_cartridge:
	mov r8, r8, lsr #16
	ldr r9, [r1, r8, lsl #2]
	mov r10, r9, lsr #24
	cmp r10, #0x8
	bge ldr_str_address_calc_fix_cartridge_cont
	tst r0, #(1 << 25)
	andne r8, r0, #0xF
	ldrne r9, [r1, r8, lsl #2]
ldr_str_address_calc_fix_cartridge_cont:
	bic r9, r9, #0x07000000
	sub r9, r9, #0x05000000
	sub r9, r9, #0x00FC0000
	str r9, [r1, r8, lsl #2]
	b data_abort_handler_cont_finish

ldr_str_address_calc_fix_sram:
	mov r8, r8, lsr #16
	ldr r9, [r1, r8, lsl #2]
	mov r10, r9, lsr #24
	cmp r10, #0xE
	beq ldr_str_address_calc_fix_sram_cont
	tst r0, #(1 << 25)
	andne r8, r0, #0xF
	ldrne r9, [r1, r8, lsl #2]
ldr_str_address_calc_fix_sram_cont:
	sub r9, r9, #0x0B800000
	sub r9, r9, #0x00008600
	str r9, [r1, r8, lsl #2]
	b data_abort_handler_cont_finish

ldr_str_address_calc_cont2:
	tst r0, #(1 << 24)
	tstne r0, #(1 << 21)
	strne r9, [r1, r8, lsr #14]

	and r11, r0, #(0xF << 12)

	push {r0, r1}
	tst r0, #(1 << 20)
	beq ldr_str_address_calc_cont2_write
	tst r0, #(1 << 22)
	mov r0, r9
	mov r1, #4
	movne r1, #1
	bl read_address_from_handler
	ldr r1, [sp, #4]
	str r0, [r1, r11, lsr #10]
	pop {r0,r1}

	tst r0, #(1 << 24)
	addeq r9, r10
	streq r9, [r1, r8, lsr #14]
	add r5, #4
	b data_abort_handler_cont_finish

ldr_str_address_calc_cont2_write:
	tst r0, #(1 << 22)
	mov r0, r9
	mov r2, #4
	movne r2, #1
	ldr r1, [r1, r11, lsr #10]
	andne r1, r1, #0xFF
	bl write_address_from_handler
	pop {r0,r1}

	tst r0, #(1 << 24)
	addeq r9, r10
	streq r9, [r1, r8, lsr #14]
	add r5, #4
	b data_abort_handler_cont_finish

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

.global ldm_stm_address_calc
ldm_stm_address_calc:
	mov r11, r0, lsl #16
	mov r11, r11, lsr #16
	//count nr bits
	ldr r2,= 0x10000040
	and r3, r11, #0xFF
	ldrb r3, [r2, r3]
	ldrb r2, [r2, r11, lsr #8]
	add r2, r2, r3

	and r3, r0, #0x1800000
	eor r3, r3, #0x1000000
	add pc, r3, lsr #20

	nop

ldm_stm_address_calc_10:
	sub r9, r9, r2, lsl #2
	b ldm_stm_address_calc_cont

ldm_stm_address_calc_11:
	add r9, r9, #4
	b ldm_stm_address_calc_cont

ldm_stm_address_calc_00:
	sub r9, r9, r2, lsl #2
	add r9, r9, #4

ldm_stm_address_calc_01:
ldm_stm_address_calc_cont:
	and r3, r9, #0x0F000000
	add pc, r3, lsr #22

	nop
	b address_calc_ignore_arm	//bios: ignore
	b address_calc_ignore_arm	//itcm: ignore
	b address_calc_ignore_arm	//main: ignore
	b address_calc_ignore_arm	//wram: can't happen
	b ldm_stm_address_calc_cont2	//io, manual execution
	b address_calc_ignore_arm	//pal: can't happen
	b ldm_stm_address_calc_cont2	//sprites vram, manual execution
	b address_calc_ignore_arm	//oam: can't happen
	b ldm_stm_address_calc_fix_cartridge	//card: fix
	b ldm_stm_address_calc_fix_cartridge	//card: fix	
	b ldm_stm_address_calc_fix_cartridge	//card: fix
	b ldm_stm_address_calc_fix_cartridge	//card: fix
	b ldm_stm_address_calc_fix_cartridge	//card: fix
	b ldm_stm_address_calc_cont2	//eeprom, manual execution
	b ldm_stm_address_calc_fix_sram	//sram: fix
	b address_calc_ignore_arm	//nothing: shouldn't happen
	
ldm_stm_address_calc_fix_cartridge:
	ldr r9, [r1, r8, lsr #14]
	bic r9, r9, #0x07000000
	sub r9, r9, #0x05000000
	sub r9, r9, #0x00FC0000
	str r9, [r1, r8, lsr #14]
	b data_abort_handler_cont_finish

ldm_stm_address_calc_fix_sram:
	ldr r9, [r1, r8, lsr #14]
	sub r9, r9, #0x0B800000
	sub r9, r9, #0x00008600
	str r9, [r1, r8, lsr #14]
	b data_abort_handler_cont_finish

ldm_stm_address_calc_cont2:
	tst r0, #(1 << 23)
	rsbeq r2, r2, #0
	tst r0, #(1 << 21)
	ldrne r3, [r1, r8, lsr #14]
	addne r3, r3, r2, lsl #2
	strne r3, [r1, r8, lsr #14]

	tst r0, #(1 << 20)
	mov r4, r1
	beq ldm_stm_address_calc_cont2_write_loop
ldm_stm_address_calc_cont2_load_loop:
	tst r11, #1
	beq ldm_stm_address_calc_cont2_load_loop_cont
	mov r0, r9
	mov r1, #4
	bl read_address_from_handler
	str r0, [r4]
	add r9, r9, #4
ldm_stm_address_calc_cont2_load_loop_cont:
	add r4, r4, #4
	movs r11, r11, lsr #1
	bne ldm_stm_address_calc_cont2_load_loop
	add r5, #4
	b data_abort_handler_cont_finish

ldm_stm_address_calc_cont2_write_loop:
	tst r11, #1
	beq ldm_stm_address_calc_cont2_write_loop_cont
	mov r0, r9
	ldr r1, [r4]
	mov r2, #4
	bl write_address_from_handler
	add r9, r9, #4
ldm_stm_address_calc_cont2_write_loop_cont:
	add r4, r4, #4
	movs r11, r11, lsr #1
	bne ldm_stm_address_calc_cont2_write_loop
	add r5, #4
	b data_abort_handler_cont_finish

address_calc_ignore_arm:
	add r5, #4
	b data_abort_handler_cont_finish