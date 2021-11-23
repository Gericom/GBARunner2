#pragma once

#define REG_APBP_REP0       (*(vu16*)0x80C0)
#define REG_APBP_CMD0       (*(vu16*)0x80C2)

#define REG_APBP_REP1       (*(vu16*)0x80C4)
#define REG_APBP_CMD1       (*(vu16*)0x80C6)

#define REG_APBP_REP2       (*(vu16*)0x80C8)
#define REG_APBP_CMD2       (*(vu16*)0x80CA)

#define REG_APBP_PSEM       (*(vu16*)0x80CC)
#define REG_APBP_PMASK      (*(vu16*)0x80CE)
#define REG_APBP_PCLEAR     (*(vu16*)0x80D0)
#define REG_APBP_SEM        (*(vu16*)0x80D2)

#define APBP_CONTROL_ARM_BIG_ENDIAN     (1 << 2)
#define APBP_CONTROL_IRQ_CMD0_DISABLE   (1 << 8)
#define APBP_CONTROL_IRQ_CMD1_DISABLE   (1 << 12)
#define APBP_CONTROL_IRQ_CMD2_DISABLE   (1 << 13)

#define REG_APBP_CONTROL    (*(vu16*)0x80D4)

#define APBP_STAT_REP0_UNREAD           (1 << 5)
#define APBP_STAT_REP1_UNREAD           (1 << 6)
#define APBP_STAT_REP2_UNREAD           (1 << 7)

#define APBP_STAT_CMD0_NEW              (1 << 8)
#define APBP_STAT_CMD1_NEW              (1 << 12)
#define APBP_STAT_CMD2_NEW              (1 << 13)

#define APBP_STAT_SEM_FLAG              (1 << 9)

#define REG_APBP_STAT       (*(vu16*)0x80D6)

#define REG_APBP_ARM_STAT   (*(vu16*)0x80D8)

static inline void apbp_setSemaphore(u16 mask)
{
    REG_APBP_PSEM = mask;
}

static inline void apbp_setSemaphoreMask(u16 mask)
{
    REG_APBP_PMASK = mask;
}

static inline void apbp_clearSemaphore(u16 mask)
{
    REG_APBP_PCLEAR = mask;
}

static inline u16 apbp_getSemaphore(void)
{
    return REG_APBP_SEM;
}

void apbp_sendData(u16 id, u16 data);
u16 apbp_receiveData(u16 id);