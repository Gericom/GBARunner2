.section .itcm

//we may not use r5 and r6 here
.global thumb7_8_address_calc
thumb7_8_address_calc:
	//self-modifying-code test, final code won't be using 2 mov instructions, but a single addition instead (also modifying the code for the fix cartridge and sram stuff)
//	and r8, r10, #(7 << 3)
//	mov r9, r8, lsr #3
//	strb r9, thumb7_8_address_calc_op1
//	and r12, r10, #(7 << 6)
//	mov r9, r12, lsr #6
//	strb r9, thumb7_8_address_calc_op2
//	b thumb7_8_address_calc_op1	//flush pipeline
//thumb7_8_address_calc_op1:
//	.word 0xE1A0E000	//mov lr, rb
//thumb7_8_address_calc_op2:
//	.word 0xE1A0D000	//mov r13, ro

	and r8, r10, #(7 << 3)
	ldr lr, [r11, r8, lsr #1]
	and r12, r10, #(7 << 6)
	ldr r13, [r11, r12, lsr #4]
	add r9, lr, r13
	add pc, r9, lsr #22

	nop
	b address_calc_ignore_thumb	//bios: ignore
	b address_calc_ignore_thumb	//itcm: ignore
	b address_calc_ignore_thumb	//main: ignore
	b address_calc_ignore_thumb	//wram: can't happen
	b thumb7_8_address_calc_cont	//io, manual execution
	b address_calc_ignore_thumb	//pal: can't happen
	b thumb7_8_address_calc_cont	//sprites vram, manual execution
	b address_calc_ignore_thumb	//oam: can't happen
	b thumb7_8_address_calc_fix_cartridge	//card: fix
	b thumb7_8_address_calc_fix_cartridge	//card: fix	
	b thumb7_8_address_calc_fix_cartridge	//card: fix
	b thumb7_8_address_calc_fix_cartridge	//card: fix
	b thumb7_8_address_calc_fix_cartridge	//card: fix
	b thumb7_8_address_calc_cont	//eeprom, manual execution
	b thumb7_8_address_calc_fix_sram	//sram: fix
	b address_calc_ignore_thumb	//nothing: shouldn't happen

thumb7_8_address_calc_fix_cartridge:
	ldr r4,= 0x083B0000
	cmp r9, r4
	bge thumb7_8_address_calc_cont
	cmp lr, #0x08000000
	movlt r8, r12, lsr #3
	movlt lr, r13
	bic lr, lr, #0x06000000
	sub lr, lr, #0x05000000
	sub lr, lr, #0x00FC0000
	str lr, [r11, r8, lsr #1]
	msr cpsr_c, #0x97
	b data_abort_handler_thumb_finish

thumb7_8_address_calc_fix_sram:
	cmp lr, #0x0E000000
	movlt r8, r12, lsr #3
	movlt lr, r13
	sub lr, lr, #0x0B800000
	sub lr, lr, #0x00008C00
	sub lr, lr, #0x00000060
	str lr, [r11, r8, lsr #1]
	msr cpsr_c, #0x97
	b data_abort_handler_thumb_finish

thumb7_8_address_calc_cont:
	tst r10, #(1 << 9)
	bne thumb8_address_calc
thumb7_address_calc:
	tst r10, #(1 << 11)
	beq thumb7_address_calc_write
	and r8, r10, #7
	tst r10, #(1 << 10)
	mov r10, r9
	mov r9, r11
	mov r11, #4
	movne r11, #1
	bl read_address_from_handler
	str r10, [r9, r8, lsl #2]
	msr cpsr_c, #0x97
	add lr, #2
	b data_abort_handler_thumb_finish

thumb7_address_calc_write:
	tst r10, #(1 << 10)
	and r10, r10, #7
	ldr r11, [r11, r10, lsl #2]
	mov r12, #4
	movne r12, #1
	andne r11, r11, #0xFF
	mov r10, r9
	bl write_address_from_handler
	msr cpsr_c, #0x97
	add lr, #2
	b data_abort_handler_thumb_finish

thumb8_address_calc:
	ands r8, r10, #(3 << 10)
	beq thumb8_address_calc_write
thumb8_address_calc_read:
	and r13, r10, #7
	mov r10, r9
	add r9, r11, r13, lsl #2
	mov r11, #2
	cmp r8, #(1 << 10)
	moveq r11, #1
	bl read_address_from_handler
	cmp r8, #(2 << 10)
	beq thumb8_address_calc_read_cont
	cmp r8, #(1 << 10)
	mov r10, r10, lsl #16
	moveq r10, r10, lsl #8
	mov r10, r10, asr #16
	moveq r10, r10, asr #8
thumb8_address_calc_read_cont:
	str r10, [r9]//, r8, lsl #2]
	msr cpsr_c, #0x97
	add lr, #2
	b data_abort_handler_thumb_finish

thumb8_address_calc_write:
	mov r12, #2
	and r10, r10, #7
	mov r10, r10, lsl #2
	ldrh r11, [r11, r10]
	mov r10, r9
	bl write_address_from_handler
	msr cpsr_c, #0x97
	add lr, #2
	b data_abort_handler_thumb_finish

.global thumb9_address_calc
thumb9_address_calc:
	and r8, r10, #(7 << 3)
	ldr lr, [r11, r8, lsr #1]
	and r0, r10, #(31 << 6)
	tst r10, #(1 << 12)
	addeq r9, lr, r0, lsr #4
	addne r9, lr, r0, lsr #6
	add pc, r9, lsr #22

	nop
	b address_calc_ignore_thumb	//bios: ignore
	b address_calc_ignore_thumb	//itcm: ignore
	b address_calc_ignore_thumb	//main: ignore
	b address_calc_ignore_thumb	//wram: can't happen
	b thumb9_address_calc_cont	//io, manual execution
	b address_calc_ignore_thumb	//pal: can't happen
	b thumb9_address_calc_cont	//sprites vram, manual execution
	b address_calc_ignore_thumb	//oam: can't happen
	b thumb9_address_calc_fix_cartridge	//card: fix
	b thumb9_address_calc_fix_cartridge	//card: fix	
	b thumb9_address_calc_fix_cartridge	//card: fix
	b thumb9_address_calc_fix_cartridge	//card: fix
	b thumb9_address_calc_fix_cartridge	//card: fix
	b thumb9_address_calc_cont	//eeprom, manual execution
	b thumb9_address_calc_fix_sram	//sram: fix
	b address_calc_ignore_thumb	//nothing: shouldn't happen

thumb9_address_calc_fix_cartridge:
	ldr r4,= 0x083B0000
	cmp r9, r4
	bge thumb9_address_calc_cont
	bic lr, lr, #0x06000000
	sub lr, lr, #0x05000000
	sub lr, lr, #0x00FC0000
	str lr, [r11, r8, lsr #1]
	msr cpsr_c, #0x97
	b data_abort_handler_thumb_finish

thumb9_address_calc_fix_sram:
	sub lr, lr, #0x0B800000
	sub lr, lr, #0x00008C00
	sub lr, lr, #0x00000060
	str lr, [r11, r8, lsr #1]
	msr cpsr_c, #0x97
	b data_abort_handler_thumb_finish

thumb9_address_calc_cont:
	tst r10, #(1 << 11)
	beq thumb9_address_calc_write
	and r4, r10, #7
	mov r7, r11
	tst r10, #(1 << 12)
	mov r11, #4
	movne r11, #1
	mov r10, r9
	bl read_address_from_handler
	str r10, [r7, r4, lsl #2]
	msr cpsr_c, #0x97
	add lr, #2
	b data_abort_handler_thumb_finish

thumb9_address_calc_write:
	tst r10, #(1 << 12)
	and r10, r10, #7
	ldr r11, [r11, r10, lsl #2]
	mov r12, #4
	movne r12, #1
	andne r11, r11, #0xFF
	mov r10, r9
	bl write_address_from_handler
	msr cpsr_c, #0x97
	add lr, #2
	b data_abort_handler_thumb_finish
	
.global thumb10_address_calc
thumb10_address_calc:
	and r8, r10, #(7 << 3)
	ldr lr, [r11, r8, lsr #1]
	and r0, r10, #(31 << 6)
	add r9, lr, r0, lsr #5
	add pc, r9, lsr #22

	nop
	b address_calc_ignore_thumb	//bios: ignore
	b address_calc_ignore_thumb	//itcm: ignore
	b address_calc_ignore_thumb	//main: ignore
	b address_calc_ignore_thumb	//wram: can't happen
	b thumb10_address_calc_cont	//io, manual execution
	b address_calc_ignore_thumb	//pal: can't happen
	b thumb10_address_calc_cont	//sprites vram, manual execution
	b address_calc_ignore_thumb	//oam: can't happen
	b thumb10_address_calc_fix_cartridge	//card: fix
	b thumb10_address_calc_fix_cartridge	//card: fix	
	b thumb10_address_calc_fix_cartridge	//card: fix
	b thumb10_address_calc_fix_cartridge	//card: fix
	b thumb10_address_calc_fix_cartridge	//card: fix
	b thumb10_address_calc_cont	//eeprom, manual execution
	b thumb10_address_calc_fix_sram	//sram: fix
	b address_calc_ignore_thumb	//nothing: shouldn't happen

thumb10_address_calc_fix_cartridge:
	ldr r4,= 0x083B0000
	cmp r9, r4
	bge thumb10_address_calc_cont
	bic lr, lr, #0x06000000
	sub lr, lr, #0x05000000
	sub lr, lr, #0x00FC0000
	str lr, [r11, r8, lsr #1]
	msr cpsr_c, #0x97
	b data_abort_handler_thumb_finish

thumb10_address_calc_fix_sram:
	sub lr, lr, #0x0B800000
	sub lr, lr, #0x00008C00
	sub lr, lr, #0x00000060
	str lr, [r11, r8, lsr #1]
	msr cpsr_c, #0x97
	b data_abort_handler_thumb_finish

thumb10_address_calc_cont:
	tst r10, #(1 << 11)
	beq thumb10_address_calc_write
thumb10_address_calc_read:
	and r4, r10, #7
	mov r7, r11
	mov r11, #2
	mov r10, r9
	bl read_address_from_handler
	str r10, [r7, r4, lsl #2]
	msr cpsr_c, #0x97
	add lr, #2
	b data_abort_handler_thumb_finish

thumb10_address_calc_write:
	mov r12, #2
	and r10, r10, #7
	mov r10, r10, lsl #2
	ldrh r11, [r11, r10]
	mov r10, r9
	bl write_address_from_handler
	msr cpsr_c, #0x97
	add lr, #2
	b data_abort_handler_thumb_finish

.global thumb15_address_calc
thumb15_address_calc:
	and r8, r10, #(7 << 8)
	ldr r9, [r11, r8, lsr #6]
	add pc, r9, lsr #22

	nop
	b address_calc_ignore_thumb	//bios: ignore
	b address_calc_ignore_thumb	//itcm: ignore
	b address_calc_ignore_thumb	//main: ignore
	b address_calc_ignore_thumb	//wram: can't happen
	b thumb15_address_calc_cont	//io, manual execution
	b address_calc_ignore_thumb	//pal: can't happen
	b thumb15_address_calc_cont	//sprites vram, manual execution
	b address_calc_ignore_thumb	//oam: can't happen
	b thumb15_address_calc_fix_cartridge	//card: fix
	b thumb15_address_calc_fix_cartridge	//card: fix	
	b thumb15_address_calc_fix_cartridge	//card: fix
	b thumb15_address_calc_fix_cartridge	//card: fix
	b thumb15_address_calc_fix_cartridge	//card: fix
	b thumb15_address_calc_cont	//eeprom, manual execution
	b thumb15_address_calc_fix_sram	//sram: fix
	b address_calc_ignore_thumb	//nothing: shouldn't happen

thumb15_address_calc_fix_cartridge:
	ldr r4,= 0x083B0000
	cmp r9, r4
	bge thumb15_address_calc_cont
	bic r9, r9, #0x06000000
	sub r9, r9, #0x05000000
	sub r9, r9, #0x00FC0000
	str r9, [r11, r8, lsr #6]
	msr cpsr_c, #0x97
	b data_abort_handler_thumb_finish

thumb15_address_calc_fix_sram:
	sub r9, r9, #0x0B800000
	sub r9, r9, #0x00008C00
	sub r9, r9, #0x00000060
	str r9, [r11, r8, lsr #6]
	msr cpsr_c, #0x97
	b data_abort_handler_thumb_finish

thumb15_address_calc_cont:
	and r1, r10, #0xFF
	ldr r12,= 0x10000040
	ldrb r13, [r12, r1]
	add lr, r9, r13, lsl #2
	str lr, [r11, r8, lsr #6]

	tst r10, #(1 << 11)
	mov r4, r11
	beq thumb15_address_calc_cont_write_loop
thumb15_address_calc_cont_load_loop:
	tst r1, #1
	beq thumb15_address_calc_cont_load_loop_cont
	mov r10, r9
	mov r11, #4
	bl read_address_from_handler
	str r10, [r4]
	add r9, r9, #4
thumb15_address_calc_cont_load_loop_cont:
	add r4, r4, #4
	movs r1, r1, lsr #1
	bne thumb15_address_calc_cont_load_loop
	msr cpsr_c, #0x97
	add lr, #2
	b data_abort_handler_thumb_finish

thumb15_address_calc_cont_write_loop:
	tst r1, #1
	beq thumb15_address_calc_cont_write_loop_cont
	mov r10, r9
	ldr r11, [r4]
	mov r12, #4
	bl write_address_from_handler
	add r9, r9, #4
thumb15_address_calc_cont_write_loop_cont:
	add r4, r4, #4
	movs r1, r1, lsr #1
	bne thumb15_address_calc_cont_write_loop
	msr cpsr_c, #0x97
	add lr, #2
	b data_abort_handler_thumb_finish

address_calc_ignore_thumb:
	msr cpsr_c, #0x97
	add lr, #2
	b data_abort_handler_thumb_finish