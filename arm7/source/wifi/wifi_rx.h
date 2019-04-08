#pragma once

#define REG_WIFI_RXCNT				(*(vu16*)0x4808030)

#define REG_WIFI_RXBUF_BEGIN		(*(vu16*)0x4808050)
#define REG_WIFI_RXBUF_END			(*(vu16*)0x4808052)

#define REG_WIFI_RXBUF_WR_CSR		(*(vu16*)0x4808054)
#define REG_WIFI_RXBUF_WR_ADDR		(*(vu16*)0x4808056)

#define REG_WIFI_RXBUF_RD_ADDR		(*(vu16*)0x4808058)
#define REG_WIFI_RXBUF_RD_CSR		(*(vu16*)0x480805A)
#define REG_WIFI_RXBUF_RD_COUNT		(*(vu16*)0x480805C)
#define REG_WIFI_RXBUF_RD_DATA		(*(vu16*)0x4808060)

#define REG_WIFI_RXBUF_GAP			(*(vu16*)0x4808062)
#define REG_WIFI_RXBUF_GAPDISP		(*(vu16*)0x4808064)

#define REG_WIFI_824E				(*(vu16*)0x480824E)

void wifi_irqRxStart();
void wifi_irqRxEnd();
void wifi_initRx();

typedef void(*wifi_rx_done_callback_t)(void* arg);

void wifi_setRxDoneCallback(wifi_rx_done_callback_t callback, void* arg);