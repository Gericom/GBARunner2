#include <nds.h>
#include "spi.h"

u8 spi_transferByte(u8 value)
{
    REG_SPI_DATA = value;
    while(REG_SPI_CNT & SPI_CNT_BUSY);
    return REG_SPI_DATA;
}