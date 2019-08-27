#include <nds.h>
#include "wifi.h"
#include "wifi_rf.h"

static bool waitRFReady()
{
	for (int i = 0; i < 10240; i++)
		if (!REG_WIFI_RFR_STATUS)
			return true;
	return false;
}

void wifi_writeRF(u32 val)
{
	REG_WIFI_RFR_DAT = val & 0xFFFF;
	REG_WIFI_RFR_CMD = val >> 16;
	waitRFReady();
}