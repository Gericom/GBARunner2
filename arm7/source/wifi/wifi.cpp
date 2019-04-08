#include <nds.h>
#include <string.h>
#include "../irq.h"
#include "wifi_common.h"
#include "wifi_rx.h"
#include "wifi_tx.h"
#include "wifi_bb.h"
#include "wifi_rf.h"
#include "wifi_flash.h"
#include "wifi.h"

wifi_work_t gWifiWork;

void wifi_irqPreTbtt()
{
	REG_WIFI_IF = WIFI_IRQ_PRE_TBTT;
}

void wifi_irqTbtt()
{
	REG_WIFI_IF = WIFI_IRQ_TBTT;
}

void wifi_irqActEnd()
{
	REG_WIFI_IF = WIFI_IRQ_ACT_END;
	REG_WIFI_TXREQ_EN_RESET = WIFI_TXREQ_LOC_ALL;
}

void wifi_irqRfWakeup()
{
	REG_WIFI_IF = WIFI_IRQ_RF_WAKEUP;
}

void wifi_irqTxErr()
{
	REG_WIFI_IF = WIFI_IRQ_TX_ERROR;
}

void wifi_irqRxCntup()
{
	REG_WIFI_IF = WIFI_IRQ_RX_COUNT;
}

void wifi_irqCntOvf()
{
	REG_WIFI_IF = WIFI_IRQ_RX_COUNT_OVERFLOW | WIFI_IRQ_TX_ERROR_OVERFLOW;
}

void wifi_irqMpEnd()
{
	REG_WIFI_IF = WIFI_IRQ_MP_END;
}

extern "C" void wifi_irq()
{
	while(true)
	{
		u16 irqFlags = REG_WIFI_IF & REG_WIFI_IE;
		if (!irqFlags)
			break;
		if (irqFlags & WIFI_IRQ_TX_START)
			wifi_irqTxStart();
		if (irqFlags & WIFI_IRQ_RX_START)
			wifi_irqRxStart();
		if (irqFlags & WIFI_IRQ_PRE_TBTT)
			wifi_irqPreTbtt();
		if (irqFlags & WIFI_IRQ_TBTT)
			wifi_irqTbtt();
		if (irqFlags & WIFI_IRQ_ACT_END)
			wifi_irqActEnd();
		if (irqFlags & WIFI_IRQ_RF_WAKEUP)
			wifi_irqRfWakeup();
		if (irqFlags & WIFI_IRQ_TX_ERROR)
			wifi_irqTxErr();
		if (irqFlags & WIFI_IRQ_RX_COUNT)
			wifi_irqRxCntup();
		if (irqFlags & WIFI_IRQ_RX_END)
			wifi_irqRxEnd();
		if (irqFlags & (WIFI_IRQ_RX_COUNT_OVERFLOW | WIFI_IRQ_TX_ERROR_OVERFLOW))
			wifi_irqCntOvf();
		if (irqFlags & WIFI_IRQ_TX_END)
			wifi_irqTxEnd();
		if (irqFlags & WIFI_IRQ_MP_END)
			wifi_irqMpEnd();
	}
}

bool wifi_setMode(u8 mode)
{
	if(mode > 3)
		return false;
	REG_WIFI_MODE_WEP = (REG_WIFI_MODE_WEP & 0xFFF8) | mode;
	return true;
}

bool wifi_setWepMode(u8 mode)
{
	if(mode > 3)
		return false;
	REG_WIFI_MODE_WEP = (REG_WIFI_MODE_WEP & 0xFFC7) | (mode << 3);
	return true;
}

void wifi_setMacAddress(const wifi_macaddr_t* address)
{
	//write in 16 bits, wifi ram does not support byte writes
	REG_WIFI_MACADDR.address16[0] = address->address16[0];
	REG_WIFI_MACADDR.address16[1] = address->address16[1];
	REG_WIFI_MACADDR.address16[2] = address->address16[2];
}

