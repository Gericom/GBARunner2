#include <nds.h>
#include "wifi.h"
#include "../spi.h"
#include "nvram.h"
#include "../irq.h"
#include "wifi_flash.h"

void wifi_initFlashData()
{
	int oldIrq = irq_masterDisable();
    {
        //Read command
        REG_SPI_CNT = SPI_CNT_BUS_ENABLE | SPI_CNT_WIDTH_8BIT | SPI_CNT_CS_HOLD | SPI_CNT_DEV_NVRAM | SPI_CNT_RATE_4M;
        spi_transferByte(NVRAM_CMD_READ);

        //Address
        spi_transferByte((0 >> 16) & 0xFF);
        spi_transferByte((0 >> 8) & 0xFF);
        spi_transferByte((0) & 0xFF);

        u16* dst = (u16*)&WIFI_RAM->firmData;
        //Read 2 bytes a time, because wifi ram does not support byte writes
        for(int i = 0; i < 512; i += 2) 
        {            
            u8 a = spi_transferByte(0);
            u8 b = spi_transferByte(0);
            *dst++ = a | (b << 8);
        }

        REG_SPI_CNT = 0;
    }
	irq_masterRestore(oldIrq);
}