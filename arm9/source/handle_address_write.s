.section .itcm

address_write_table_32bit_dtcm = 0x10000140
address_write_table_16bit_dtcm = 0x10000248
address_write_table_8bit_dtcm = 0x10000454

.global write_address_from_handler
write_address_from_handler:
//r0=address, r1=value, r2=nr bytes
	ldr r3,= 0x06010000
	cmp r0, r3
	bge write_address_from_handler_sprites
	ldr r3,= 0x0400020C
	cmp r0, r3
	bxge lr
	cmp r2, #4
	bne write_address_from_handler_no32
	bic r2, r0, #0x04000000
	mov r2, r2, lsr #1
	ldr r3,= address_write_table_32bit_dtcm
	ldrh r3, [r3, r2]
	orr pc, r3, #0x01000000	//itcm

write_address_from_handler_sprites:
	add r0, #0x3F0000
	cmp r2, #1
	biceq r0, #1
	orreq r1, r1, r1, lsl #8
	moveq r2, #2
	cmp r2, #4
	streq r1, [r0]
	strneh r1, [r0]
	bx lr

write_address_from_handler_no32:
	cmp r2, #2
	bne write_address_from_handler_no16
	bic r2, r0, #0x04000000
	ldr r3,= address_write_table_16bit_dtcm
	ldrh r3, [r3, r2]
	orr pc, r3, #0x01000000	//itcm

write_address_from_handler_no16:
	bic r2, r0, #0x04000000
	mov r2, r2, lsl #1
	ldr r3,= address_write_table_8bit_dtcm
	ldrh r3, [r3, r2]
	orr pc, r3, #0x01000000	//itcm

.global address_write_table_32bit_dtcm_setup
address_write_table_32bit_dtcm_setup:
	ldr r0,= address_write_table_32bit_dtcm
	ldr r1,= address_write_table_32bit
	mov r2, #0x20C
address_write_table_32bit_dtcm_setup_loop:
	ldr r3, [r1], #4
	strh r3, [r0], #2
	subs r2, #4
	bne address_write_table_32bit_dtcm_setup_loop
	bx lr

.global address_write_table_16bit_dtcm_setup
address_write_table_16bit_dtcm_setup:
	ldr r0,= address_write_table_16bit_dtcm
	ldr r1,= address_write_table_16bit
	mov r2, #0x20C
address_write_table_16bit_dtcm_setup_loop:
	ldr r3, [r1], #4
	strh r3, [r0], #2
	subs r2, #2
	bne address_write_table_16bit_dtcm_setup_loop
	bx lr

.global address_write_table_8bit_dtcm_setup
address_write_table_8bit_dtcm_setup:
	ldr r0,= address_write_table_8bit_dtcm
	ldr r1,= address_write_table_8bit
	mov r2, #0x20C
address_write_table_8bit_dtcm_setup_loop:
	ldr r3, [r1], #4
	strh r3, [r0], #2
	subs r2, #1
	bne address_write_table_8bit_dtcm_setup_loop
	bx lr

.global write_address_nomod_8
write_address_nomod_8:
	strb r1, [r0]
	bx lr

.global write_address_nomod_16
write_address_nomod_16:
	strh r1, [r0]
	bx lr

.global write_address_nomod_32
write_address_nomod_32:
	str r1, [r0]
	bx lr

.global write_address_ignore
write_address_ignore:
	bx lr

.global write_address_dispcontrol
write_address_dispcontrol:
	ldr r2,= DISPCNT_copy
	strh r1, [r2]
write_address_dispcontrol_cont:
	ldr r2,= 0xFF80
	and r2, r1, r2
	tst r1, #(1 << 5)//hblank free bit is moved on the ds
	orrne r2, #(1 << 23)
	tst r1, #(1 << 6)//obj mode bit is moved on the ds aswell
	orrne r2, #(1 << 4)
	orr r2, #(1 << 16)//display mode, which did not exist on gba
	and r3, r1, #7
	mov r1, r3
	cmp r3, #1
	moveq r1, #2
	biceq r2, #(1 << 11)
	cmp r3, #3
	movge r1, #5
	orr r2, r1
	str r2, [r0]
	bx lr

.global write_address_dispcontrol_bottom8
write_address_dispcontrol_bottom8:
	ldr r3,= DISPCNT_copy
	ldrh r2, [r3]
	and r2, #0xFF00
	orr r2, r2, r1
	strh r2, [r3]
	ldr r3,= 0x4000001
	ldrb r3, [r3]
	orr r1, r1, r3, lsl #8
	b write_address_dispcontrol_cont

.global write_address_dispcontrol_top8
write_address_dispcontrol_top8:
	ldr r3,= DISPCNT_copy
	ldrh r2, [r3]
	and r2, r2, #0xFF
	orr r2, r2, r1, lsl #8
	strh r2, [r3]
	strb r1, [r0]
	bx lr

.global write_address_dma_src
write_address_dma_src:
	cmp r1, #0x08000000
	blt write_address_dma_src_cont
	cmp r1, #0x0E000000
	bge write_address_dma_src_cont2
	bic r1, r1, #0x07000000
	sub r1, r1, #0x05000000
	sub r1, r1, #0x00FC0000
	str r1, [r0]
	bx lr
write_address_dma_src_cont:
	ldr r3,= 0x06010000
	cmp r1, r3
	blt write_address_dma_src_cont2
	ldr r3,= 0x06018000
	cmp r1, r3
	addlt r1, #0x3F0000
write_address_dma_src_cont2:
	str r1, [r0]
	bx lr

