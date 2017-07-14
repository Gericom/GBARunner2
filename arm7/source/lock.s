.global lock_lock
lock_lock:
	mov r1, #1
1:
	swpb r1, r1, [r0]
	cmp r1, #0
	bne 1b
	bx lr

.global lock_unlock
lock_unlock:
	mov r1, #0
	strb r1, [r0]
	bx lr