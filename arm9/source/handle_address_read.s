.section .itcm

#include "consts.s"

//bios_op = 0xE129F000 //[00DCh+8] after startup and softreset //before this address 0x27C is read
//bios_op = 0xE25EF004 //[0134h+8] during irq execution
//bios_op = 0xE55EC002 //[013Ch+8] after irq execution
bios_op = 0xE3A02004 //[0188h+8] after swi execution; reads between 0x1C8 and 0x274

.global read_address_from_handler_32bit
read_address_from_handler_32bit:
	cmp r9, #0x0F000000
	ldrlo pc, [pc, r9, lsr #22]
	b read_address_undefined_memory_32

	.word read_address_from_handler_bios_32
	.word read_address_undefined_memory_32
	.word read_address_ignore
	.word read_address_ignore
	.word read_address_from_handler_io_32
	.word read_address_ignore
	.word read_address_from_handler_vram_32
	.word read_address_ignore
	.word read_address_from_handler_rom_32
	.word read_address_from_handler_rom_32
	.word read_address_from_handler_rom_32
	.word read_address_from_handler_rom_32
	.word read_address_from_handler_rom_32
	.word read_address_from_handler_eeprom_32
	.word read_address_from_handler_sram_32

read_address_from_handler_bios_32:
	cmp r9, #0x4000
		bge read_address_undefined_memory_32
	//bios area, check if this was caused by an opcode in the bios area
	//switch back to abt mode, since we have some info there
	msr cpsr_c, #0xD7
	mrs sp, spsr
	tst sp, #0x20
	//back to fiq mode
	msr cpsr_c, #0xD1
	//for arm the instruction address + 8 is in r5, and it's exactly the address we want to read
	moveq r10, r5
	//thumb
	ldrne r11,= reg_table
	ldrne r10, [r11, #(8 << 2)] //value of lr = instruction address + 8

	sub r10, #8
	cmp r10, #0x4000
	ldrlt r10, [r9] //if the opcode is in the bios, read the data
	ldrge r10,= bios_op
	andge r11, r9, #3
	movge r11, r11, lsl #3
	movge r10, r10, ror r11
	bx lr

read_address_from_handler_io_32:
	sub r12, r9, #0x04000000
	cmp r12, #0x20C
		movlt r12, r12, lsr #1
		ldrlt r13,= address_read_table_32bit_dtcm
		ldrlth r13, [r13, r12]
		orrlt pc, r13, #0x01000000	//itcm
	b read_address_undefined_memory_32

read_address_from_handler_vram_32:
	add r10, r9, #0x3F0000
	ldr r10, [r10]
	bx lr

read_address_from_handler_rom_32:
	//ldr r12,= ROM_ADDRESS_MAX
	cmp r9, #ROM_ADDRESS_MAX//r12
		blt read_address_from_handler_rom_in_mem_32

	bic r10, r9, #0x0E000000

	mov r13, r10, lsr #8

	ldr r12,= sd_is_cluster_cached_table
	ldrsh r13, [r12, r13]
	cmp r13, #0
		blt read_address_from_handler_rom_32_not_cached

	mov r10, r10, lsl #23
	ldr r12,= sd_cluster_cache
	add r12, r13, lsl #9
	ldr r10, [r12, r10, lsr #23]
	bx lr

read_address_from_handler_rom_in_mem_32:
	bic r10, r9, #0x06000000
	sub r10, r10, #0x05000000
	sub r10, r10, #0x00FC0000
	ldr r10, [r10]
	bx lr

read_address_from_handler_rom_32_not_cached:
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r9,lr}
	mov r0, r10
	bl sdread32_uncached
	mov r10, r0
	pop {r0-r9,lr}
	bx lr

read_address_from_handler_eeprom_32:
	mov r10, #1
	bx lr

read_address_from_handler_sram_32:
	ldr r11,= 0x01FF0000
	bic r10, r9, r11
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	ldrb r10, [r10]
	orr r10, r10, lsl #8
	orr r10, r10, lsl #16
	bx lr

.global read_address_from_handler_16bit
read_address_from_handler_16bit:
	cmp r9, #0x0F000000
	ldrlo pc, [pc, r9, lsr #22]
	b read_address_undefined_memory_16

	.word read_address_from_handler_bios_16
	.word read_address_undefined_memory_16
	.word read_address_ignore
	.word read_address_ignore
	.word read_address_from_handler_io_16
	.word read_address_ignore
	.word read_address_from_handler_vram_16
	.word read_address_ignore
	.word read_address_from_handler_rom_16
	.word read_address_from_handler_rom_16
	.word read_address_from_handler_rom_16
	.word read_address_from_handler_rom_16
	.word read_address_from_handler_rom_16
	.word read_address_from_handler_eeprom_16
	.word read_address_from_handler_sram_16

read_address_from_handler_bios_16:
	cmp r9, #0x4000
	bge read_address_undefined_memory_16
	//bios area, check if this was caused by an opcode in the bios area
	//switch back to abt mode, since we have some info there
	msr cpsr_c, #0xD7
	mrs sp, spsr
	tst sp, #0x20
	//back to fiq mode
	msr cpsr_c, #0xD1
	//for arm the instruction address + 8 is in r5, and it's exactly the address we want to read
	moveq r10, r5
	//thumb
	ldrne r11,= reg_table
	ldrne r10, [r11, #(8 << 2)] //value of lr = instruction address + 8

	sub r10, #8
	cmp r10, #0x4000
	ldrlth r10, [r9] //if the opcode is in the bios, read the data
	ldrge r10,= (bios_op & 0xFFFF)
	tst r9, #1
	movne r10, r10, ror #8
	bx lr

read_address_from_handler_io_16:
	sub r12, r9, #0x04000000
	cmp r12, #0x20C
		ldrlt r13,= address_read_table_16bit_dtcm
		ldrlth r13, [r13, r12]
		orrlt pc, r13, #0x01000000	//itcm
	b read_address_undefined_memory_16

read_address_from_handler_vram_16:
	add r10, r9, #0x3F0000
	ldrh r10, [r10]
	tst r9, #1
		movne r10, r10, ror #8
	bx lr

read_address_from_handler_rom_16:
	//ldr r12,= ROM_ADDRESS_MAX
	cmp r9, #ROM_ADDRESS_MAX //r12
		blt read_address_from_handler_rom_in_mem_16

	bic r10, r9, #0x0E000000

	mov r13, r10, lsr #8

	ldr r12,= sd_is_cluster_cached_table
	ldrsh r13, [r12, r13]
	cmp r13, #0
		blt read_address_from_handler_rom_16_not_cached

	mov r10, r10, lsl #23
	mov r10, r10, lsr #23
	ldr r12,= sd_cluster_cache
	add r12, r13, lsl #9
	ldrh r10, [r12, r10]
	tst r9, #1
	movne r10, r10, ror #8
	bx lr

read_address_from_handler_rom_in_mem_16:
	bic r10, r9, #0x06000000
	sub r10, r10, #0x05000000
	sub r10, r10, #0x00FC0000
	ldrh r10, [r10]
	tst r9, #1
	movne r10, r10, ror #8
	bx lr

read_address_from_handler_rom_16_not_cached:
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r9,lr}
	mov r0, r10
	bl sdread16_uncached
	mov r10, r0
	pop {r0-r9,lr}
	tst r9, #1
	movne r10, r10, ror #8
	bx lr

read_address_from_handler_eeprom_16:
	mov r10, #1
	bx lr

read_address_from_handler_sram_16:
	ldr r11,= 0x01FF0000
	bic r10, r9, r11
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	ldrb r10, [r10]
	orr r10, r10, lsl #8
	tst r9, #1
	movne r10, r10, ror #8
	bx lr

.global read_address_from_handler_8bit
read_address_from_handler_8bit:
	cmp r9, #0x0F000000
	ldrlo pc, [pc, r9, lsr #22]
	b read_address_undefined_memory_8

	.word read_address_from_handler_bios_8
	.word read_address_undefined_memory_8
	.word read_address_ignore
	.word read_address_ignore
	.word read_address_from_handler_io_8
	.word read_address_ignore
	.word read_address_from_handler_vram_8
	.word read_address_ignore
	.word read_address_from_handler_rom_8
	.word read_address_from_handler_rom_8
	.word read_address_from_handler_rom_8
	.word read_address_from_handler_rom_8
	.word read_address_from_handler_rom_8
	.word read_address_from_handler_eeprom_8
	.word read_address_from_handler_sram_8

read_address_from_handler_bios_8:
	cmp r9, #0x4000
		bge read_address_undefined_memory_8
	//bios area, check if this was caused by an opcode in the bios area
	//switch back to abt mode, since we have some info there
	msr cpsr_c, #0xD7
	mrs sp, spsr
	tst sp, #0x20
	//back to fiq mode
	msr cpsr_c, #0xD1
	//for arm the instruction address + 8 is in r5, and it's exactly the address we want to read
	moveq r10, r5
	//thumb
	ldrne r11,= reg_table
	ldrne r10, [r11, #(8 << 2)] //value of lr = instruction address + 8

	sub r10, #8
	cmp r10, #0x4000
	ldrltb r10, [r9] //if the opcode is in the bios, read the data
	bxlt lr
	ldr r10,= bios_op
	and r11, r9, #3
	mov r11, r11, lsl #3
	mov r10, r10, ror r11
	and r10, r10, #0xFF
	bx lr

read_address_from_handler_io_8:
	sub r12, r9, #0x04000000
	cmp r12, #0x20C
		movlt r12, r12, lsl #1
		ldrlt r13,= address_read_table_8bit_dtcm
		ldrlth r13, [r13, r12]
		orrlt pc, r13, #0x01000000	//itcm
	b read_address_undefined_memory_8

read_address_from_handler_vram_8:
	add r10, r9, #0x3F0000
	ldrb r10, [r10]
	bx lr

read_address_from_handler_rom_8:
	//ldr r12,= ROM_ADDRESS_MAX
	cmp r9, #ROM_ADDRESS_MAX//r12
		blt read_address_from_handler_rom_in_mem_8

	bic r10, r9, #0x0E000000

	mov r13, r10, lsr #8

	ldr r12,= sd_is_cluster_cached_table
	ldrsh r13, [r12, r13]
	cmp r13, #0
	blt read_address_from_handler_rom_8_not_cached

	mov r10, r10, lsl #23
	ldr r12,= sd_cluster_cache
	add r12, r13, lsl #9
	ldrb r10, [r12, r10, lsr #23]
	bx lr

read_address_from_handler_rom_in_mem_8:
	bic r10, r9, #0x06000000
	sub r10, r10, #0x05000000
	sub r10, r10, #0x00FC0000
	ldrb r10, [r10]
	bx lr

read_address_from_handler_rom_8_not_cached:
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r9,lr}
	mov r0, r10
	bl sdread8_uncached
	mov r10, r0
	pop {r0-r9,lr}
	bx lr

read_address_from_handler_eeprom_8:
	mov r10, #1
	bx lr

read_address_from_handler_sram_8:
	ldr r11,= 0x01FF0000
	bic r10, r9, r11
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	ldrb r10, [r10]
	bx lr

//read_address_from_handler_highio:
//	ldr r12,= 0x02040000
//	sub r12, r10, r12
//	cmp r12, #0x3B0000
//	addge r10, r12, #0x08000000
//	bge read_address_from_handler_rom
//	mov r10, #0
//	bx lr

.global read_address_nomod_8
read_address_nomod_8:
	ldrb r10, [r9]
	bx lr

.global read_address_nomod_16
read_address_nomod_16:
	ldrh r10, [r9]
	bx lr

.global read_address_nomod_32
read_address_nomod_32:
	ldr r10, [r9]
	bx lr

.global read_address_ignore
read_address_ignore:
	mov r10, #0
	bx lr

//What we want to simulate here is the result of the cpu prefetch
//When a location is read that is undefined, no data is put on the bus, so the previous data will be read instead

//== FROM GBATEK ==
//Accessing unused memory at 00004000h-01FFFFFFh, and 10000000h-FFFFFFFFh (and 02000000h-03FFFFFFh when RAM is disabled via Port 4000800h) returns the recently pre-fetched opcode. For ARM code this is simply:
//  WORD = [$+8]
//For THUMB code the result consists of two 16bit fragments and depends on the address area and alignment where the opcode was stored.
//For THUMB code in Main RAM, Palette Memory, VRAM, and Cartridge ROM this is:
//  LSW = [$+4], MSW = [$+4]
//For THUMB code in BIOS or OAM (and in 32K-WRAM on Original-NDS (in GBA mode)):
//  LSW = [$+4], MSW = [$+6]   ;for opcodes at 4-byte aligned locations
//  LSW = [$+2], MSW = [$+4]   ;for opcodes at non-4-byte aligned locations
//For THUMB code in 32K-WRAM on GBA, GBA SP, GBA Micro, NDS-Lite (but not NDS):
//  LSW = [$+4], MSW = OldHI   ;for opcodes at 4-byte aligned locations
//  LSW = OldLO, MSW = [$+4]   ;for opcodes at non-4-byte aligned locations
//Whereas OldLO/OldHI are usually:
//  OldLO=[$+2], OldHI=[$+2]
//Unless the previous opcode's prefetch was overwritten; that can happen if the previous opcode was itself an LDR opcode, ie. if it was itself reading data:
//  OldLO=LSW(data), OldHI=MSW(data)
//  Theoretically, this might also change if a DMA transfer occurs.
//Note: Additionally, as usually, the 32bit data value will be rotated if the data address wasn't 4-byte aligned, and the upper bits of the 32bit value will be masked in case of LDRB/LDRH reads.

//So we need to look if we were in arm or thumb mode and act accordingly

.global read_address_undefined_memory_32
read_address_undefined_memory_32:
	//switch back to abt mode, since we have some info there
	msr cpsr_c, #0xD7
	mrs sp, spsr
	tst sp, #0x20
	//back to fiq mode
	msr cpsr_c, #0xD1
	bne read_address_undefined_memory_32_thumb
read_address_undefined_memory_32_arm:
	//for arm the instruction address + 8 is in r5, and it's exactly the address we want to read
	ldr r10, [r5]
	and r11, r9, #3
	mov r11, r11, lsl #3
	mov r10, r10, ror r11
	bx lr

read_address_undefined_memory_32_thumb:
	ldr r11,= reg_table
	ldr r10, [r11, #(8 << 2)] //value of lr = instruction address + 8
	ldrh r11, [r10, #-4]
	orr r10, r11, r11, lsl #16
	and r11, r9, #3
	mov r11, r11, lsl #3
	mov r10, r10, ror r11
	bx lr

.global read_address_undefined_memory_16
read_address_undefined_memory_16:
	//switch back to abt mode, since we have some info there
	msr cpsr_c, #0xD7
	mrs sp, spsr
	tst sp, #0x20
	//back to fiq mode
	msr cpsr_c, #0xD1
	bne read_address_undefined_memory_16_thumb
read_address_undefined_memory_16_arm:
	//for arm the instruction address + 8 is in r5, and it's exactly the address we want to read
	ldrh r10, [r5]
	tst r9, #1
	movne r10, r10, ror #8
	bx lr

read_address_undefined_memory_16_thumb:
	ldr r11,= reg_table
	ldr r10, [r11, #(8 << 2)] //value of lr = instruction address + 8
	ldrh r10, [r10, #-4]
	tst r9, #1
	movne r10, r10, ror #8
	bx lr

.global read_address_undefined_memory_8
read_address_undefined_memory_8:
	//switch back to abt mode, since we have some info there
	msr cpsr_c, #0xD7
	mrs sp, spsr
	tst sp, #0x20
	//back to fiq mode
	msr cpsr_c, #0xD1
	bne read_address_undefined_memory_8_thumb
read_address_undefined_memory_8_arm:
	//for arm the instruction address + 8 is in r5, and it's exactly the address we want to read
	ldr r10, [r5]
	and r11, r9, #3
	mov r11, r11, lsl #3
	mov r10, r10, ror r11
	and r10, #0xFF
	bx lr

read_address_undefined_memory_8_thumb:
	ldr r11,= reg_table
	ldr r10, [r11, #(8 << 2)] //value of lr = instruction address + 8
	ldrb r10, [r10, #-4]
	bx lr

.global read_address_dispcontrol
read_address_dispcontrol:
	ldr r10,= DISPCNT_copy
	ldr r10, [r10]
	bx lr

.global read_address_dispcontrol_bottom8
read_address_dispcontrol_bottom8:
	ldr r10,= DISPCNT_copy
	ldrb r10, [r10]
	bx lr

.global read_address_dispcontrol_top8
read_address_dispcontrol_top8:
	ldr r10,= (DISPCNT_copy + 1)
	ldrb r10, [r10]
	bx lr

.global read_address_dispstat
read_address_dispstat:
	ldrh r10, [r9]
	bic r10, #0xFF00
	bic r10, #0x0080
	ldr r11,= shadow_dispstat
	ldrh r11, [r11]
	and r11, #0xFF00
	orr r10, r10, r11
	bx lr

.global read_address_vcount
read_address_vcount:
	ldrh r10, [r9]
	cmp r10, #160
	bxlt lr
	cmp r10, #192
	movlt r10, #159
	bxlt lr
	sub r10, #32
	cmp r10, #227
	movgt r10, #227
	bx lr

.global read_address_timer_counter
read_address_timer_counter:
	ldrh r10, [r9]
	mov r10, r10, lsr #1
	//mov r10, r10, lsl #17
	//mov r10, r10, lsr #16
	bx lr

.global read_address_timer
read_address_timer:
	ldr r10, [r9]
	mov r12, r10, lsr #16
	mov r12, r12, lsl #16
	mov r13, r10, lsl #16
	orr r10, r12, r13, lsr #17
	bx lr

.global read_address_ie
read_address_ie:
	ldr r13,= 0x4000210
	ldrh r10, [r13]
	//ldrb r11, [r13, #2]
	//tst r11, #1
	//orrne r10, #1
	bx lr

.global read_address_ie_bottom8
read_address_ie_bottom8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x210]
	//ldrb r11, [r13, #0x212]
	//tst r11, #1
	//orrne r10, #1
	bx lr

.global read_address_ie_top8
read_address_ie_top8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x211]
	bx lr

.global read_address_if
read_address_if:
	ldr r13,= 0x4000214
	ldrh r10, [r13]
	ldrh r11, fake_irq_flags
	orr r10, r10, r11
	bx lr

.global read_address_if_bottom8
read_address_if_bottom8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x214]
	ldrb r11, fake_irq_flags
	orr r10, r10, r11
	//ldrb r11, [r13, #0x216]
	//bic r10, #1
	//tst r11, #1
	//orrne r10, #1
	bx lr

.global read_address_if_top8
read_address_if_top8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x215]
	ldrb r11, (fake_irq_flags + 1)
	orr r10, r10, r11
	bx lr

.global read_address_ie_if
read_address_ie_if:
	ldr r13,= 0x4000210
	ldrh r12, [r13]
	//ldrb r11, [r13, #2]
	//tst r11, #1
	//orrne r12, #1
	//ldrb r11, [r13, #6]
	ldrh r13, [r13, #4]
	ldrh r11, fake_irq_flags
	orr r13, r13, r11
	orr r10, r12, r13, lsl #16
	//bic r10, #(1 << 16)
	//tst r11, #1
	//orrne r10, #(1 << 16)
	bx lr

.global read_address_wait_control
read_address_wait_control:
	ldr r13,= WAITCNT_copy
	ldr r10, [r13]
	bx lr

.global read_address_wait_control_bottom8
read_address_wait_control_bottom8:
	ldr r13,= WAITCNT_copy
	ldrb r10, [r13]
	bx lr

.global read_address_wait_control_top8
read_address_wait_control_top8:
	ldr r13,= (WAITCNT_copy + 1)
	ldrb r10, [r13]
	bx lr

.global fake_irq_flags
fake_irq_flags:
	.word 0