void wifi_setBssid(const wifi_macaddr_t* bssid)
{
	//write in 16 bits, wifi ram does not support byte writes
	REG_WIFI_BSSID.address16[0] = bssid->address16[0];
	REG_WIFI_BSSID.address16[1] = bssid->address16[1];
	REG_WIFI_BSSID.address16[2] = bssid->address16[2];
}

void wifi_setRetryLimit(u8 limit)
{
	REG_WIFI_RETRYLIMIT = limit;
}

void wifi_setUseTmpttPowerSave(bool powerSave)
{
	if (powerSave)
		REG_WIFI_POWERSAVE |= WIFI_POWERSAVE_AUTO_SLEEP;
	else
	{
		REG_WIFI_POWERSAVE &= ~WIFI_POWERSAVE_AUTO_SLEEP;
		REG_WIFI_8048 = 0;
	}
}

void wifi_setPowerState(int state)
{
	REG_WIFI_POWERSTATE = state;
}

bool wifi_setBeaconInterval(u16 interval)
{
	if (interval < 10 || interval > 1000)
		return false;
	REG_WIFI_BEACON_INTERVAL = interval;
	return true;
}

bool wifi_setDtimInterval(u8 interval)
{
	if (interval == 0)
		return false;
	REG_WIFI_DTIM_INTERVAL = interval;
	REG_WIFI_DTIM_COUNTER = 0;
}

void wifi_updateTxTimeStampOffset()
{	
	u16 offset = WIFI_RAM->firmData.wifiData.wifiTxTimestampOffs + 0x202;
	if(gWifiWork.use2Mbps)
	{
		offset -= 0x6161;
		if (REG_WIFI_PREAMBLE & 2)
			offset -= 0x6060;
	}
	REG_WIFI_TX_TIMESTAMP_OFFS = offset;
}

void wifi_setUse2Mbps(bool use2Mbps)
{
	gWifiWork.use2Mbps = use2Mbps;
	wifi_updateTxTimeStampOffset();
}

void wifi_setShortPreamble(bool shortPreamble)
{
	if(shortPreamble)
		REG_WIFI_PREAMBLE |= 6;
	else
		REG_WIFI_PREAMBLE &= ~6;
	wifi_updateTxTimeStampOffset();
}

bool wifi_setChannel(int channel)
{
	if(channel < 1 || channel > 14)
		return false;
	channel -= 1;
	int oldPower = REG_WIFI_POWERFORCE;
	REG_WIFI_POWERFORCE = 0x8001;
	while(true)
	{
		u16 powerState = REG_WIFI_POWERSTATE;
		u16 stat = REG_WIFI_RF_STAT;
		if((powerState >> 8) == 2 && (stat == 0 || stat == 2))
			break;
	}
	if(WIFI_RAM->firmData.wifiData.rfType == 2 || WIFI_RAM->firmData.wifiData.rfType == 5)
	{
		u32 val = 0;
		memcpy(&val, &WIFI_RAM->firmData.wifiData.rest[0x24 + channel * 6], 3);
		wifi_writeRF(val);
		memcpy(&val, &WIFI_RAM->firmData.wifiData.rest[0x27 + channel * 6], 3);
		wifi_writeRF(val);

		if(gWifiWork.bkReg & 0x10000)
		{
			if(!(gWifiWork.bkReg & 0x8000))
			{
				u8 n = WIFI_RAM->firmData.wifiData.rest[0x86 + channel];
				wifi_writeRF(gWifiWork.bkReg | ((n & 0x1F) << 10));
			}
		}
		else
			wifi_writeBBReg(WIFI_BB_REG_GAIN, WIFI_RAM->firmData.wifiData.rest[0x78 + channel]);
	}
	else if(WIFI_RAM->firmData.wifiData.rfType == 3)
	{
		int n = WIFI_RAM->firmData.wifiData.rfEntryCount;
		int count = WIFI_RAM->firmData.wifiData.rest[n++];
		for(int i = 0; i < count; i++) 
		{
			int reg = WIFI_RAM->firmData.wifiData.rest[n];
			int val = WIFI_RAM->firmData.wifiData.rest[n + 1 + channel];
			wifi_writeBBReg(reg, val);
			n += 15;
		}
		for(int i = 0; i < WIFI_RAM->firmData.wifiData.unk43; i++) 
		{
			int reg = WIFI_RAM->firmData.wifiData.rest[n];
			int val = WIFI_RAM->firmData.wifiData.rest[n + 1 + channel];
			wifi_writeRF((reg << 8) | val | 0x50000);
			n += 15;
		}
	}
	REG_WIFI_POWERFORCE = oldPower;
	REG_WIFI_8048 = 3;
}

