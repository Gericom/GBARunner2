.section .itcm

//we may not use r5 and r6 here
.global thumb7_8_address_calc
thumb7_8_address_calc:
	and r8, r0, #(7 << 3)
	ldr lr, [r1, r8, lsr #1]
	and r11, r0, #(7 << 6)
	ldr r10, [r1, r11, lsr #4]
	add r9, lr, r10
	and r2, r9, #0x0F000000
	add pc, r2, lsr #22

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
	mov r7, lr, lsr #24
	cmp r7, #0x8
	movlt r8, r11, lsr #3
	movlt lr, r10
	bic lr, lr, #0x07000000
	sub lr, lr, #0x05000000
	sub lr, lr, #0x00FC0000
	str lr, [r1, r8, lsr #1]
	b data_abort_handler_thumb_finish

thumb7_8_address_calc_fix_sram:
	mov r7, lr, lsr #24
	cmp r7, #0xE
	movne r8, r11, lsr #3
	movne lr, r10
	sub lr, lr, #0x0B800000
	sub lr, lr, #0x00008600
	str lr, [r1, r8, lsr #1]
	b data_abort_handler_thumb_finish

thumb7_8_address_calc_cont:
	tst r0, #(1 << 9)
	bne thumb8_address_calc
thumb7_address_calc:
	tst r0, #(1 << 11)
	beq thumb7_address_calc_write
	push {r0, r1}
	tst r0, #(1 << 10)
	mov r1, #4
	movne r1, #1
	mov r0, r9
	bl read_address_from_handler
	pop {r2, r3}//r2=r0,r3=r1
	and r1, r2, #7
	str r0, [r3, r1, lsl #2]
	add r5, #2
	b data_abort_handler_thumb_finish

thumb7_address_calc_write:
	tst r0, #(1 << 10)
	mov r2, #4
	movne r2, #1
	and r0, r0, #7
	ldr r1, [r1, r0, lsl #2]
	andne r1, r1, #0xFF
	mov r0, r9
	bl write_address_from_handler
	add r5, #2
	b data_abort_handler_thumb_finish

thumb8_address_calc:
	ands r4, r0, #(3 << 10)
	beq thumb8_address_calc_write
thumb8_address_calc_read:
	push {r0, r1}
	mov r1, #2
	cmp r4, #(1 << 10)
	moveq r1, #1
	mov r0, r9
	bl read_address_from_handler
	cmp r4, #(2 << 10)
	beq thumb8_address_calc_read_cont
	cmp r4, #(1 << 10)
	mov r0, r0, lsl #16
	moveq r0, r0, lsl #8
	mov r0, r0, asr #16
	moveq r0, r0, asr #8
thumb8_address_calc_read_cont:
	pop {r2, r3}//r2=r0,r3=r1
	and r1, r2, #7
	str r0, [r3, r1, lsl #2]
	add r5, #2
	b data_abort_handler_thumb_finish

thumb8_address_calc_write:
	mov r2, #2
	and r0, r0, #7
	mov r0, r0, lsl #2
	ldrh r1, [r1, r0]
	mov r0, r9
	bl write_address_from_handler
	add r5, #2
	b data_abort_handler_thumb_finish

.global thumb9_address_calc
thumb9_address_calc:
	and r8, r0, #(7 << 3)
	ldr lr, [r1, r8, lsr #1]
	and r10, r0, #(31 << 6)
	tst r0, #(1 << 12)
	addeq r9, lr, r10, lsr #4
	addne r9, lr, r10, lsr #6
	and r2, r9, #0x0F000000
	add pc, r2, lsr #22

	nop
	b address_calc_ignore_thumb	//bios: ignore
	b address_calc_ignore_thumb	//itcm: ignore
	b address_calc_ignore_thumb	//main: ignore
	b address_calc_ignore_thumb	//wram: can't happen
	b thumb9_address_calc_cont	//io, manual execution
	b address_calc_ignore_thumb	//pal: can't happen
	b thumb9_address_calc_cont	//sprites vram, manual execution
	b address_calc_ignore_thumb	//oam: can't happen
	b thumb9_10_address_calc_fix_cartridge	//card: fix
	b thumb9_10_address_calc_fix_cartridge	//card: fix	
	b thumb9_10_address_calc_fix_cartridge	//card: fix
	b thumb9_10_address_calc_fix_cartridge	//card: fix
	b thumb9_10_address_calc_fix_cartridge	//card: fix
	b thumb9_address_calc_cont	//eeprom, manual execution
	b thumb9_10_address_calc_fix_sram	//sram: fix
	b address_calc_ignore_thumb	//nothing: shouldn't happen

thumb9_10_address_calc_fix_cartridge:
	bic lr, lr, #0x07000000
	sub lr, lr, #0x05000000
	sub lr, lr, #0x00FC0000
	str lr, [r1, r8, lsr #1]
	b data_abort_handler_thumb_finish

thumb9_10_address_calc_fix_sram:
	sub lr, lr, #0x0B800000
	sub lr, lr, #0x00008600
	str lr, [r1, r8, lsr #1]
	b data_abort_handler_thumb_finish

thumb9_address_calc_cont:
	tst r0, #(1 << 11)
	beq thumb9_address_calc_write
	push {r0, r1}
	tst r0, #(1 << 12)
	mov r1, #4
	movne r1, #1
	mov r0, r9
	bl read_address_from_handler
	pop {r2, r3}//r2=r0,r3=r1
	and r1, r2, #7
	str r0, [r3, r1, lsl #2]
	add r5, #2
	b data_abort_handler_thumb_finish

thumb9_address_calc_write:
	tst r0, #(1 << 12)
	mov r2, #4
	movne r2, #1
	and r0, r0, #7
	ldr r1, [r1, r0, lsl #2]
	andne r1, r1, #0xFF
	mov r0, r9
	bl write_address_from_handler
	add r5, #2
	b data_abort_handler_thumb_finish
	
.global thumb10_address_calc
thumb10_address_calc:
	and r8, r0, #(7 << 3)
	ldr lr, [r1, r8, lsr #1]
	and r10, r0, #(31 << 6)
	add r9, lr, r10, lsr #5
	and r2, r9, #0x0F000000
	add pc, r2, lsr #22

	nop
	b address_calc_ignore_thumb	//bios: ignore
	b address_calc_ignore_thumb	//itcm: ignore
	b address_calc_ignore_thumb	//main: ignore
	b address_calc_ignore_thumb	//wram: can't happen
	b thumb10_address_calc_cont	//io, manual execution
	b address_calc_ignore_thumb	//pal: can't happen
	b thumb10_address_calc_cont	//sprites vram, manual execution
	b address_calc_ignore_thumb	//oam: can't happen
	b thumb9_10_address_calc_fix_cartridge	//card: fix
	b thumb9_10_address_calc_fix_cartridge	//card: fix	
	b thumb9_10_address_calc_fix_cartridge	//card: fix
	b thumb9_10_address_calc_fix_cartridge	//card: fix
	b thumb9_10_address_calc_fix_cartridge	//card: fix
	b thumb10_address_calc_cont	//eeprom, manual execution
	b thumb9_10_address_calc_fix_sram	//sram: fix
	b address_calc_ignore_thumb	//nothing: shouldn't happen

thumb10_address_calc_cont:
	tst r0, #(1 << 11)
	beq thumb10_address_calc_write
thumb10_address_calc_read:
	push {r0, r1}
	mov r1, #2
	mov r0, r9
	bl read_address_from_handler
	pop {r2, r3}//r2=r0,r3=r1
	and r1, r2, #7
	str r0, [r3, r1, lsl #2]
	add r5, #2
	b data_abort_handler_thumb_finish

thumb10_address_calc_write:
	mov r2, #2
	and r0, r0, #7
	mov r0, r0, lsl #2
	ldrh r1, [r1, r0]
	mov r0, r9
	bl write_address_from_handler
	add r5, #2
	b data_abort_handler_thumb_finish

.global thumb15_address_calc
thumb15_address_calc:
	and r8, r0, #(7 << 8)
	ldr r9, [r1, r8, lsr #6]
	and r2, r9, #0x0F000000
	add pc, r2, lsr #22

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
	bic r9, r9, #0x07000000
	sub r9, r9, #0x05000000
	sub r9, r9, #0x00FC0000
	str r9, [r1, r8, lsr #6]
	b data_abort_handler_thumb_finish

thumb15_address_calc_fix_sram:
	sub r9, r9, #0x0B800000
	sub r9, r9, #0x00008600
	str r9, [r1, r8, lsr #6]
	b data_abort_handler_thumb_finish

thumb15_address_calc_cont:
	and r11, r0, #0xFF
	ldr r2,= 0x10000040
	ldrb r3, [r2, r11]
	add lr, r9, r3, lsl #2
	str lr, [r1, r8, lsr #6]

	tst r0, #(1 << 11)
	mov r4, r1
	beq thumb15_address_calc_cont_write_loop
thumb15_address_calc_cont_load_loop:
	tst r11, #1
	beq thumb15_address_calc_cont_load_loop_cont
	mov r0, r9
	mov r1, #4
	bl read_address_from_handler
	str r0, [r4]
	add r9, r9, #4
thumb15_address_calc_cont_load_loop_cont:
	add r4, r4, #4
	movs r11, r11, lsr #1
	bne thumb15_address_calc_cont_load_loop
	add r5, #2
	b data_abort_handler_thumb_finish

thumb15_address_calc_cont_write_loop:
	tst r11, #1
	beq thumb15_address_calc_cont_write_loop_cont
	mov r0, r9
	ldr r1, [r4]
	mov r2, #4
	bl write_address_from_handler
	add r9, r9, #4
thumb15_address_calc_cont_write_loop_cont:
	add r4, r4, #4
	movs r11, r11, lsr #1
	bne thumb15_address_calc_cont_write_loop
	add r5, #2
	b data_abort_handler_thumb_finish

address_calc_ignore_thumb:
	add r5, #2
	b data_abort_handler_thumb_finish