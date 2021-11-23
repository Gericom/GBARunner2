#pragma once

#define ICU_IRQ_MASK_TMR1       (1 << 9)
#define ICU_IRQ_MASK_TMR0       (1 << 10)
#define ICU_IRQ_MASK_BTDMP0     (1 << 11)
#define ICU_IRQ_MASK_BTDMP1     (1 << 12)
#define ICU_IRQ_MASK_SIO        (1 << 13)
#define ICU_IRQ_MASK_APBP       (1 << 14)
#define ICU_IRQ_MASK_DMA        (1 << 15)

#define REG_ICU_IRQ_PENDING     (*(vu16*)0x8200)
#define REG_ICU_IRQ_ACK         (*(vu16*)0x8202)
#define REG_ICU_IRQ_REQ         (*(vu16*)0x8204)
#define REG_ICU_IRQ_INT0        (*(vu16*)0x8206)
#define REG_ICU_IRQ_INT1        (*(vu16*)0x8208)
#define REG_ICU_IRQ_INT2        (*(vu16*)0x820A)
#define REG_ICU_IRQ_VINT        (*(vu16*)0x820C)
#define REG_ICU_IRQ_MODE        (*(vu16*)0x820E)
#define REG_ICU_IRQ_POLARITY    (*(vu16*)0x8210)

#define REG_ICU_VINT_ADDR_HI(x) (*(vu16*)(0x8212 + (x) * 4))
#define REG_ICU_VINT_ADDR_LO(x) (*(vu16*)(0x8214 + (x) * 4))

#define REG_ICU_IRQ_DISABLE     (*(vu16*)0x8252)

void icu_init(void);