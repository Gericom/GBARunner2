#pragma once

#define BTDMP_REG_BASE                      0x8280
#define BTDMP_CHANNEL_LEN                   0x80
#define BTDMP_CHANNEL_REG_BASE(x)           (BTDMP_REG_BASE + (x) * BTDMP_CHANNEL_LEN)

#define BTDMP_RECEIVE_ENABLE_OFF            0x0000
#define BTDMP_RECEIVE_ENABLE_ON             0x8000
#define REG_BTDMP_RECEIVE_ENABLE(x)         (*(vu16*)(BTDMP_CHANNEL_REG_BASE(x) + 0x1E))

#define REG_BTDMP_TRANSMIT_UNK20(x)         (*(vu16*)(BTDMP_CHANNEL_REG_BASE(x) + 0x20))
#define REG_BTDMP_TRANSMIT_UNK22(x)         (*(vu16*)(BTDMP_CHANNEL_REG_BASE(x) + 0x22))
#define REG_BTDMP_TRANSMIT_UNK24(x)         (*(vu16*)(BTDMP_CHANNEL_REG_BASE(x) + 0x24))
#define REG_BTDMP_TRANSMIT_UNK26(x)         (*(vu16*)(BTDMP_CHANNEL_REG_BASE(x) + 0x26))
#define REG_BTDMP_TRANSMIT_UNK28(x)         (*(vu16*)(BTDMP_CHANNEL_REG_BASE(x) + 0x28))
#define REG_BTDMP_TRANSMIT_UNK2A(x)         (*(vu16*)(BTDMP_CHANNEL_REG_BASE(x) + 0x2A))

#define BTDMP_TRANSMIT_ENABLE_OFF           0x0000
#define BTDMP_TRANSMIT_ENABLE_ON            0x8000
#define REG_BTDMP_TRANSMIT_ENABLE(x)        (*(vu16*)(BTDMP_CHANNEL_REG_BASE(x) + 0x3E))

#define BTDMP_TRANSMIT_FIFO_STAT_FULL       (1 << 3)
#define BTDMP_TRANSMIT_FIFO_STAT_EMPTY      (1 << 4)
#define REG_BTDMP_TRANSMIT_FIFO_STAT(x)     (*(vu16*)(BTDMP_CHANNEL_REG_BASE(x) + 0x42))

#define REG_BTDMP_TRANSMIT_FIFO_DATA(x)     (*(vs16*)(BTDMP_CHANNEL_REG_BASE(x) + 0x46))

#define BTDMP_TRANSMIT_FIFO_CONFIG_FLUSH    (1 << 2)
#define REG_BTDMP_TRANSMIT_FIFO_CONFIG(x)   (*(vs16*)(BTDMP_CHANNEL_REG_BASE(x) + 0x4A))

void btdmp_enableReceive(int channel);
void btdmp_enableTransmit(int channel);
void btdmp_disableReceive(int channel);
void btdmp_disableTransmit(int channel);
void btdmp_flushTransmitFifo(int channel);