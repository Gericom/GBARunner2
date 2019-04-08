#pragma once

#define SPI_CNT_RATE_4M         (0 << 0)   
#define SPI_CNT_RATE_2M         (1 << 0)
#define SPI_CNT_RATE_1M         (2 << 0)
#define SPI_CNT_RATE_512K       (3 << 0)

#define SPI_CNT_BUSY            (1 << 7)

#define SPI_CNT_DEV_POWER       (0 << 8)
#define SPI_CNT_DEV_NVRAM       (1 << 8)
#define SPI_CNT_DEV_TOUCH       (2 << 8)
#define SPI_CNT_DEV_RESERVED    (3 << 8)

#define SPI_CNT_WIDTH_8BIT      (0 << 10)
#define SPI_CNT_WIDTH_16BIT     (1 << 10)

#define SPI_CNT_CS_HOLD         (1 << 11)

#define SPI_CNT_IRQ             (1 << 14)

#define SPI_CNT_BUS_ENABLE      (1 << 15)

#define REG_SPI_CNT     (*(vu16*)0x040001C0)
#define REG_SPI_DATA    (*(vu16*)0x040001C2)

u8 spi_transferByte(u8 value);