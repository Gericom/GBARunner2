#pragma once

#define REG_WIFI_RF_DATA2			(*(vu16*)0x480817C)
#define REG_WIFI_RF_DATA1			(*(vu16*)0x480817E)
#define REG_WIFI_RF_BUSY			(*(vu16*)0x4808180)
#define REG_WIFI_RF_CNT				(*(vu16*)0x4808184)

void wifi_writeRF(u32 val);