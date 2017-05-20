.section .itcm

/**
* uint16_t *arm9_memcpy16(uint16_t *_dst, uint16_t *_src, size_t _count)
*
* Function that is similar to standard memcpy function, but works with 16-bit data.
* @param _dst - pointer to destination buffer (16-bit aligned)
* @param _src - pointer to source buffer
* @param _count - number of 16-bit values to be copied
*/
.align
.global arm9_memcpy16
.func arm9_memcpy16
arm9_memcpy16:
stmfd sp!, {r0, r4, r5, r6, r7, r8, lr}
cmp r2, #2
blt 6f
/* Alignment is known to be at least 16-bit */
tst r0, #2
ldrneh r4, [r1], #2
strneh r4, [r0], #2
subne r2, r2, #1
/* Now destination address is 32-bit aligned, still need to check whether */
/* source is 32-bit aligned or not */
tst r1, #2
bne 3f
/* Both destination and source are 32-bit aligned */
cmp r2, #8
blt 2f
tst r0, #4
ldrne r4, [r1], #4
strne r4, [r0], #4
subne r2, r2, #2
tst r0, #8
ldmneia r1!, {r4-r5}
stmneia r0!, {r4-r5}
subne r2, r2, #4
/* Destination address is 128-bit aligned, source address is 32-bit aligned */
1: subs r2, r2, #8
ldmgeia r1!, {r4-r7}
stmgeia r0!, {r4-r7}
bge 1b
add r2, r2, #8
/* Copy up to 3 remaining aligned 32-bit values */
2: tst r2, #4
ldmneia r1!, {r4-r5}
stmneia r0!, {r4-r5}
tst r2, #2
ldrne r4, [r1], #4
strne r4, [r0], #4
and r2, r2, #1
b 6f
/* Destination is 32-bit aligned, but source is only 16-bit aligned */
3: cmp r2, #8
blt 5f
tst r0, #4
ldrneh r4, [r1], #2
ldrneh r5, [r1], #2
orrne r4, r4, r5, asl #16
strne r4, [r0], #4
subne r2, r2, #2
tst r0, #8
ldrneh r4, [r1], #2
ldrne r5, [r1], #4
ldrneh r6, [r1], #2
orrne r4, r4, r5, asl #16
movne r5, r5, lsr #16
orrne r5, r5, r6, asl #16
stmneia r0!, {r4-r5}
subne r2, r2, #4
/* Destination is 128-bit aligned, but source is only 16-bit aligned */
4: subs r2, r2, #8
ldrgeh r4, [r1], #2
ldmgeia r1!, {r5-r7}
ldrgeh r3, [r1], #2
orrge r4, r4, r5, asl #16
movge r5, r5, lsr #16
orrge r5, r5, r6, asl #16
movge r6, r6, lsr #16
orrge r6, r6, r7, asl #16
movge r7, r7, lsr #16
orrge r7, r7, r3, asl #16
stmgeia r0!, {r4-r7}
bge 4b
add r2, r2, #8
/* Copy up to 6 remaining 16-bit values (to 32-bit aligned destination) */
5: subs r2, r2, #2
ldrgeh r4, [r1], #2
ldrgeh r5, [r1], #2
orrge r4, r4, r5, asl #16
strge r4, [r0], #4
bge 5b
add r2, r2, #2
/* Copy the last remaining 16-bit value if any */
6: subs r2, r2, #1
ldrgeh r4, [r1], #2
strgeh r4, [r0], #2
	
ldmfd sp!, {r0, r4, r5, r6, r7, r8, pc}
.endfunc

//r0: src
//r1: dst
/*.global copy_512
copy_512:
	push {r4-r11,lr}
	tst r0, #3
		bne copy_512_unaligned_src
	tst r1, #3
		bne copy_512_unaligned_dst
	//both source and destination are 32 bit aligned
.rept 10
	ldmia r0!, {r2-r12,lr}
	stmia r1!, {r2-r12,lr}
.endr
	ldmia r0!, {r2-r9}
	stmia r1!, {r2-r9}
	pop {r4-r11,pc}

copy_512_unaligned_src:
	tst r1, #3
		bne copy_512_unaligned_src_dst

copy_512_unaligned_dst:

	mov r2, r0
	mov r0, r1
	mov r1, r2
	mov r2, #256
	bl arm9_memcpy16
	pop {r4-r11,pc}

copy_512_unaligned_src_dst:
	//copy first 16 bit, then we're aligned again
	ldrh r2, [r0], #2
	strh r2, [r1], #2
.rept 10
	ldmia r0!, {r2-r12,lr}
	stmia r1!, {r2-r12,lr}
.endr
	ldmia r0!, {r2-r8}
	stmia r1!, {r2-r8}
	ldrh r2, [r0], #2
	strh r2, [r1], #2
	pop {r4-r11,pc}*/