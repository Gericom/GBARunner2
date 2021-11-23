#ifdef SELF_MODIFYING_MIX
.section .itcm

#include "consts.s"

.global write_address_from_handler_16bit_selfmodify
write_address_from_handler_16bit_selfmodify:
	mov r11, r0, lsl #16
	mov r11, r11, lsr #16

.global write_address_from_handler_16bit
write_address_from_handler_16bit:
	cmp r9, #0x10000000
	ldrlo pc, [pc, r9, lsr #22]
	bx lr

	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_from_handler_io_16
	.word write_address_ignore
	.word write_address_from_handler_vram_16
	.word write_address_ignore
	.word write_address_from_handler_rom_gpio_16
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore
	.word write_address_ignore //write_address_from_handler_eeprom_16
	.word write_address_from_handler_sram_16
	.word write_address_from_handler_sram_16

write_address_from_handler_io_16:
	ldr r12,= write_table_16bit_dtcm_new
	sub r13, r9, #0x04000000
	cmp r13, #0x20C
	    ldrlth r13, [r12, r13]
	    orrlt pc, r13, #0x01000000	//itcm
    bx lr

write_address_from_handler_vram_16:
	ldr r13,= DISPCNT_copy
	bic r10, r9, #0xFE0000
	ldr r12,= 0x06018000
	ldrh r13, [r13]
	cmp r10, r12
		bicge r10, #0x8000

	and r12, r13, #7
	cmp r12, #3
	ldrlt r13,= 0x06010000
	ldrge r13,= 0x06014000
	cmp r10, r13
		bge 1f

	//add alpha bit when in 15 bit bitmap mode
	//caution: this is potentially dangerous!
	cmp r12, #3
	cmpne r12, #5
		orreq r11, #0x8000
	strh r11, [r10]
	bx lr

1:
	add r10, #0x3F0000
	strh r11, [r10]
	bx lr

write_address_from_handler_rom_gpio_16:
	ldr r13,= 0x080000C4
	subs r13, r9, r13
		bxlt lr
	cmp r13, #0x4
		bxgt lr
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r3,lr}
	mov r0, r9
	mov r1, r11
	ldr r12,= rio_write
	blx r12
	pop {r0-r3,lr}
	bx lr

write_address_from_handler_sram_16:
	ldr r12,= 0x01FF0000
	bic r10, r9, r12
	sub r10, r10, #((0x0E000000 - MAIN_MEMORY_ADDRESS_SAVE_DATA) & 0x0FF00000) //#0x0BC00000
	sub r10, r10, #((0x0E000000 - MAIN_MEMORY_ADDRESS_SAVE_DATA) & 0x000FF000) //#0x00010000
	tst r9, #1
	movne r11, r11, ror #8
	bic r10, r10, #1
	strb r11, [r10]
	ldr r12,= save_save_work_state_uncached
	mov r11, #1
	strb r11, [r12]
	bx lr
#endif