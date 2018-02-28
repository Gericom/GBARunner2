.section .itcm

#include "consts.s"

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

.global write_address_timer_counter
write_address_timer_counter:
	mov r12, r11, lsl #17
	mov r12, r12, lsr #16
	strh r12, [r9]
	ldr r12,= 0x04000188
	str r9, [r12]
	str r11, [r12]
	bx lr

.global write_address_timer
write_address_timer:
	mov r12, r11, lsr #16
	mov r13, r11, lsl #17
	mov r13, r13, lsr #16
	orr r12, r13, r12, lsl #16
	str r12, [r9]
	ldr r12,= 0x04000188
	str r9, [r12]
	str r11, [r12]
	bx lr