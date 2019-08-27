#include <nds.h>
#include "wifi.h"
#include "wifi_rx.h"

static wifi_rx_done_callback_t sRxDoneCallback;
static void* sRxDoneCallbackArg;

void wifi_irqRxStart()
{
	REG_WIFI_ISR = WIFI_IRQ_RX_START;
}

void wifi_irqRxEnd()
{
	REG_WIFI_ISR = WIFI_IRQ_RX_END;
	if (sRxDoneCallback)
		sRxDoneCallback(sRxDoneCallbackArg);
}

void wifi_initRx()
{
	//setup rx buffer
	REG_WIFI_MDP_CONFIG = 0x8000;
	REG_WIFI_RXBUF_STR = 0x4000 + WIFI_RAM_RX_BUF_OFFSET;
	REG_WIFI_RXBUF_WCUR = WIFI_RAM_RX_BUF_OFFSET >> 1;
	REG_WIFI_RXBUF_END = 0x5F60;
	REG_WIFI_RXBUF_BNR = WIFI_RAM_RX_BUF_OFFSET >> 1;
	REG_WIFI_RDMA_JUMP = 0x5F5E;
	REG_WIFI_MDP_CONFIG = 0x8001;
	REG_WIFI_824E = 0xFFFF;
	WIFI_RAM->unk5F60[8] = 0xFFFF;
	WIFI_RAM->unk5F60[9] = 0xFFFF;
	WIFI_RAM->unk5F60[15] = 0xFFFF;
	WIFI_RAM->unk5F60[11] = 0xFFFF;
}

void wifi_setRxDoneCallback(wifi_rx_done_callback_t callback, void* arg)
{
	sRxDoneCallback = callback;
	sRxDoneCallbackArg = arg;
}