.global write_address_dma_src_top16
write_address_dma_src_top16:
	cmp r1, #0x0800
	blt write_address_dma_src_cont_top16
	cmp r1, #0x0E00
	bge write_address_dma_src_cont_top16_2
	bic r1, r1, #0x0700
	sub r1, r1, #0x0500
	sub r1, r1, #0x00FC
	strh r1, [r0]
	bx lr
write_address_dma_src_cont_top16:
	ldr r3,= 0x0601
	cmp r1, r3
	addeq r1, #0x3F
write_address_dma_src_cont_top16_2:
	strh r1, [r0]
	bx lr

.global write_address_dma_dst
write_address_dma_dst:
	ldr r3,= 0x06010000
	cmp r1, r3
	blt write_address_dma_dst_cont
	ldr r3,= 0x06018000
	cmp r1, r3
	addlt r1, #0x3F0000
write_address_dma_dst_cont:
	str r1, [r0]
	bx lr

.global write_address_dma_dst_top16
write_address_dma_dst_top16:
	ldr r3,= 0x0601
	cmp r1, r3
	addeq r1, #0x3F
	strh r1, [r0]
	bx lr

.global write_address_dma_size
write_address_dma_size:
	ldr r3,= 0x040000DC
	cmp r0, r3
	streqh r1, [r0]
	bxeq lr
	cmp r1, #0
	moveq r1, #0x4000
	strh r1, [r0]
	bx lr

.global write_address_dma_control
write_address_dma_control:
	bic r1, r1, #0x1F
	ldr r3,= 0x040000DE
	cmp r0, r3
	ldreqh r3, [r0, #-2]
	cmpeq r3, #0
	orreq r1, #1
	mov r3, r1, lsr #12
	and r3, #3
	cmp r3, #3
	bge write_address_dma_control_cont
	bic r1, r1, #0x3800
	orr r1, r1, r3, lsl #11
	strh r1, [r0]
	bx lr
write_address_dma_control_cont:
	ldr r3,= 0x40000C6
	cmp r0, r3
	bicne r1, r1, #0x8000
	strneh r1, [r0]
	bxne lr
	ldr r3,= 0x40000C0
	ldr r2,= (0x02400000 - 1536)
	str r2, [r3]
	bic r1, r1, #0x3800
	orr r1, r1, #(3 << 5)
	bic r1, r1, #0x1F
	mov r2, #384
	strh r2, [r0, #-2]
	strh r1, [r0]
	bx lr

.global write_address_dma_size_control
write_address_dma_size_control:
	mov r2, r1, lsl #16
	movs r2, r2, lsr #16
	bne write_address_dma_size_control_cont
	ldr r3,= 0x040000DC
	cmp r0, r3
	movne r2, #0x4000
	moveq r2, #0x10000
write_address_dma_size_control_cont:
	ldr r3,= 0x1FFFFF
	bic r1, r3
	orr r1, r2
	mov r3, r1, lsr #28
	and r3, #3
	cmp r3, #3
	bge write_address_dma_size_control_cont2
	bic r1, r1, #0x38000000
	orr r1, r1, r3, lsl #27
	str r1, [r0]
	bx lr
write_address_dma_size_control_cont2:
	ldr r3,= 0x040000C4
	cmp r0, r3
	bicne r1, #0x80000000
	strne r1, [r0]
	bxne lr
	ldr r3,= 0x40000C0
	ldr r2,= (0x02400000 - 1536)
	str r2, [r3]
	bic r1, r1, #0x38000000
	orr r1, r1, #(3 << (5 + 16))
	ldr r3,= 0x1FFFFF
	bic r1, r3
	orr r1, r1, #384
	str r1, [r0]
	bx lr

.global write_address_timer_counter
write_address_timer_counter:
	mov r1, r1, lsl #17
	mov r1, r1, lsr #16
	strh r1, [r0]
	bx lr

.global write_address_timer
write_address_timer:
	mov r2, r1, lsr #16
	mov r3, r1, lsl #17
	mov r3, r3, lsr #16
	orr r2, r3, r2, lsl #16
	str r2, [r0]
	bx lr

.global write_address_ie
write_address_ie:
	ldr r3,= 0x4000210
	strh r1, [r3]
	bx lr

.global write_address_ie_bottom8
write_address_ie_bottom8:
	ldr r3,= 0x4000210
	strb r1, [r3]
	bx lr

.global write_address_ie_top8
write_address_ie_top8:
	ldr r3,= 0x4000211
	strb r1, [r3]
	bx lr

.global write_address_if
write_address_if:
	ldr r3,= 0x4000214
	strh r1, [r3]
	bx lr

.global write_address_if_bottom8
write_address_if_bottom8:
	ldr r3,= 0x4000214
	strb r1, [r3]
	bx lr

.global write_address_if_top8
write_address_if_top8:
	ldr r3,= 0x4000215
	strb r1, [r3]
	bx lr

.global write_address_ie_if
write_address_ie_if:
	ldr r3,= 0x4000210
	strh r1, [r3]
	mov r1, r1, lsr #16
	strh r1, [r3, #4]
	bx lr

.global write_address_wait_control
write_address_wait_control:
	ldr r3,= WAITCNT_copy
	str r1, [r3]
	bx lr

.global write_address_wait_control_bottom8
write_address_wait_control_bottom8:
	ldr r3,= WAITCNT_copy
	strb r1, [r3]
	bx lr

.global write_address_wait_control_top8
write_address_wait_control_top8:
	ldr r3,= (WAITCNT_copy + 1)
	strb r1, [r3]
	bx lr