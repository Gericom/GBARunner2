.section .itcm

address_read_table_32bit_dtcm = 0x1000086C
address_read_table_16bit_dtcm = 0x10000974
address_read_table_8bit_dtcm = 0x10000B80

.global read_address_from_handler
read_address_from_handler:
//r0=address, r1=nr bytes
	cmp r2, #0x0D000000
	movge r0, #1
	bxge lr
	ldr r3,= 0x06010000
	cmp r0, r3
	bge read_address_from_handler_sprites
	ldr r3,= 0x0400020C
	cmp r0, r3
	bxge lr
	bic r2, r0, #0x04000000
	cmp r1, #4
	bne read_address_from_handler_no32
	mov r2, r2, lsr #1
	ldr r3,= address_read_table_32bit_dtcm
	ldrh r3, [r3, r2]
	orr pc, r3, #0x01000000	//itcm

read_address_from_handler_sprites:
	add r0, #0x3F0000
	cmp r1, #1
	ldreqb r0, [r0]
	cmp r1, #2
	ldreqh r0, [r0]
	cmp r1, #4
	ldreq r0, [r0]
	bx lr

read_address_from_handler_no32:
	cmp r1, #2
	bne write_address_from_handler_no16
	ldr r3,= address_read_table_16bit_dtcm
	ldrh r3, [r3, r2]
	orr pc, r3, #0x01000000	//itcm

write_address_from_handler_no16:
	mov r2, r2, lsl #1
	ldr r3,= address_read_table_8bit_dtcm
	ldrh r3, [r3, r2]
	orr pc, r3, #0x01000000	//itcm

.global address_read_table_32bit_dtcm_setup
address_read_table_32bit_dtcm_setup:
	ldr r0,= address_read_table_32bit_dtcm
	ldr r1,= address_read_table_32bit
	mov r2, #0x20C
address_read_table_32bit_dtcm_setup_loop:
	ldr r3, [r1], #4
	strh r3, [r0], #2
	subs r2, #4
	bne address_read_table_32bit_dtcm_setup_loop
	bx lr

.global address_read_table_16bit_dtcm_setup
address_read_table_16bit_dtcm_setup:
	ldr r0,= address_read_table_16bit_dtcm
	ldr r1,= address_read_table_16bit
	mov r2, #0x20C
address_read_table_16bit_dtcm_setup_loop:
	ldr r3, [r1], #4
	strh r3, [r0], #2
	subs r2, #2
	bne address_read_table_16bit_dtcm_setup_loop
	bx lr

.global address_read_table_8bit_dtcm_setup
address_read_table_8bit_dtcm_setup:
	ldr r0,= address_read_table_8bit_dtcm
	ldr r1,= address_read_table_8bit
	mov r2, #0x20C
address_read_table_8bit_dtcm_setup_loop:
	ldr r3, [r1], #4
	strh r3, [r0], #2
	subs r2, #1
	bne address_read_table_8bit_dtcm_setup_loop
	bx lr

.global read_address_nomod_8
read_address_nomod_8:
	ldrb r0, [r0]
	bx lr

.global read_address_nomod_16
read_address_nomod_16:
	ldrh r0, [r0]
	bx lr

.global read_address_nomod_32
read_address_nomod_32:
	ldr r0, [r0]
	bx lr

.global read_address_ignore
read_address_ignore:
	mov r0, #0
	bx lr

.global read_address_dispcontrol
read_address_dispcontrol:
	ldr r0,= DISPCNT_copy
	ldr r0, [r0]
	bx lr

.global read_address_dispcontrol_bottom8
read_address_dispcontrol_bottom8:
	ldr r0,= DISPCNT_copy
	ldrb r0, [r0]
	bx lr

.global read_address_dispcontrol_top8
read_address_dispcontrol_top8:
	ldr r0,= (DISPCNT_copy + 1)
	ldrb r0, [r0]
	bx lr

.global read_address_timer_counter
read_address_timer_counter:
	ldrh r0, [r0]
	mov r0, r0, lsl #17
	mov r0, r0, lsr #16
	bx lr

.global read_address_timer
read_address_timer:
	ldr r0, [r0]
	mov r2, r0, lsr #16
	mov r2, r2, lsl #16
	mov r3, r0, lsl #17
	orr r0, r2, r3, lsr #16
	bx lr

.global read_address_ie
read_address_ie:
	ldr r3,= 0x4000210
	ldrh r0, [r3]
	bx lr

.global read_address_ie_bottom8
read_address_ie_bottom8:
	ldr r3,= 0x4000000
	ldrb r0, [r3, #0x210]
	bx lr

.global read_address_ie_top8
read_address_ie_top8:
	ldr r3,= 0x4000000
	ldrb r0, [r3, #0x211]
	bx lr

.global read_address_if
read_address_if:
	ldr r3,= 0x4000214
	ldrh r0, [r3]
	bx lr

.global read_address_if_bottom8
read_address_if_bottom8:
	ldr r3,= 0x4000000
	ldrb r0, [r3, #0x214]
	bx lr

.global read_address_if_top8
read_address_if_top8:
	ldr r3,= 0x4000000
	ldrb r0, [r3, #0x215]
	bx lr

.global read_address_ie_if
read_address_ie_if:
	ldr r3,= 0x4000210
	ldrh r2, [r3]
	ldrh r3, [r3, #4]
	orr r0, r2, r3, lsl #16
	bx lr

.global read_address_wait_control
read_address_wait_control:
	ldr r3,= WAITCNT_copy
	ldr r0, [r3]
	bx lr

.global read_address_wait_control_bottom8
read_address_wait_control_bottom8:
	ldr r3,= WAITCNT_copy
	ldrb r0, [r3]
	bx lr

.global read_address_wait_control_top8
read_address_wait_control_top8:
	ldr r3,= (WAITCNT_copy + 1)
	ldrb r0, [r3]
	bx lr