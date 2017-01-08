.section .itcm

.include "consts.s"

bios_op = 0xE3A02004
//bios_op = 0xE55EC002

.global read_address_from_handler_32bit
read_address_from_handler_32bit:
	cmp r9, #0x06000000
	bge read_address_from_handler_sprites_32bit
	subs r12, r9, #0x04000000
	blt read_address_from_handler_below_io_32bit
	cmp r12, #0x20C
	bge read_address_undefined_memory_32
	mov r12, r12, lsr #1
	ldr r13,= address_read_table_32bit_dtcm
	ldrh r13, [r13, r12]
	orr pc, r13, #0x01000000	//itcm

read_address_from_handler_sprites_32bit:
	cmp r9, #0x08000000
	bge read_address_from_handler_rom_32bit
	add r10, r9, #0x3F0000
	ldr r10, [r10]
	bx lr

read_address_from_handler_rom_32bit:
	cmp r9, #0x0D000000
	bge read_address_from_handler_eeprom_32bit
	ldr r12,= 0x083B0000
	cmp r9, r12
	blt read_address_from_handler_rom_in_mem_32bit

	bic r10, r9, #0x0E000000

	//block c and d to arm9
	//ldr r12,= 0x4000242
	//ldr r13,= 0x8080
	//strh r13, [r12]

	ldr r11,= sd_sd_info
	ldr r13, [r11] //rom size
	cmp r10, r13
	movgt r10, #0
	bxgt lr

	ldr r13, [r11, #4]//cluster shift
	mov r13, r10, lsr r13

	ldr r12,= sd_is_cluster_cached_table
	ldrb r13, [r12, r13]
	cmp r13, #0xFF
	beq read_address_from_handler_rom_32bit_not_cached
	ldr r12,= sd_cluster_cache_info
	ldr r11, [r12, r13, lsl #3]
	bic r11, #0xFFFFFF

	ldr r12, [r12, #0x810]//access counter
	and r12, #0xFFFFFF
	orr r11, r12
	ldr r12,= sd_cluster_cache_info
	str r11, [r12, r13, lsl #3]
	
	ldr r11, [r12, #0x810]
	add r11, #1
	str r11, [r12, #0x810]

	ldr r11, [r12, #0x808]//cluster shift
	mov r13, r13, lsl r11
	ldr r11, [r12, #0x80C]//cluster mask
	and r10, r10, r11
	ldr r12,= sd_cluster_cache
	add r12, r13
	ldr r10, [r12, r10]
	bx lr

read_address_from_handler_rom_32bit_not_cached:
	ldr sp,= 0x10000000 + (16 * 1024)
	push {r0-r9,lr}
	mov r0, r10
	ldr r1,= sdread32_uncached
	blx r1
	mov r10, r0
	pop {r0-r9,lr}
	bx lr

read_address_from_handler_eeprom_32bit:
	cmp r9, #0x0E000000
	bge read_address_from_handler_sram_32bit
	mov r10, #1
	bx lr

read_address_from_handler_sram_32bit:
	ldr r11,= 0x01FF8000
	bic r10, r9, r11
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	//sub r10, r10, #0x0B800000
	//sub r10, r10, #0x00008C00
	//sub r10, r10, #0x00000060
	ldr r10, [r10]
	bx lr

read_address_from_handler_rom_in_mem_32bit:
	bic r10, r9, #0x06000000
	sub r10, r10, #0x05000000
	sub r10, r10, #0x00FC0000
	ldr r10, [r10]
	bx lr

read_address_from_handler_below_io_32bit:
	cmp r9, #0x4000
	bge read_address_undefined_memory_32
	//bios area, check if this was caused by an opcode in the bios area
	//switch back to abt mode, since we have some info there
	msr cpsr_c, #0x97
	mrs sp, spsr
	tst sp, #0x20
	//back to fiq mode
	msr cpsr_c, #0x91
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


.global read_address_from_handler_16bit
read_address_from_handler_16bit:
	cmp r9, #0x06000000
	bge read_address_from_handler_sprites_16bit
	subs r12, r9, #0x04000000
	blt read_address_from_handler_below_io_16bit
	cmp r12, #0x20C
	bge read_address_undefined_memory_16
	ldr r13,= address_read_table_16bit_dtcm
	ldrh r13, [r13, r12]
	orr pc, r13, #0x01000000	//itcm

read_address_from_handler_sprites_16bit:
	cmp r9, #0x08000000
	bge read_address_from_handler_rom_16bit
	add r10, r9, #0x3F0000
	ldrh r10, [r10]
	tst r9, #1
	movne r10, r10, ror #8
	bx lr

read_address_from_handler_rom_16bit:
	cmp r9, #0x0D000000
	bge read_address_from_handler_eeprom_16bit
	ldr r12,= 0x083B0000
	cmp r9, r12
	blt read_address_from_handler_rom_in_mem_16bit

	bic r10, r9, #0x0E000000

	//block c and d to arm9
	//ldr r12,= 0x4000242
	//ldr r13,= 0x8080
	//strh r13, [r12]

	ldr r11,= sd_sd_info
	ldr r13, [r11] //rom size
	cmp r10, r13
	movgt r10, #0
	bxgt lr

	ldr r13, [r11, #4]//cluster shift
	mov r13, r10, lsr r13

	ldr r12,= sd_is_cluster_cached_table
	ldrb r13, [r12, r13]
	cmp r13, #0xFF
	beq read_address_from_handler_rom_16bit_not_cached
	ldr r12,= sd_cluster_cache_info
	ldr r11, [r12, r13, lsl #3]
	bic r11, #0xFFFFFF

	ldr r12, [r12, #0x810]//access counter
	and r12, #0xFFFFFF
	orr r11, r12
	ldr r12,= sd_cluster_cache_info
	str r11, [r12, r13, lsl #3]
	
	ldr r11, [r12, #0x810]
	add r11, #1
	str r11, [r12, #0x810]

	ldr r11, [r12, #0x808]//cluster shift
	mov r13, r13, lsl r11
	ldr r11, [r12, #0x80C]//cluster mask
	and r10, r10, r11
	ldr r12,= sd_cluster_cache
	add r12, r13
	ldrh r10, [r12, r10]
	tst r9, #1
	movne r10, r10, ror #8
	bx lr

read_address_from_handler_rom_16bit_not_cached:
	ldr sp,= 0x10000000 + (16 * 1024)
	push {r0-r9,lr}
	mov r0, r10
	ldr r1,= sdread16_uncached
	blx r1
	mov r10, r0
	pop {r0-r9,lr}
	tst r9, #1
	movne r10, r10, ror #8
	bx lr

read_address_from_handler_eeprom_16bit:
	cmp r9, #0x0E000000
	bge read_address_from_handler_sram_16bit
	mov r10, #1
	bx lr

read_address_from_handler_sram_16bit:
	ldr r11,= 0x01FF8000
	bic r10, r9, r11
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	//sub r10, r10, #0x0B800000
	//sub r10, r10, #0x00008C00
	//sub r10, r10, #0x00000060
	ldrh r10, [r10]
	bx lr

read_address_from_handler_rom_in_mem_16bit:
	bic r10, r9, #0x06000000
	sub r10, r10, #0x05000000
	sub r10, r10, #0x00FC0000
	ldrh r10, [r10]
	tst r9, #1
	movne r10, r10, ror #8
	bx lr

read_address_from_handler_below_io_16bit:
	cmp r9, #0x4000
	bge read_address_undefined_memory_16
	//bios area, check if this was caused by an opcode in the bios area
	//switch back to abt mode, since we have some info there
	msr cpsr_c, #0x97
	mrs sp, spsr
	tst sp, #0x20
	//back to fiq mode
	msr cpsr_c, #0x91
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

.global read_address_from_handler_8bit
read_address_from_handler_8bit:
	cmp r9, #0x06000000
	bge read_address_from_handler_sprites_8bit
	subs r12, r9, #0x04000000
	blt read_address_from_handler_below_io_8bit
	cmp r12, #0x20C
	bge read_address_undefined_memory_8
	mov r12, r12, lsl #1
	ldr r13,= address_read_table_8bit_dtcm
	ldrh r13, [r13, r12]
	orr pc, r13, #0x01000000	//itcm

read_address_from_handler_sprites_8bit:
	cmp r9, #0x08000000
	bge read_address_from_handler_rom_8bit
	add r10, r9, #0x3F0000
	ldrb r10, [r10]
	bx lr

read_address_from_handler_rom_8bit:
	cmp r9, #0x0D000000
	bge read_address_from_handler_eeprom_8bit
	ldr r12,= 0x083B0000
	cmp r9, r12
	blt read_address_from_handler_rom_in_mem_8bit

	bic r10, r9, #0x0E000000

	//block c and d to arm9
	//ldr r12,= 0x4000242
	//ldr r13,= 0x8080
	//strh r13, [r12]

	ldr r11,= sd_sd_info
	ldr r13, [r11] //rom size
	cmp r10, r13
	movgt r10, #0
	bxgt lr

	ldr r13, [r11, #4]//cluster shift
	mov r13, r10, lsr r13

	ldr r12,= sd_is_cluster_cached_table
	ldrb r13, [r12, r13]
	cmp r13, #0xFF
	beq read_address_from_handler_rom_8bit_not_cached
	ldr r12,= sd_cluster_cache_info
	ldr r11, [r12, r13, lsl #3]
	bic r11, #0xFFFFFF

	ldr r12, [r12, #0x810]//access counter
	and r12, #0xFFFFFF
	orr r11, r12
	ldr r12,= sd_cluster_cache_info
	str r11, [r12, r13, lsl #3]
	
	ldr r11, [r12, #0x810]
	add r11, #1
	str r11, [r12, #0x810]

	ldr r11, [r12, #0x808]//cluster shift
	mov r13, r13, lsl r11
	ldr r11, [r12, #0x80C]//cluster mask
	and r10, r10, r11
	ldr r12,= sd_cluster_cache
	add r12, r13
	ldrb r10, [r12, r10]
	bx lr

read_address_from_handler_rom_8bit_not_cached:
	ldr sp,= 0x10000000 + (16 * 1024)
	push {r0-r9,lr}
	mov r0, r10
	ldr r1,= sdread8_uncached
	blx r1
	mov r10, r0
	pop {r0-r9,lr}
	bx lr

read_address_from_handler_eeprom_8bit:
	cmp r9, #0x0E000000
	bge read_address_from_handler_sram_8bit
	mov r10, #1
	bx lr

read_address_from_handler_sram_8bit:
	ldr r11,= 0x01FF8000
	bic r10, r9, r11
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	//sub r10, r10, #0x0B800000
	//sub r10, r10, #0x00008C00
	//sub r10, r10, #0x00000060
	ldrb r10, [r10]
	bx lr

read_address_from_handler_rom_in_mem_8bit:
	bic r10, r9, #0x06000000
	sub r10, r10, #0x05000000
	sub r10, r10, #0x00FC0000
	ldrb r10, [r10]
	bx lr

read_address_from_handler_below_io_8bit:
	cmp r9, #0x4000
	bge read_address_undefined_memory_8
	//bios area, check if this was caused by an opcode in the bios area
	//switch back to abt mode, since we have some info there
	msr cpsr_c, #0x97
	mrs sp, spsr
	tst sp, #0x20
	//back to fiq mode
	msr cpsr_c, #0x91
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
	msr cpsr_c, #0x97
	mrs sp, spsr
	tst sp, #0x20
	//back to fiq mode
	msr cpsr_c, #0x91
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
	msr cpsr_c, #0x97
	mrs sp, spsr
	tst sp, #0x20
	//back to fiq mode
	msr cpsr_c, #0x91
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
	msr cpsr_c, #0x97
	mrs sp, spsr
	tst sp, #0x20
	//back to fiq mode
	msr cpsr_c, #0x91
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
	mov r10, r10, lsl #17
	mov r10, r10, lsr #16
	bx lr

.global read_address_timer
read_address_timer:
	ldr r10, [r9]
	mov r12, r10, lsr #16
	mov r12, r12, lsl #16
	mov r13, r10, lsl #17
	orr r10, r12, r13, lsr #16
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
	bx lr

.global read_address_if_bottom8
read_address_if_bottom8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x214]
	//ldrb r11, [r13, #0x216]
	//bic r10, #1
	//tst r11, #1
	//orrne r10, #1
	bx lr

.global read_address_if_top8
read_address_if_top8:
	ldr r13,= 0x4000000
	ldrb r10, [r13, #0x215]
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