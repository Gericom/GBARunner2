#pragma once

#define IPC_FIFO_LENGTH 64

typedef struct
{
    vu16 cmdLo;
    vu16 cmdHi;
    vu16 argLo;
    vu16 argHi;
} ipc_cmd_t;

typedef struct
{
    vu16 readPtr;
    vu16 writePtr;
    ipc_cmd_t fifo[IPC_FIFO_LENGTH];
} ipc_fifo_t;

void ipc_init(void);