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

#define WIFI_VERSION_DS_PHAT		0x1440
#define WIFI_VERSION_DS_LITE		0xC340

#define REG_WIFI_VERSION			(*(vu16*)0x4808000)
#define REG_WIFI_MAC_CMD			(*(vu16*)0x4808004)
#define REG_WIFI_MAC_CONFIG			(*(vu16*)0x4808006)

#define REG_WIFI_TX_CONFIG			(*(vu16*)0x4808008)

#define REG_WIFI_RX_CONFIG			(*(vu16*)0x480800A)

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

#define REG_WIFI_ISR				(*(vu16*)0x4808010)
#define REG_WIFI_IMR				(*(vu16*)0x4808012)

#define REG_WIFI_MAC_ADRS			(*(volatile wifi_macaddr_t*)0x4808018)
#define REG_WIFI_BSSID				(*(volatile wifi_macaddr_t*)0x4808020)

#define REG_WIFI_KSID				(*(vu16*)0x4808028) //key related?
#define REG_WIFI_AID				(*(vu16*)0x480802A)
#define REG_WIFI_RETRY_LIMIT		(*(vu16*)0x480802C)

#define REG_WIFI_WEP_CONFIG			(*(vu16*)0x4808032)


#define WIFI_SHUTDOWN_POWERON		0
#define WIFI_SHUTDOWN_POWEROFF		1

#define REG_WIFI_SHUTDOWN			(*(vu16*)0x4808036)

#define WIFI_WAKEUP_CTRL_AUTO_WAKEUP	1
#define WIFI_WAKEUP_CTRL_AUTO_SLEEP		2 //target measurement pilot transmission time (tmptt) power save

#define REG_WIFI_WAKEUP_CTRL		(*(vu16*)0x4808038)
#define REG_WIFI_SET_POWER			(*(vu16*)0x480803C)
#define REG_WIFI_SET_POWER_FORCE	(*(vu16*)0x4808040)

#define REG_WIFI_MSEQ16				(*(vu16*)0x4808044)

#define REG_WIFI_MP_POWER_SEQ		(*(vu16*)0x4808048)

#define REG_WIFI_BCN_PARAM			(*(vu16*)0x4808088)
#define REG_WIFI_BEACON_PERIOD		(*(vu16*)0x480808C)
#define REG_WIFI_TIM_COUNT			(*(vu16*)0x480808E)

#define REG_WIFI_TX_PREAMBLE_TYPE	(*(vu16*)0x48080BC)

#define WIFI_BUFFERING_SELECT_BSSID	(1 << 10)

#define REG_WIFI_BUFFERING_SELECT	(*(vu16*)0x48080D0)

#define REG_WIFI_RESPONSE_CONTROL	(*(vu16*)0x48080D4)
#define REG_WIFI_OVF_THRESHOLD		(*(vu16*)0x48080D8)

#define REG_WIFI_DEFRAG_OFFSET		(*(vu16*)0x48080DA)

#define WIFI_DS_MASK_DATA_IGNORE	(1 << 0)

#define REG_WIFI_DS_MASK			(*(vu16*)0x48080E0)

#define REG_WIFI_TSF_ENABLE			(*(vu16*)0x48080E8)
#define REG_WIFI_TBTT_ENABLE		(*(vu16*)0x48080EA)

#define REG_WIFI_NAV_ENABLE			(*(vu16*)0x48080EC)
#define REG_WIFI_TMPTT_ENABLE		(*(vu16*)0x48080EE)

#define REG_WIFI_NEXT_TBTT_TSF_0	(*(vu16*)0x48080F0)
#define REG_WIFI_NEXT_TBTT_TSF_1	(*(vu16*)0x48080F2)
#define REG_WIFI_NEXT_TBTT_TSF_2	(*(vu16*)0x48080F4)
#define REG_WIFI_NEXT_TBTT_TSF_3	(*(vu16*)0x48080F6)

#define REG_WIFI_TSF_0				(*(vu16*)0x48080F8)
#define REG_WIFI_TSF_1				(*(vu16*)0x48080FA)
#define REG_WIFI_TSF_2				(*(vu16*)0x48080FC)
#define REG_WIFI_TSF_3				(*(vu16*)0x48080FE)

#define REG_WIFI_PRE_TBTT			(*(vu16*)0x4808110)

#define REG_WIFI_RDY_TIMEOUT		(*(vu16*)0x4808120)
#define REG_WIFI_RX_TIMEOUT			(*(vu16*)0x4808122)
#define REG_WIFI_TBTT_ACT_TIME		(*(vu16*)0x4808124)
#define REG_WIFI_TMPTT_ACT_TIME		(*(vu16*)0x4808128)

#define REG_WIFI_TIMEOUT_PARAM		(*(vu16*)0x4808130)

#define REG_WIFI_ACK_CCA_TIMEOUT	(*(vu16*)0x4808132)

#define REG_WIFI_ACTIVE_ZONE_TIMER	(*(vu16*)0x4808134)
#define REG_WIFI_TSF_TXOFFSET		(*(vu16*)0x4808140)

#define REG_WIFI_TSF_RXOFFSET		(*(vu16*)0x4808142)
#define REG_WIFI_CCA_DELAY			(*(vu16*)0x4808144)
#define REG_WIFI_TXPE_OFF_DELAY		(*(vu16*)0x4808146)
#define REG_WIFI_TX_DELAY			(*(vu16*)0x4808148)
#define REG_WIFI_RX_DELAY			(*(vu16*)0x480814A)
#define REG_WIFI_TRX_PE_INTERVAL	(*(vu16*)0x480814C)

#define REG_WIFI_RF_WAKEUP_TIME			(*(vu16*)0x4808150)
#define REG_WIFI_MultiAck_Delay_TIME	(*(vu16*)0x4808154)

#define WIFI_RF_PIN_TXMAIN			(1 << 1)
#define WIFI_RF_PIN_TXON			(1 << 6)
#define WIFI_RF_PIN_RXON			(1 << 7)

#define REG_WIFI_RF_PINS			(*(vu16*)0x480819C)

#define REG_WIFI_TRPE_DIRECT_CTL	(*(vu16*)0x48081A0)
#define REG_WIFI_SERIAL_DAT_SELECT	(*(vu16*)0x48081A2)

#define REG_WIFI_RF_STAT			(*(vu16*)0x4808214)

#define REG_WIFI_DDO_PARA_DAT		(*(vu16*)0x4808254)

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
void wifi_deinit();
void wifi_stop();
void wifi_start();
