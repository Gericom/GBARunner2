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
    u16 id;
    u16 mode;
    wifi_macaddr_t multiMacs[4];
};

extern sio_arm7_work_t gSioWork;

void sio_updateMode(u16 cnt, u16 rcnt);
void sio_writeReg16(u16 addr, u16 val);
void sio_init();