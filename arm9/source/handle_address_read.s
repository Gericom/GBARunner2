.section .itcm

.include "consts.s"

.macro read_from_rom size
	bic r10, r9, #0x0E000000
	//ensure block d is mapped to the arm7
	ldr r12,= 0x4000243
	mov r13, #0x8A
	strb r13, [r12]
	//send read command
	ldr r12,= 0x04000188
	ldr r13,= 0xAA5500C8
	str r13, [r12]
	str r10, [r12]	//address
	mov r11, #\size
	str r11, [r12]	//size

	//wait for the arm7 sync command
	ldr r13,= 0x55AAC8AC
	ldr r12,= 0x04000184
1:
	ldr r10, [r12]
	tst r10, #(1 << 8)
	bne 1b
	ldr r10,= 0x04100000
	ldr r10, [r10]	//read word from fifo
	cmp r10, r13
	bne 1b

	//block d to arm9 lcdc
	ldr r12,= 0x4000243
	mov r13, #0x80
	strb r13, [r12]
.endm

.global read_address_from_handler_32bit
read_address_from_handler_32bit:
	cmp r9, #0x06000000
	bge read_address_from_handler_sprites_32bit
	subs r12, r9, #0x04000000
	blt read_address_ignore
	cmp r12, #0x20C
	bge read_address_ignore
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

	//ensure block c and d are mapped to the arm7
	ldr r12,= 0x4000242
	ldr r13,= 0x8A82
	strh r13, [r12]

	ldr r12,= 0x4000000

	//send read command
	ldr r13,= 0xAA5500C9
	str r13, [r12, #0x188]
	str r10, [r12, #0x188]	//address

	//wait for the arm7 sync command
1:
	ldr r10, [r12, #0x184]
	tst r10, #(1 << 8)
	bne 1b
	ldr r10,= 0x04100000
	ldr r10, [r10]	//read word from fifo

	//block d to arm9 lcdc for safety
	ldr r12,= 0x4000243
	mov r13, #0x80
	strb r13, [r12]

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

.global read_address_from_handler_16bit
read_address_from_handler_16bit:
	cmp r9, #0x06000000
	bge read_address_from_handler_sprites_16bit
	subs r12, r9, #0x04000000
	blt read_address_ignore
	cmp r12, #0x20C
	bge read_address_ignore
	ldr r13,= address_read_table_16bit_dtcm
	ldrh r13, [r13, r12]
	orr pc, r13, #0x01000000	//itcm

read_address_from_handler_sprites_16bit:
	cmp r9, #0x08000000
	bge read_address_from_handler_rom_16bit
	add r10, r9, #0x3F0000
	ldrh r10, [r10]
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
	bx lr

read_address_from_handler_rom_16bit_not_cached:
	ldr sp,= 0x10000000 + (16 * 1024)
	push {r0-r9,lr}
	mov r0, r10
	ldr r1,= sdread16_uncached
	blx r1
	mov r10, r0
	pop {r0-r9,lr}
	bx lr

	//ensure block c and d are mapped to the arm7
	ldr r12,= 0x4000242
	ldr r13,= 0x8A82
	strh r13, [r12]
	ldr r12,= 0x4000000
	//send read command
	ldr r13,= 0xAA5500CA
	str r13, [r12, #0x188]
	str r10, [r12, #0x188]	//address

	//wait for the arm7 sync command
1:
	ldr r10, [r12, #0x184]
	tst r10, #(1 << 8)
	bne 1b
	ldr r10,= 0x04100000
	ldr r10, [r10]	//read word from fifo

	//block d to arm9 lcdc for safety
	ldr r12,= 0x4000243
	mov r13, #0x80
	strb r13, [r12]

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
	bx lr

.global read_address_from_handler_8bit
read_address_from_handler_8bit:
	cmp r9, #0x06000000
	bge read_address_from_handler_sprites_8bit
	subs r12, r9, #0x04000000
	blt read_address_ignore
	cmp r12, #0x20C
	bge read_address_ignore
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

	//ensure block c and d are mapped to the arm7
	ldr r12,= 0x4000242
	ldr r13,= 0x8A82
	strh r13, [r12]
	ldr r12,= 0x4000000
	//send read command
	ldr r13,= 0xAA5500CB
	str r13, [r12, #0x188]
	str r10, [r12, #0x188]	//address

	//wait for the arm7 sync command
1:
	ldr r10, [r12, #0x184]
	tst r10, #(1 << 8)
	bne 1b
	ldr r10,= 0x04100000
	ldr r10, [r10]	//read word from fifo

	//block d to arm9 lcdc for safety
	ldr r12,= 0x4000243
	mov r13, #0x80
	strb r13, [r12]

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

.global read_address_from_handler
read_address_from_handler:
//r10=address, r11=nr bytes
	cmp r9, #0x06000000
	bge read_address_from_handler_sprites
	subs r12, r9, #0x04000000
	blt read_address_ignore
	cmp r12, #0x20C
	bge read_address_ignore

	add pc, r11, lsl #4

	nop

	nop
	nop
	nop
	nop

read_address_from_handler_1:
	mov r12, r12, lsl #1
	ldr r13,= address_read_table_8bit_dtcm
	ldrh r13, [r13, r12]
	orr pc, r13, #0x01000000	//itcm

read_address_from_handler_2:
	ldr r13,= address_read_table_16bit_dtcm
	ldrh r13, [r13, r12]
	orr pc, r13, #0x01000000	//itcm
	nop

	nop
	nop
	nop
	nop

read_address_from_handler_4:
	mov r12, r12, lsr #1
	ldr r13,= address_read_table_32bit_dtcm
	ldrh r13, [r13, r12]
	orr pc, r13, #0x01000000	//itcm

read_address_from_handler_sprites:
	cmp r9, #0x08000000
	bge read_address_from_handler_rom
	add r10, r9, #0x3F0000
	cmp r11, #2
	ldrltb r10, [r10]
	ldreqh r10, [r10]
	ldrgt r10, [r10]
	bx lr

read_address_from_handler_rom:
	cmp r9, #0x0D000000
	bge read_address_from_handler_eeprom
	ldr r12,= 0x083B0000
	cmp r9, r12
	blt read_address_from_handler_rom_in_mem

	bic r10, r9, #0x0E000000

	//block c and d to arm9
	//ldr r12,= 0x4000242
	//ldr r13,= 0x8080
	//strh r13, [r12]

	ldr r12,= sd_sd_info
	ldr r13, [r12] //rom size
	cmp r10, r13
	movgt r10, #0
	bxgt lr

	ldr r13, [r12, #4]//cluster shift
	mov r13, r10, lsr r13

	ldr r12,= sd_is_cluster_cached_table
	ldrb r13, [r12, r13]
	cmp r13, #0xFF
	beq read_address_from_handler_rom_not_cached
	cmp r11, #2

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
	
	ldrltb r10, [r12,r10]
	ldreqh r10, [r12,r10]
	ldrgt r10, [r12,r10]
	//ldrb r10, [r12, r10]
	bx lr

read_address_from_handler_rom_not_cached:
	ldr sp,= 0x10000000 + (16 * 1024)
	push {r0-r9,lr}
	mov r0, r10
	cmp r11, #2
	ldrlt r1,= sdread8_uncached
	ldreq r1,= sdread16_uncached
	ldrgt r1,= sdread32_uncached
	blx r1
	mov r10, r0
	pop {r0-r9,lr}
	bx lr

	//ensure block c and d are mapped to the arm7
	ldr r12,= 0x4000242
	ldr r13,= 0x8A82
	strh r13, [r12]
	ldr r12,= 0x4000000
	//send read command
	
	cmp r11, #2
	ldrlt r13,= 0xAA5500CB
	ldreq r13,= 0xAA5500CA
	ldrgt r13,= 0xAA5500C9

	str r13, [r12, #0x188]
	str r10, [r12, #0x188]	//address

	//wait for the arm7 sync command
	//ldr r12,= 0x04000184
1:
	ldr r10, [r12, #0x184]
	tst r10, #(1 << 8)
	bne 1b
	ldr r10,= 0x04100000
	ldr r10, [r10]	//read word from fifo

	//block d to arm9 lcdc for safety
	ldr r12,= 0x4000243
	mov r13, #0x80
	strb r13, [r12]

	bx lr

read_address_from_handler_eeprom:
	cmp r9, #0x0E000000
	bge read_address_from_handler_sram
	mov r10, #1
	bx lr

read_address_from_handler_sram:
	ldr r12,= 0x01FF8000
	bic r10, r9, r12
	sub r10, r10, #0x0BC00000
	sub r10, r10, #0x00010000
	//sub r10, r10, #0x0B800000
	//sub r10, r10, #0x00008C00
	//sub r10, r10, #0x00000060
	cmp r11, #2
	ldrltb r10, [r10]
	ldreqh r10, [r10]
	ldrgt r10, [r10]
	bx lr

read_address_from_handler_rom_in_mem:
	bic r10, r9, #0x06000000
	sub r10, r10, #0x05000000
	sub r10, r10, #0x00FC0000
	cmp r11, #2
	ldrltb r10, [r10]
	ldreqh r10, [r10]
	ldrgt r10, [r10]
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