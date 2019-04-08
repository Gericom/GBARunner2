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

#define REG_WIFI_TXBUF_WR_ADDR		(*(vu16*)0x4808068)
#define REG_WIFI_TXBUF_WR_COUNT		(*(vu16*)0x480806C)
#define REG_WIFI_TXBUF_WR_DATA		(*(vu16*)0x4808070)

#define REG_WIFI_TXBUF_GAP			(*(vu16*)0x4808074)
#define REG_WIFI_TXBUF_GAPDISP		(*(vu16*)0x4808076)

#define REG_WIFI_TXREQ_BEACON		(*(vu16*)0x4808080)
#define REG_WIFI_TXREQ_BEACON_TIM	(*(vu16*)0x4808084)

#define REG_WIFI_TXREQ_CMD			(*(vu16*)0x4808090)
#define REG_WIFI_TXREQ_LOC1			(*(vu16*)0x48080A0)
#define REG_WIFI_TXREQ_LOC2			(*(vu16*)0x48080A4)
#define REG_WIFI_TXREQ_LOC3			(*(vu16*)0x48080A8)

#define WIFI_TXREQ_LOC1				(1 << 0)
#define WIFI_TXREQ_CMD				(1 << 1)
#define WIFI_TXREQ_LOC2				(1 << 2)
#define WIFI_TXREQ_LOC3				(1 << 3)
#define WIFI_TXREQ_BEACON			(1 << 4)

#define WIFI_TXREQ_LOC_ALL			(WIFI_TXREQ_LOC1 | WIFI_TXREQ_LOC2 | WIFI_TXREQ_LOC3)

#define REG_WIFI_TXREQ_EN_RESET		(*(vu16*)0x48080AC)
#define REG_WIFI_TXREQ_EN_SET		(*(vu16*)0x48080AE)
#define REG_WIFI_TXREQ_EN_STAT		(*(vu16*)0x48080B0)

#define REG_WIFI_TXREQ_RESET		(*(vu16*)0x48080B4)

#define REG_WIFI_TXREQ_BUSY			(*(vu16*)0x48080B6)
#define REG_WIFI_TXREQ_RESULT		(*(vu16*)0x48080B8)


void wifi_irqTxStart();
void wifi_irqTxEnd();
void wifi_initTx();

typedef void (*wifi_tx_done_callback_t)(void* arg);

void wifi_setTxDoneCallback(wifi_tx_done_callback_t callback, void* arg);