bool wifi_setActiveZoneTime(u16 time)
{
	if (time < 10)
		return false;
	REG_WIFI_ACTIVE_ZONE_TIME = time;
}

void wifi_initRF()
{
	//initialize the mac tx rx registers from firmware
	wifi_flashdata_t* flashData = &WIFI_RAM->firmData.wifiData;
	REG_WIFI_8146 = flashData->wifi8146;
	REG_WIFI_8148 = flashData->wifi8148;
	REG_WIFI_814A = flashData->wifi814A;
	REG_WIFI_814C = flashData->wifi814C;
	REG_WIFI_8120 = flashData->wifi8120;
	REG_WIFI_8122 = flashData->wifi8122;
	REG_WIFI_8154 = flashData->wifi8154;
	REG_WIFI_8144 = flashData->wifi8144;
	REG_WIFI_8132 = flashData->wifi8132_1;
	REG_WIFI_8132 = flashData->wifi8132_2;
	REG_WIFI_TX_TIMESTAMP_OFFS = flashData->wifiTxTimestampOffs;
	REG_WIFI_8142 = flashData->wifi8142;
	REG_WIFI_POWERSAVE = flashData->wifiPowersave;
	REG_WIFI_8124 = flashData->wifi8124;
	REG_WIFI_8128 = flashData->wifi8128;
	REG_WIFI_8150 = flashData->wifi8150;
	REG_WIFI_RF_CNT = ((flashData->rfBitsPerEntry >> 7) << 8) | (flashData->rfBitsPerEntry & 0x7F);
	if(flashData->rfType == 3)
	{
		//write rf calibration data for version 3
		gWifiWork.bbpCnt = flashData->rest[flashData->rfEntryCount];
		for(int i = 0; i < flashData->rfEntryCount; i++)
			wifi_writeRF(flashData->rest[i] | (i << 8) | 0x50000);
	}
	else
	{
		//write rf calibration data for others
		int entryLength = ((flashData->rfBitsPerEntry & 0x1F) + 7) / 8;
		for(int i = 0; i < flashData->rfEntryCount; i++)
		{
			u32 data = 0;
			memcpy(&data, &flashData->rest[i * entryLength], entryLength);
			wifi_writeRF(data);
			if(flashData->rfType == 2 && (data >> 18) == 9)
				gWifiWork.bkReg = data & ~0x7C00;
		}
	}
}

void wifi_wakeup()
{
	REG_WIFI_POWER = WIFI_POWER_ENABLE;
	swiDelay(8 * 0x20BA);//we should wait for 8ms
	REG_WIFI_BB_POWER = 0;
	if (WIFI_RAM->firmData.wifiData.rfType == 2)
	{
		u8 val = wifi_readBBReg(WIFI_BB_REG_01);
		wifi_writeBBReg(WIFI_BB_REG_01, val & 0x7F);
		wifi_writeBBReg(WIFI_BB_REG_01, val);
		swiDelay(40 * 0x20BA);//we should wait for 40ms
	}
	if (WIFI_RAM->firmData.wifiData.rfType == 2 || WIFI_RAM->firmData.wifiData.rfType == 3)
		wifi_initRF();
}

void wifi_shutdown()
{
	if (WIFI_RAM->firmData.wifiData.rfType == 2)
		wifi_writeRF(0xC008);
	u8 val = wifi_readBBReg(WIFI_BB_REG_GAIN);
	wifi_writeBBReg(WIFI_BB_REG_GAIN, val | 0x3F);
	REG_WIFI_BB_POWER = 0x800D;
	REG_WIFI_POWER = WIFI_POWER_DISABLE;
}

