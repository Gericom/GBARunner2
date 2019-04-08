#pragma once

#define REG_IRQ_IME     (*(vu32*)0x04000208)
#define REG_IRQ_IE      (*(vu32*)0x04000210)
#define REG_IRQ_IF      (*(vu32*)0x04000214)

static inline u32 irq_masterDisable()
{
    u32 old = REG_IRQ_IME;
	REG_IRQ_IME = 0;
	return old;   
}

static inline void irq_masterRestore(u32 old)
{
	REG_IRQ_IME = old;
}