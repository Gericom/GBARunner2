#include "teak.h"
#include "btdmp.h"

void btdmp_enableReceive(int channel)
{
    REG_BTDMP_RECEIVE_ENABLE(channel) = BTDMP_RECEIVE_ENABLE_ON;
}

void btdmp_enableTransmit(int channel)
{
    REG_BTDMP_TRANSMIT_ENABLE(channel) = BTDMP_TRANSMIT_ENABLE_ON;
}

void btdmp_disableReceive(int channel)
{
    REG_BTDMP_RECEIVE_ENABLE(channel) = BTDMP_RECEIVE_ENABLE_OFF;
}

void btdmp_disableTransmit(int channel)
{
    REG_BTDMP_TRANSMIT_ENABLE(channel) = BTDMP_TRANSMIT_ENABLE_OFF;
}

void btdmp_flushTransmitFifo(int channel)
{
    REG_BTDMP_TRANSMIT_FIFO_CONFIG(channel) = BTDMP_TRANSMIT_FIFO_CONFIG_FLUSH;
}