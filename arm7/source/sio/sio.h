#pragma once

#include "../wifi/wifi_common.h"

#define SIO_MODE_GPIO       0
#define SIO_MODE_NORMAL     1
#define SIO_MODE_MULTI      2
#define SIO_MODE_UART       3
#define SIO_MODE_JOY        4

#define SIO_ID_MASTER       0
#define SIO_ID_SLAVE_0      1
#define SIO_ID_SLAVE_1      2
#define SIO_ID_SLAVE_2      3

struct sio_arm7_work_t
{
    u8 id;
    u8 multiModes[4];
    wifi_macaddr_t multiMacs[4];
};

#define SIO_CMD_MODE_CHANGE 0x00
#define SIO_CMD_DATA    	0x80

extern sio_arm7_work_t gSioWork;

void sio_updateMode(u16 cnt, u16 rcnt);
void sio_writeReg16(u16 addr, u16 val);
void sio_init();
void sio_sendSimplePacket(wifi_macaddr_t* dst, u8 cmd, u8 arg0, u16 arg1, u16 arg2);