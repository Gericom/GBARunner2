#pragma once

#define SIO_MULTI_CNT_ERROR			(1 << 6)
#define SIO_MULTI_CNT_START_BUSY    (1 << 7)
#define SIO_MULTI_CNT_IRQ		    (1 << 14)

void sio_multiOnCmdRecv(u8 cmd, u8 arg0, u16 arg1, u16 arg2);
void sio_multiStart();
void sio_multiSendWrite(u16 data);
void sio_multiCntWrite(u16 val);