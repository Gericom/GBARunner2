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

//r0 = start
//r1 = length
.global dc_invalidate_range
dc_invalidate_range:
    add         r1, r1, r0
    bic         r0, r0, #0x1F
1:
    mcr         p15, 0, r0, c7, c6, 1
    add         r0, r0, #32
    cmp         r0, r1
    blt         1b
    bx          lr

//r0 = start
//r1 = length
.global dc_flush_range
dc_flush_range:
    mov         r12, #0
    add         r1, r1, r0
    bic         r0, r0, #0x1F
1:
    mcr         p15, 0, r12, c7, c10, 4 /* wait write buffer empty */
    mcr         p15, 0, r0, c7, c14, 1 /* flush */
    add         r0, r0, #32
    cmp         r0, r1
    blt         1b
    bx          lr

.global dc_flush_all
dc_flush_all:
    mov         r12, #0
    mov         r1, #0          // r1: Set No counter (0-3)

1:
    mov         r0, #0          // r0: Line counter (0-DCACHE_SIZE/4)

2:
    orr         r2, r1, r0
    mcr         p15, 0, r12, c7, c10, 4 /* wait write buffer empty */
    mcr         p15, 0, r2, c7, c14, 2  /* flush */
    add         r0, r0, #32
    cmp         r0, #0x1000/4
    blt         2b

    add         r1, r1, #1<<30
    cmp         r1, #0
    bne         1b

    bx          lr

.global dc_invalidate_all
dc_invalidate_all:
    mov         r0, #0
    mcr         p15, 0, r0, c7, c6, 0
    bx          lr

.global dc_wait_write_buffer_empty
dc_wait_write_buffer_empty:
    mov         r0, #0
    mcr         p15, 0, r0, c7, c10, 4
    bx          lr