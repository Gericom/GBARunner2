#include <nds.h>
#include "wifi.h"
#include "wifi_bb.h"

static bool waitBBReady()
{
	for (int i = 0; i < 10240; i++)
		if (!REG_WIFI_BB_BUSY)
			return true;
	return false;
}

u8 wifi_readBBReg(u8 reg)
{
	REG_WIFI_BB_CNT = reg | WIFI_BB_CNT_READ;
	waitBBReady();
	return REG_WIFI_BB_READ;
}

bool wifi_writeBBReg(u8 reg, u8 val)
{
	REG_WIFI_BB_WRITE = val;
	REG_WIFI_BB_CNT = reg | WIFI_BB_CNT_WRITE;
	return waitBBReady();
}

void wifi_initBB()
{
	REG_WIFI_BB_MODE = WIFI_BB_MODE_NORMAL;
	for(int i = 0; i < 0x69; i++)
		wifi_writeBBReg(i, WIFI_RAM->firmData.wifiData.bbConfig[i]);

	wifi_writeBBReg(0x5A, 2);
}

bool wifi_setReceiveMode(u8 ccaMode, u8 edThreshold)
{
	if (ccaMode > WIFI_BB_CCA_MODE_MAX || edThreshold > WIFI_BB_ED_THRES_MAX)
		return false;
	wifi_writeBBReg(WIFI_BB_REG_CCA_MODE, ccaMode);
	wifi_writeBBReg(WIFI_BB_REG_ED_THRES, edThreshold);
	return true;
}