void wifi_initMac()
{
	REG_WIFI_MODE_RST = 0;
	REG_WIFI_TXSTATCNT = 0;
	REG_WIFI_800A = 0;
	REG_WIFI_IE = 0;
	REG_WIFI_IF = 0xFFFF;
	REG_WIFI_8254 = 0;
	REG_WIFI_TXREQ_RESET = 0xFFFF;
	REG_WIFI_TXREQ_BEACON = 0;
	REG_WIFI_DTIM_INTERVAL = 1;
	REG_WIFI_DTIM_COUNTER = 0;
	REG_WIFI_AID = 0;
	REG_WIFI_KSID = 0;
	REG_WIFI_US_COUNTCNT = 0;
	REG_WIFI_US_COMPARECNT = 0;
	REG_WIFI_CMD_COUNTCNT = 1;
	REG_WIFI_80EC = 0x3F03;
	REG_WIFI_81A2 = 1;
	REG_WIFI_81A0 = 0;
	REG_WIFI_TX_PRE_TBTT = 0x800;
	REG_WIFI_PREAMBLE = 1;
	REG_WIFI_80D4 = 3;
	REG_WIFI_80D8 = 4;
	REG_WIFI_RX_LEN_CROP = 0x602;
	REG_WIFI_TXBUF_GAPDISP = 0;
	REG_WIFI_8130 = 0x146;
}

void wifi_init()
{
	memset(&gWifiWork, 0, sizeof(gWifiWork));

	//enable power
	(*(vu32*)0x04000304) |= 2;
	//set waitstates
	(*(vu16*)0x04000206) = 0x30;

	wifi_initFlashData();

	wifi_wakeup();
	wifi_initMac();
	wifi_initRF();
	wifi_initBB();
	wifi_setMacAddress(&WIFI_RAM->firmData.wifiData.macAddress);
	wifi_setRetryLimit(7);
	wifi_setMode(2);
	wifi_setUse2Mbps(true);
	wifi_setWepMode(0);
	wifi_setBeaconInterval(500);
	wifi_setActiveZoneTime(0xFFFF);
	wifi_setShortPreamble(true);

	wifi_setChannel(13);

	wifi_setReceiveMode(WIFI_BB_CCA_MODE_CS, 0x1F);
	//wifi_shutdown();

	REG_IRQ_IE |= (1 << 24);
}

void wifi_stop()
{
	REG_WIFI_IE = 0;
	REG_WIFI_MODE_RST = 0;
	REG_WIFI_US_COMPARECNT = 0;
	REG_WIFI_US_COUNTCNT = 0;
	REG_WIFI_TXSTATCNT = 0;
	REG_WIFI_800A = 0;
	REG_WIFI_TXREQ_EN_RESET = 0xFFFF;
	REG_WIFI_TXREQ_RESET = 0xFFFF;
}

void wifi_start()
{
	wifi_stop();
	REG_WIFI_WEP_CNT = 0x8000;
	REG_WIFI_ACTIVE_ZONE_TIME = 0xFFFF;
	REG_WIFI_AID = 0;
	REG_WIFI_KSID = 0;
	REG_WIFI_POWERSAVE = 0xF;
	wifi_initTx();
	wifi_initRx();
	REG_WIFI_RXCNT = 0x8000;
	REG_WIFI_IF = 0xFFFF;
	REG_WIFI_IE = WIFI_IRQ_TX_END | WIFI_IRQ_RX_END;

	REG_WIFI_TXSTATCNT = 0;
	REG_WIFI_800A = 0;

	REG_WIFI_MODE_RST = 1;
	REG_WIFI_8048 = 0;
	wifi_setUseTmpttPowerSave(false);
	//REG_WIFI_TXREQ_EN_SET = WIFI_TXREQ_CMD;
	wifi_setPowerState(2);
	for (int i = 0; i < 4000; i++)
		if (REG_WIFI_RF_PINS & WIFI_RF_PIN_RXON)
			break;
}