.section .text.start

.global inttbl
inttbl:
.global _start
_start:
	br start, always
	br trap_handler, always
	br nmi_handler, always
	br int0_handler, always
	nop
	nop
	nop
	nop
	nop
	nop
	br int1_handler, always
	nop
	nop
	nop
	nop
	nop
	nop
	br int2_handler, always
	nop
	nop
	nop
	nop
	nop
	nop

.text

.global nmi_handler
nmi_handler:
	cntx s
	rst 0x80, mod0
	retic always

.global trap_handler
trap_handler:
	cntx s
	rst 0x80, mod0
	retic always

#.global trap_handler
#trap_handler:
#	cntx s
#	load 0, ps01
#	rst 0x80, mod0
#	push r0
#	push a0e
#	pusha a0

#	mov 0x600, r0
#	mov [r0], a0
#	add 1, a0
#	mov a0l, [r0]

	//ack timer0
#	mov 0x8202, r0
#	mov 0x400, a0
#	mov a0l, [r0]

#	popa a0
#	pop a0e
#	pop r0
#	retic always

.global trap_handler_tmr
trap_handler_tmr:
	//cntx s
	//load 0, ps01
	//rst 0x80, mod0
	push stt0
	push r0

	//convert this trap to a proper timer irq
	mov 0x8204, r0	//r0 = &REG_ICU_IRQ_REQ
	set 0x400, [r0]	//*r0 |= ICU_IRQ_MASK_TMR0
	rst 0x400, [r0] //*r0 &= ~ICU_IRQ_MASK_TMR0

	pop r0
	pop stt0
	reti always

.global int1_handler
int1_handler:
	cntx s
	load 0, ps01
	rst 0x80, mod0
	push a0e
	pusha a0
	push a1e
	pusha a1
	push b0e
	pusha b0
	push b1e
	pusha b1
	push p0
	push p1
	push sv
	push r0

	//ack irq
	//mov 0x8202, r0	//r0 = &REG_ICU_IRQ_ACK
	//mov 0x8000, a0	//a0 = ICU_IRQ_MASK_DMA
	//mov a0l, [r0]	//*r0 = a0l

	//call onDMACompleted, always
	call gbaa_updateMixer, always

	pop r0
	pop sv
	pop p1
	pop p0
	popa b1
	pop b1e
	popa b0
	pop b0e
	popa a1
	pop a1e
	popa a0
	pop a0e
	retic always

.global int0_handler
int0_handler:
	cntx s
	load 0, ps01
	rst 0x80, mod0
	push a0e
	pusha a0
	push a1e
	pusha a1
	push b0e
	pusha b0
	push b1e
	pusha b1
	push p0
	push p1
	push sv
	push r0

	//ack irq
	mov 0x8202, r0	//r0 = &REG_ICU_IRQ_ACK
	mov 0x4000, a0	//a0 = ICU_IRQ_MASK_APBP
	mov a0l, [r0]	//*r0 = a0l

	call onIpcCommandReceived, always

	pop r0
	pop sv
	pop p1
	pop p0
	popa b1
	pop b1e
	popa b0
	pop b0e
	popa a1
	pop a1e
	popa a0
	pop a0e
	retic always

.global int2_handler
int2_handler:
	cntx s
	load 0, ps01
	rst 0x80, mod0
	push a0e
	pusha a0
	push a1e
	pusha a1
	push b0e
	pusha b0
	push b1e
	pusha b1
	push p0
	push p1
	push sv
	push r0

	//ack irq
	mov 0x8202, r0	//r0 = &REG_ICU_IRQ_ACK
	mov 0x8000, a0	//a0 = ICU_IRQ_MASK_DMA
	mov a0l, [r0]	//*r0 = a0l

	call gbaa_updateDma, always

	pop r0
	pop sv
	pop p1
	pop p0
	popa b1
	pop b1e
	popa b0
	pop b0e
	popa a1
	pop a1e
	popa a0
	pop a0e
	retic always

.global start
start:
	mov 0, mod3
	nop
	nop

	mov 0, sp
	addv 0x4ff, sp
	dint
	mov 0, mod3
	eint
	call initConfigRegs, always
	call initConfigRegsShadow, always
	//...
	dint
	call main, always
exit:
	br exit, always

.global
initConfigRegs:
	rst 0x6ce3, mod0
	set 3, mod0
	load 0u8, page
	rst 0xd000, mod1
	set 0x2000, mod1
	mov 0, mod2
	mov 0, cfgi
	mov 0, cfgj
	mov 0, stepi0
	mov 0, stepj0
	mov 0x110c, ar0
	mov 0x55ab, ar1
	mov 0x0021, arp0
	mov 0x258c, arp1
	mov 0x4ab5, arp2
	mov 0x0442, arp3
	ret always

.global initConfigRegsShadow
initConfigRegsShadow:
	cntx s
	mov 1, st0
	mov 0, st1
	mov 0, st2
	mov 0x110c, ar0
	mov 0x55ab, ar1
	mov 0x0021, arp0
	mov 0x258c, arp1
	mov 0x4ab5, arp2
	mov 0x0442, arp3
	cntx r
	ret always