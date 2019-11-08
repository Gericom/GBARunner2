#ifndef __FIFO_H__
#define __FIFO_H__

#define FIFO_CNT_EMPTY	(1 << 8)

#define REG_FIFO_CNT	(*((vu32*)0x04000184))
#define REG_SEND_FIFO	(*((vu32*)0x04000188))
#define REG_RECV_FIFO	(*((vu32*)0x04100000))

#endif