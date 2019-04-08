#pragma once

#include "nvram.h"

struct wifi_work_t
{
	bool use2Mbps;
	u32 bkReg;
	u16 bbpCnt;
};

extern wifi_work_t gWifiWork;

struct wifi_pkt_ieee_header_t
{
	struct
	{
		u16 version : 2;
		u16 type : 2;
		u16 subType : 4;
		u16 toDS : 1;
		u16 fromDS : 1;
		u16 moreFrag : 1;
		u16 retry : 1;
		u16 powerMgmt : 1;
		u16 moreData : 1;
		u16 wep : 1;
		u16 order : 1;
	} frameCtrl;
	u16 durationId;
	wifi_macaddr_t addr1;
	wifi_macaddr_t addr2;
	wifi_macaddr_t addr3;
	struct
	{
		u16 seqNum : 12;
		u16 fragNum : 4;
	} seqCtrl;
};

#define WIFI_RAM_SIZE				0x2000
//self chosen values:
#define WIFI_RAM_TX_BUF_LENGTH		0x800
#define WIFI_RAM_RX_BUF_LENGTH		0x1560

#define WIFI_RAM_TX_BUF_OFFSET		(0x200)
#define WIFI_RAM_RX_BUF_OFFSET		(WIFI_RAM_TX_BUF_OFFSET + WIFI_RAM_TX_BUF_LENGTH)

struct wifi_ram_t
{
	nvram_data_t firmData;
	volatile u8 txBuf[WIFI_RAM_TX_BUF_LENGTH];
	volatile u8 rxBuf[WIFI_RAM_RX_BUF_LENGTH];
	u16 unk5F60[16];
	u16 wepKeys[4][16];
};

static_assert(sizeof(wifi_ram_t) == WIFI_RAM_SIZE, "Invalid wifi ram length!");

#define WIFI_RAM					((wifi_ram_t*)0x4804000)

#define WIFI_CHIPID_DS_PHAT			0x1440
#define WIFI_CHIPID_DS_LITE			0xC340

#define REG_WIFI_CHIPID				(*(vu16*)0x4808000)
#define REG_WIFI_MODE_RST			(*(vu16*)0x4808004)
#define REG_WIFI_MODE_WEP			(*(vu16*)0x4808006)

#define REG_WIFI_TXSTATCNT			(*(vu16*)0x4808008)

#define REG_WIFI_800A				(*(vu16*)0x480800A)

#define WIFI_IRQ_RX_END				(1 << 0)
#define WIFI_IRQ_TX_END				(1 << 1)
#define WIFI_IRQ_RX_COUNT			(1 << 2)
#define WIFI_IRQ_TX_ERROR			(1 << 3)
#define WIFI_IRQ_RX_COUNT_OVERFLOW	(1 << 4)
#define WIFI_IRQ_TX_ERROR_OVERFLOW	(1 << 5)
#define WIFI_IRQ_RX_START			(1 << 6)
#define WIFI_IRQ_TX_START			(1 << 7)
#define WIFI_IRQ_TXBUF_COUNT_EXP	(1 << 8)
#define WIFI_IRQ_RXBUF_COUNT_EXP	(1 << 9)
#define WIFI_IRQ_RF_WAKEUP			(1 << 11)
#define WIFI_IRQ_MP_END				(1 << 12)
#define WIFI_IRQ_ACT_END			(1 << 13)	//probably the active time zone
#define WIFI_IRQ_TBTT				(1 << 14)	//Target Beacon Transmission Time (TBTT)
#define WIFI_IRQ_PRE_TBTT			(1 << 15)

#define REG_WIFI_IF					(*(vu16*)0x4808010)
#define REG_WIFI_IE					(*(vu16*)0x4808012)

#define REG_WIFI_MACADDR			(*(volatile wifi_macaddr_t*)0x4808018)
#define REG_WIFI_BSSID				(*(volatile wifi_macaddr_t*)0x4808020)

#define REG_WIFI_KSID				(*(vu16*)0x4808028) //key related?
#define REG_WIFI_AID				(*(vu16*)0x480802A)
#define REG_WIFI_RETRYLIMIT			(*(vu16*)0x480802C)

#define REG_WIFI_WEP_CNT			(*(vu16*)0x4808032)


#define WIFI_POWER_ENABLE			0
#define WIFI_POWER_DISABLE			1

#define REG_WIFI_POWER				(*(vu16*)0x4808036)

#define WIFI_POWERSAVE_AUTO_WAKEUP	1
#define WIFI_POWERSAVE_AUTO_SLEEP	2 //target measurement pilot transmission time (tmptt) power save

