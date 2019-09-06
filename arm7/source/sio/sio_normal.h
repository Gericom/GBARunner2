#pragma once

#define SIO_NORMAL_CNT_START_BUSY    (1 << 7)
#define SIO_NORMAL_CNT_USE_32BIT     (1 << 12)
#define SIO_NORMAL_CNT_IRQ		     (1 << 14)

void sio_normalOnCmdRecv(u8 cmd, u8 arg0, u16 arg1, u16 arg2);
void sio_normalStart();
void sio_normalCntWrite(u16 val);