#pragma once

#define REG_WIFI_MDP_CONFIG			(*(vu16*)0x4808030)

#define REG_WIFI_RXBUF_STR			(*(vu16*)0x4808050)
#define REG_WIFI_RXBUF_END			(*(vu16*)0x4808052)

#define REG_WIFI_RXBUF_CUR			(*(vu16*)0x4808054)
#define REG_WIFI_RXBUF_WCUR			(*(vu16*)0x4808056)

#define REG_WIFI_RDMA_STR			(*(vu16*)0x4808058)
#define REG_WIFI_RXBUF_BNR			(*(vu16*)0x480805A)
#define REG_WIFI_RDMA_CNT			(*(vu16*)0x480805C)
#define REG_WIFI_RDMA_PORT			(*(vu16*)0x4808060)

#define REG_WIFI_RDMA_JUMP			(*(vu16*)0x4808062)
#define REG_WIFI_RDMA_JUMP_CNT		(*(vu16*)0x4808064)

#define REG_WIFI_824E				(*(vu16*)0x480824E)

void wifi_irqRxStart();
void wifi_irqRxEnd();
void wifi_initRx();

typedef void(*wifi_rx_done_callback_t)(void* arg);

void wifi_setRxDoneCallback(wifi_rx_done_callback_t callback, void* arg);