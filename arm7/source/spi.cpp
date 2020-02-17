#include <nds.h>
#include "spi.h"

void spi_waitBusy()
{
    while(REG_SPI_CNT & SPI_CNT_BUSY);
}

u8 spi_transferByte(u8 value)
{
    REG_SPI_DATA = value;
    spi_waitBusy();
    return REG_SPI_DATA;
}