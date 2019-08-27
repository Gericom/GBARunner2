#pragma once

#define REG_WIFI_RFR_D_CTRL			(*(vu16*)0x4808168)
#define REG_WIFI_RFR_CMD			(*(vu16*)0x480817C)
#define REG_WIFI_RFR_DAT			(*(vu16*)0x480817E)
#define REG_WIFI_RFR_STATUS			(*(vu16*)0x4808180)
#define REG_WIFI_RFR_CONFIG			(*(vu16*)0x4808184)

void wifi_writeRF(u32 val);