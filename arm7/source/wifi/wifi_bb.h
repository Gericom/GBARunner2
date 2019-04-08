#pragma once

#define WIFI_BB_CCA_MODE_CS				0
#define WIFI_BB_CCA_MODE_ED				1
#define WIFI_BB_CCA_MODE_CS_OR_ED		2
#define WIFI_BB_CCA_MODE_CS_AND_ED		3
#define WIFI_BB_CCA_MODE_MAX			WIFI_BB_CCA_MODE_CS_AND_ED

#define WIFI_BB_ED_THRES_MAX	0x3F

#define WIFI_BB_REG_01			0x01
#define WIFI_BB_REG_CCA_MODE	0x13
#define WIFI_BB_REG_GAIN		0x1E
#define WIFI_BB_REG_ED_THRES	0x35


#define WIFI_BB_CNT_WRITE		0x5000
#define WIFI_BB_CNT_READ		0x6000

#define REG_WIFI_BB_CNT			(*(vu16*)0x4808158)
#define REG_WIFI_BB_WRITE		(*(vu16*)0x480815A)
#define REG_WIFI_BB_READ		(*(vu16*)0x480815C)
#define REG_WIFI_BB_BUSY		(*(vu16*)0x480815E)

#define WIFI_BB_MODE_NORMAL		0x100

#define REG_WIFI_BB_MODE		(*(vu16*)0x4808160)

#define REG_WIFI_BB_POWER		(*(vu16*)0x4808168)

u8 wifi_readBBReg(u8 reg);
bool wifi_writeBBReg(u8 reg, u8 val);
void wifi_initBB();
bool wifi_setReceiveMode(u8 ccaMode, u8 edThreshold);