#define REG_WIFI_POWERSAVE			(*(vu16*)0x4808038)
#define REG_WIFI_POWERSTATE			(*(vu16*)0x480803C)
#define REG_WIFI_POWERFORCE			(*(vu16*)0x4808040)

#define REG_WIFI_RANDOM				(*(vu16*)0x4808044)

#define REG_WIFI_8048				(*(vu16*)0x4808048)

#define REG_WIFI_DTIM_COUNTER		(*(vu16*)0x4808088)
#define REG_WIFI_BEACON_INTERVAL	(*(vu16*)0x480808C)
#define REG_WIFI_DTIM_INTERVAL		(*(vu16*)0x480808E)

#define REG_WIFI_PREAMBLE			(*(vu16*)0x48080BC)

#define REG_WIFI_RXFILTER			(*(vu16*)0x48080D0)

#define REG_WIFI_80D4				(*(vu16*)0x48080D4)
#define REG_WIFI_80D8				(*(vu16*)0x48080D8)

#define REG_WIFI_RX_LEN_CROP		(*(vu16*)0x48080DA)

#define REG_WIFI_RXFILTER2			(*(vu16*)0x48080E0)

#define REG_WIFI_US_COUNTCNT		(*(vu16*)0x48080E8)
#define REG_WIFI_US_COMPARECNT		(*(vu16*)0x48080EA)

#define REG_WIFI_80EC				(*(vu16*)0x48080EC)
#define REG_WIFI_CMD_COUNTCNT		(*(vu16*)0x48080EE)

#define REG_WIFI_US_COMPARE0		(*(vu16*)0x48080F0)
#define REG_WIFI_US_COMPARE1		(*(vu16*)0x48080F2)
#define REG_WIFI_US_COMPARE2		(*(vu16*)0x48080F4)
#define REG_WIFI_US_COMPARE3		(*(vu16*)0x48080F6)

#define REG_WIFI_US_COUNT0			(*(vu16*)0x48080F8)
#define REG_WIFI_US_COUNT1			(*(vu16*)0x48080FA)
#define REG_WIFI_US_COUNT2			(*(vu16*)0x48080FC)
#define REG_WIFI_US_COUNT3			(*(vu16*)0x48080FE)

#define REG_WIFI_TX_PRE_TBTT		(*(vu16*)0x4808110)

#define REG_WIFI_8120				(*(vu16*)0x4808120)
#define REG_WIFI_8122				(*(vu16*)0x4808122)
#define REG_WIFI_8124				(*(vu16*)0x4808124)
#define REG_WIFI_8128				(*(vu16*)0x4808128)

#define REG_WIFI_8130				(*(vu16*)0x4808130)

#define REG_WIFI_8132				(*(vu16*)0x4808132)

#define REG_WIFI_ACTIVE_ZONE_TIME	(*(vu16*)0x4808134)
#define REG_WIFI_TX_TIMESTAMP_OFFS	(*(vu16*)0x4808140)

#define REG_WIFI_8142				(*(vu16*)0x4808142)
#define REG_WIFI_8144				(*(vu16*)0x4808144)
#define REG_WIFI_8146				(*(vu16*)0x4808146)
#define REG_WIFI_8148				(*(vu16*)0x4808148)
#define REG_WIFI_814A				(*(vu16*)0x480814A)
#define REG_WIFI_814C				(*(vu16*)0x480814C)

#define REG_WIFI_8150				(*(vu16*)0x4808150)
#define REG_WIFI_8154				(*(vu16*)0x4808154)

#define WIFI_RF_PIN_TXMAIN			(1 << 1)
#define WIFI_RF_PIN_TXON			(1 << 6)
#define WIFI_RF_PIN_RXON			(1 << 7)

#define REG_WIFI_RF_PINS			(*(vu16*)0x480819C)

#define REG_WIFI_81A0				(*(vu16*)0x48081A0)
#define REG_WIFI_81A2				(*(vu16*)0x48081A2)

#define REG_WIFI_RF_STAT			(*(vu16*)0x4808214)

#define REG_WIFI_8254				(*(vu16*)0x4808254)

#define REG_WIFI_82B8				(*(vu16*)0x48082B8)

bool wifi_setMode(u8 mode);
bool wifi_setWepMode(u8 mode);
void wifi_setMacAddress(const wifi_macaddr_t* address);
void wifi_setRetryLimit(u8 limit);
bool wifi_setBeaconInterval(u16 interval);
bool wifi_setDtimInterval(u8 interval);

void wifi_updateTxTimeStampOffset();
void wifi_setUse2Mbps(bool use2Mbps);
void wifi_setShortPreamble(bool shortPreamble);
bool wifi_setChannel(int channel);
bool wifi_setActiveZoneTime(u16 time);

void wifi_init();
void wifi_stop();
void wifi_start();
