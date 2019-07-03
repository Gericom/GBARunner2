.section .itcm

#include "consts.s"

ADDRESS_DMA_BASE = 0x040000B0

.global read_dma_control_bot8
read_dma_control_bot8:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	ldrb r10, [r12, r10]
	bx lr

.global read_dma_control_top8
read_dma_control_top8:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	ldrb r10, [r12, r10]
	and r12, r10, #0xB0
	cmp r12, #0xB0
	orreq r10, #0x80
	bxeq lr
	//get the busy bit from the actual hardware
	ldrb r11, [r9]
	tst r11, #0x80
	biceq r10, #0x80
	orrne r10, #0x80
	bx lr

.global read_dma_control
read_dma_control:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	ldrh r10, [r12, r10]
	and r12, r10, #0xB000
	cmp r12, #0xB000
	orreq r10, #0x8000
	bxeq lr
	//get the busy bit from the actual hardware
	ldrh r11, [r9]
	tst r11, #0x8000
	biceq r10, #0x8000
	orrne r10, #0x8000
	bx lr

.global read_dma_size_control
read_dma_size_control:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	add r10, #2
	ldrh r10, [r12, r10]
	and r12, r10, #0xB000
	cmp r12, #0xB000
	orreq r10, #0x8000
	moveq r10, r10, lsl #16
	bxeq lr
	//get the busy bit from the actual hardware
	ldrh r11, [r9, #2]
	tst r11, #0x8000
	biceq r10, #0x8000
	orrne r10, #0x8000
	mov r10, r10, lsl #16
	bx lr

//todo: transfers should only start on 0->1 transitions!
.macro write_dma_control_variant offs

.global write_dma_control_\offs
write_dma_control_\offs:
	//store the shadow value
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	sub r10, #(\offs - 2)
	add r10, r12
	bic r11, r11, #0x1F

	//resolve busy status of old dma
	ldrh r13, [r10]
	and r13, r13, #0xB000
	cmp r13, #0xB000
	moveq r12, #1
	beq 9f
	//get the busy bit from the actual hardware
	ldrh r13, [r9, #(2 - \offs)]
	tst r13, #0x8000
	moveq r12, #0
	orrne r12, #1
9:
	//this is not completely correct as a second time writing while busy might give wrong results
	strh r11, [r10]
	cmp r12, #1
	andeq r12, r11, #0x8000
	cmpeq r12, #0x8000
	bxeq lr
#ifdef ENABLE_WRAM_ICACHE
	//invalidate icache
	mov r12, #0
	mcr p15, 0, r12, c7, c5, 0
#endif
	//fix mode
	mov r12, r11, lsr #12
	and r12, #3
	bic r11, r11, #0x3800
	orr r11, r11, r12, lsl #11
	tst r11, #0x8000
	beq 6f
	//fix and check dst
	ldr r12, [r10, #-6]
	bic r12, #0xF0000000
	//dst invalid
	cmp r12, #0x02000000
	biclt r11, r11, #0x8000
	blt 6f
	//dst vram
	ldr r13,= DISPCNT_copy
	ldrh r13, [r13]
	and r13, #7
	cmp r13, #3
	ldrlt r13,= 0x06010000
	ldrge r13,= 0x06014000
	cmp r12, r13
	blt 1f
	ldr r13,= 0x06018000
	cmp r12, r13
	addlt r12, #0x3F0000
1:
	str r12, [r9, #(-4 - \offs)] //store in actual dst register
	//fix count
	ldrh r12, [r10, #-2]
	cmp r12, #0
	bne 2f
	ldr r12,= (0x040000DC + \offs)
	cmp r9, r12
	moveq r12, #0x10000
	movne r12, #0x4000
2:
	strh r12, [r9, #(0 - \offs)] //store in actual count register
	orr r11, r12, lsr #16

	//is the src in rom
	ldr r13, [r10, #-0xA]
	bic r13, #0xF0000000
	cmp r13, #0x08000000
	bge dma_rom_src_\offs

	//fix src for vram
	ldr r12,= DISPCNT_copy
	ldrh r12, [r12]
	and r12, #7
	cmp r12, #3
	ldrlt r12,= 0x06010000
	ldrge r12,= 0x06014000
	cmp r13, r12
	blt 3f
	ldr r12,= 0x06018000
	cmp r13, r12
	addlt r13, #0x3F0000
3:
	str r13, [r9, #(-8 - \offs)] //store in actual src register

4:
	mov r12, r11, lsr #11
	and r12, #3
	cmp r12, #3
	beq dma_special_mode_\offs
6:
	//carry out the transfer
	strh r11, [r9, #(2 - \offs)]
	bx lr

dma_special_mode_\offs:
	bic r11, r11, #0x8000
	strh r11, [r9, #(2 - \offs)]

	ldr r13,= (0x40000C4 + \offs)
	cmp r9, r13
	bxne lr

	ldr r13, [r9, #(-8 - \offs)] //src

	ldr r11,= 0x04000188
7:
	ldr r10, [r11, #-4]
	tst r10, #1
	beq 7b

	ldr r10,= 0x04000188
	ldr r12,= 0xAA5500F8
	str r12, [r10]
	str r13, [r10]

	bx lr

dma_rom_src_\offs:
	cmp r13, #0x0E000000
	bge 3b
	bic r13, #0x06000000
	
	tst r11, #(3 << 11)
	bne 5f

	tst r11, #(1 << 10)
	moveq r12, r12, lsl #1
	movne r12, r12, lsl #2
	add r12, r13
	cmp r12, #ROM_ADDRESS_MAX
	bgt dma_rom_from_sd_\offs
5:
	//fix the address and perform the dma normally
	sub r13, #0x05000000
	sub r13, #0x00FC0000
	str r13, [r9, #(-8 - \offs)] //store in actual src register
	b 4b

dma_rom_from_sd_\offs:
	bic r11, r11, #0x8000
	strh r11, [r9, #2 - \offs]
	sub r12, r13, #0x08000000
	ldr sp,= address_dtcm + (16 * 1024)
	push {r0-r3,lr}
	mov r0, r12
	ldrh r10, [r9, #(0 - \offs)] //count
	tst r11, #1
	orrne r10, #(1 << 16)
	tst r11, #(1 << 10)
	moveq r1, r10, lsl #1
	movne r1, r10, lsl #2
	ldr r2, [r9, #(-4 - \offs)] //dst
	bl read_gba_rom_asm
	pop {r0-r3,lr}
	bx lr

.endm

write_dma_control_variant 0
write_dma_control_variant 2
write_dma_control_variant 3

.global write_dma_size_control
write_dma_size_control:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	strh r11, [r12, r10]
	mov r11, r11, lsr #16
	b write_dma_control_0

.global write_dma_control_bot8
write_dma_control_bot8:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	ldrh r13, [r12, r10]
	bic r13, #0xFF
	orr r11, r13, r11
	strh r11, [r12, r10]
	b write_dma_control_2

.global write_dma_control_top8
write_dma_control_top8:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	sub r10, #1
	ldrh r13, [r12, r10]
	bic r13, #0xFF00
	orr r11, r13, r11, lsl #8
	strh r11, [r12, r10]
	b write_dma_control_3

//the way these things handle invalid src/dst is not very nice
//for 16 and 8 bit accesses (are these even allowed??)

.global write_dma_shadow_src_internal_32
write_dma_shadow_src_internal_32:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	bic r11, #0xF0000000
	//cmp r11, #0x02000000
	//	movlo r11, #0
	//cmp r11, #0x08000000
	//	movhs r11, #0
	str r11, [r12, r10]
	bx lr

.global write_dma_shadow_src_internal_hi16
write_dma_shadow_src_internal_hi16:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	bic r11, #0xF000
	//cmp r11, #0x0200
	//	movlo r11, #0
	//cmp r11, #0x0800
	//	movhs r11, #0
	strh r11, [r12, r10]
	bx lr

.global write_dma_shadow_src_internal_hi8
write_dma_shadow_src_internal_hi8:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	bic r11, #0xF0
	//cmp r11, #0x02
	//	movlo r11, #0
	//cmp r11, #0x08
	//	movhs r11, #0
	strb r11, [r12, r10]
	bx lr

.global write_dma_shadow_src_all_32
write_dma_shadow_src_all_32:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	bic r11, #0xF0000000
	//cmp r11, #0x02000000
	//	movlo r11, #0
	str r11, [r12, r10]
	bx lr

.global write_dma_shadow_src_all_hi16
write_dma_shadow_src_all_hi16:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	bic r11, #0xF000
	//cmp r11, #0x0200
	//	movlo r11, #0
	strh r11, [r12, r10]
	bx lr

.global write_dma_shadow_src_all_hi8
write_dma_shadow_src_all_hi8:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	bic r11, #0xF0
	//cmp r11, #0x02
	//	movlo r11, #0
	strb r11, [r12, r10]
	bx lr

.global write_dma_shadow_dst_internal_32
write_dma_shadow_dst_internal_32:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	bic r11, #0xF0000000
	//cmp r11, #0x02000000
	//	bxlo lr
	//cmp r11, #0x08000000
	//	bxhs lr
	str r11, [r12, r10]
	bx lr

.global write_dma_shadow_dst_internal_hi16
write_dma_shadow_dst_internal_hi16:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	bic r11, #0xF000
	//cmp r11, #0x0200
	//	bxlo lr
	//cmp r11, #0x0800
	//	bxhs lr
	strh r11, [r12, r10]
	bx lr

.global write_dma_shadow_dst_internal_hi8
write_dma_shadow_dst_internal_hi8:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	bic r11, #0xF0
	//cmp r11, #0x02
	//	bxlo lr
	//cmp r11, #0x08
	//	bxhs lr
	strb r11, [r12, r10]
	bx lr

.global write_dma_shadow_dst_all_32
write_dma_shadow_dst_all_32:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	bic r11, #0xF0000000
	//cmp r11, #0x02000000
	//	bxlo lr
	str r11, [r12, r10]
	bx lr

.global write_dma_shadow_dst_all_hi16
write_dma_shadow_dst_all_hi16:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	bic r11, #0xF000
	//cmp r11, #0x0200
	//	bxlo lr
	strh r11, [r12, r10]
	bx lr

.global write_dma_shadow_dst_all_hi8
write_dma_shadow_dst_all_hi8:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	bic r11, #0xF0
	//cmp r11, #0x02
	//	bxlo lr
	strb r11, [r12, r10]
	bx lr

.global write_dma_shadow_32
write_dma_shadow_32:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	str r11, [r12, r10]
	bx lr

.global write_dma_shadow_16
write_dma_shadow_16:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	strh r11, [r12, r10]
	bx lr

.global write_dma_shadow_8
write_dma_shadow_8:
	ldr r12,= dma_shadow_regs_dtcm
	ldr r10,= ADDRESS_DMA_BASE
	sub r10, r9, r10
	strb r11, [r12, r10]
	bx lr