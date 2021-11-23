#include "teak.h"
#include "icu.h"

void icu_init(void)
{
    cpu_disableIrqs();
    REG_ICU_IRQ_DISABLE = 0xFFFF;
    REG_ICU_IRQ_ACK = 0xFFFF;
    REG_ICU_IRQ_INT0 = 0;
    REG_ICU_IRQ_INT1 = 0;
    REG_ICU_IRQ_INT2 = 0;
    REG_ICU_IRQ_VINT = 0;
    REG_ICU_IRQ_MODE = 0;
    REG_ICU_IRQ_POLARITY = 0;
    cpu_disableInt0();
    cpu_disableInt1();
    cpu_disableInt2();
    cpu_disableVInt();
    cpu_enableIrqs();
}