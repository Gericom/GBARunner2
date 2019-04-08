#pragma once

#include "wifi_flash.h"

#define NVRAM_CMD_PAGE_PROGRAM      0x02
#define NVRAM_CMD_READ              0x03
#define NVRAM_CMD_WRITE_DISABLE     0x04
#define NVRAM_CMD_READ_STATUS       0x05
#define NVRAM_CMD_WRITE_ENABLE      0x06
#define NVRAM_CMD_PAGE_WRITE        0x0A
#define NVRAM_CMD_READ_FAST         0x0B
#define NVRAM_CMD_READ_ID           0x9F
#define NVRAM_CMD_DEEP_WAKEUP       0xAB
#define NVRAM_CMD_DEEP_SLEEP        0xB9
#define NVRAM_CMD_SECTOR_ERASE      0xD8
#define NVRAM_CMD_PAGE_ERASE        0xDB

struct nvram_data_t
{
    u8 header[0x2A];
    wifi_flashdata_t wifiData;    
};

static_assert(sizeof(nvram_data_t) == 0x200, "Invalid nvram_data_t size!");
