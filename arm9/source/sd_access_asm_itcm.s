.section .itcm

#include "consts.s"

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

//r0: dst
//r1: src
.global copy_512
copy_512:
	push {r4-r11,lr}
	tst r1, #3
		bne copy_512_unaligned_src
	tst r0, #3
		bne copy_512_unaligned_dst
	//both source and destination are 32 bit aligned
.rept 10
	ldmia r1!, {r2-r12,lr}
	stmia r0!, {r2-r12,lr}
.endr
	ldmia r1!, {r2-r9}
	stmia r0!, {r2-r9}
	pop {r4-r11,pc}

copy_512_unaligned_src:
	tst r0, #3
		bne copy_512_unaligned_src_dst

copy_512_unaligned_dst:

	mov r2, #256
	bl arm9_memcpy16
	pop {r4-r11,pc}

copy_512_unaligned_src_dst:
	//copy first 16 bit, then we're aligned again
	ldrh r2, [r1], #2
	strh r2, [r0], #2
.rept 10
	ldmia r1!, {r2-r12,lr}
	stmia r0!, {r2-r12,lr}
.endr
	ldmia r1!, {r2-r8}
	stmia r0!, {r2-r8}
	ldrh r2, [r1], #2
	strh r2, [r0], #2
	pop {r4-r11,pc}

//r0: address
//r1: size
//r2: dst
.global read_gba_rom_asm
read_gba_rom_asm:
	add r3, r0, r1
	ldr r12,= sd_sd_info
	ldr r12, [r12]
	cmp r3, r12
		bxge lr
	push {r4-r8,lr}
	mov r4, r0, lsl #23
	mov r4, r4, lsr #23 //cluster_offset
	mov r5, r0, lsr #9 //cluster
	rsb r6, r4, #0x200
	cmp r6, r1
		movgt r6, r1
	mov r7, r2
	mov r8, r1
	mov r0, r5
	bl get_cluster_data_asm
	mov r0, r7
	add r1, r1, r4
	mov r2, r6, lsr #1
	bl arm9_memcpy16
	subs r8, r6
		pople {r4-r8,pc}
	add r7, r6
	add r5, #1
1:
	cmp r8, #512
		blt 2f
	mov r0, r5
	add r5, #1
	bl get_cluster_data_asm
	mov r0, r7
	bl copy_512
	add r7, #512
	subs r8, #512
		bgt 1b
2:
	cmp r8, #0
		pople {r4-r8,pc}
	mov r0, r5
	bl get_cluster_data_asm
	mov r0, r7
	mov r2, r8, lsr #1
	pop {r4-r8,lr}
	b arm9_memcpy16
	
//r0: cluster_index
//returns in r1: cluster data pointer
.global get_cluster_data_asm
get_cluster_data_asm:
	push {lr}
	ldr r1,= sd_is_cluster_cached_table
	add r1, r0, lsl #1
	ldrsh r3, [r1]
	cmp r3, #0
		bllt ensure_cluster_cached_asm

#ifdef CACHE_STRATEGY_LRU_LIST
	ldr r2,= sd_cluster_cache_linked_list
	ldr r1, [r2, r3, lsl #2]

	//cache_linked_list[curBlock->next].prev = curBlock->prev;
	add r0, r2, r1, lsr #14
	strh r1, [r0]

	mov r1, r1, ror #16
	add r0, r2, r1, lsr #14
	strh r1, [r0, #2]

	add r0, r2, #(4096 * 4)

	ldrh r1, [r0]
	strh r3, [r0]

	add r0, r2, r1, lsl #2
	strh r3, [r0, #2]

	orr r1, #(CACHE_LINKED_LIST_NIL << 16)
	str r1, [r2, r3, lsl #2]
#endif

	ldr r2,= sd_cluster_cache
	add r1, r2, r3, lsl #9
	pop {pc}

//r0: cluster_index
.global ensure_cluster_cached_asm
ensure_cluster_cached_asm:
	push {r5, lr}
	mov r3, r0

	ldr r12,= (sd_cluster_cache_linked_list + (4096 * 4) + 2)
	ldrh r5, [r12]
	ldr r1,= sd_cluster_cache_info
	add r1, r5, lsl #3
	ldmia r1, {r0, r2}
	ldr r12,= sd_is_cluster_cached_table
	mov r2, r2, lsl #1
	tst r0, #0x80000000
		movne r0, #0xFFFFFFFF
		strneh r0, [r12, r2]		

	add r12, r3, lsl #1
	strh r5, [r12]

	mov r2, #0x80000000
	stmia r1, {r2, r3}
	//read the data
	ldr r12,= (sd_sd_info + 4)
	ldmia r12, {r0, r1}
	ldr lr, [r12, #0x14]
	ldr r12,= sd_data_base
	sub r0, #9
	mov r2, r3, lsr r0
	ldr r2, [r12, r2, lsl #2]
	sub r2, #2
	add r0, lr, r2, lsl r0
	and r2, r3, r1, lsr #9
	add r0, r2
	mov r1, #1
	ldr r2,= sd_cluster_cache
	add r2, r2, r5, lsl #9

#ifdef ARM7_DLDI
	bl read_sd_sectors_safe
#else
	ldr r12,= (_io_dldi + 0x10)
	ldr r12, [r12]
	blx r12
#endif

	mov r3, r5

	pop {r5, pc}
