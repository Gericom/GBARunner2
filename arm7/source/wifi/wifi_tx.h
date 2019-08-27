#pragma once
#include "wifi.h"

#define WIFI_PKT_HW_HEADER_TX_STATUS_ERROR			0
#define WIFI_PKT_HW_HEADER_TX_STATUS_OK				1

#define WIFI_PKT_HW_HEADER_TX_SERVICE_RATE_1MBPS	10
#define WIFI_PKT_HW_HEADER_TX_SERVICE_RATE_2MBPS	20

struct wifi_pkt_hw_header_tx_t
{
	u16 status;
	u16 status2;
	u16 rsvRetryCount;
	u16 rsvAppRate;
	u16 serviceRate;
	u16 mpdu;
};

struct wifi_pkt_tx_t
{
	wifi_pkt_hw_header_tx_t hwHeader;
	wifi_pkt_ieee_header_t ieeeHeader;
	//u8 payload[0];
	//u32 fcs;
};

#define REG_WIFI_WDMA_STR			(*(vu16*)0x4808068)
#define REG_WIFI_WDMA_CNT			(*(vu16*)0x480806C)
#define REG_WIFI_WDMA_PORT			(*(vu16*)0x4808070)

#define REG_WIFI_WDMA_JUMP			(*(vu16*)0x4808074)
#define REG_WIFI_WDMA_JUMP_CNT		(*(vu16*)0x4808076)

#define REG_WIFI_BEACON_ADRS		(*(vu16*)0x4808080)
#define REG_WIFI_BCN_PARAM_OFFSET	(*(vu16*)0x4808084)

#define REG_WIFI_MP_ADR				(*(vu16*)0x4808090)
#define REG_WIFI_TXQ0_ADR			(*(vu16*)0x48080A0)
#define REG_WIFI_TXQ1_ADR			(*(vu16*)0x48080A4)
#define REG_WIFI_TXQ2_ADR			(*(vu16*)0x48080A8)

#define WIFI_TXQ_TXQ0				(1 << 0)
#define WIFI_TXQ_MP					(1 << 1)
#define WIFI_TXQ_TXQ1				(1 << 2)
#define WIFI_TXQ_TXQ2				(1 << 3)
#define WIFI_TXQ_BEACON				(1 << 4)

#define WIFI_TXQ_TXQ_ALL			(WIFI_TXQ_TXQ0 | WIFI_TXQ_TXQ1 | WIFI_TXQ_TXQ2)

#define REG_WIFI_QUEUE_CLOSE		(*(vu16*)0x48080AC)
#define REG_WIFI_QUEUE_OPEN			(*(vu16*)0x48080AE)
#define REG_WIFI_QUEUE_MAP			(*(vu16*)0x48080B0)

#define REG_WIFI_TXQ_RESET			(*(vu16*)0x48080B4)

#define REG_WIFI_TXREQ_BUSY			(*(vu16*)0x48080B6)
#define REG_WIFI_TX_STATUS			(*(vu16*)0x48080B8)

#define WIFI_TX_TEST_FRAME_FC_DUR_OFF	(1 << 0)
#define WIFI_TX_TEST_FRAME_CRC_OFF		(1 << 1)
#define WIFI_TX_TEST_FRAME_SEQ_OFF		(1 << 2)

#define REG_WIFI_TX_TEST_FRAME		(*(vu16*)0x4808194)


void wifi_irqTxStart();
void wifi_irqTxEnd();
void wifi_initTx();

typedef void (*wifi_tx_done_callback_t)(void* arg);

void wifi_setTxDoneCallback(wifi_tx_done_callback_t callback, void* arg);