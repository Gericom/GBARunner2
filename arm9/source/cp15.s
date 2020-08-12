.section .itcm

#include "consts.s"
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

.global mpu_getICacheRegions
mpu_getICacheRegions:
	mrc p15, 0, r0, c2, c0, 1
	bx lr

.global mpu_setICacheRegions
mpu_setICacheRegions:
	mcr p15, 0, r0, c2, c0, 1
	bx lr

.global ic_invalidateAll
ic_invalidateAll:
    mov r0, #0
	mcr p15, 0, r0, c7, c5, 0
#ifdef ENABLE_HICODE
    //unmap code
    mcr p15, 0, r0, c6, c3, 0
#endif
    bx lr