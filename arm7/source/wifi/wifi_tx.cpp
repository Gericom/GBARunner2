#include <nds.h>
#include "wifi.h"
#include "wifi_tx.h"

static wifi_tx_done_callback_t sTxDoneCallback;
static void* sTxDoneCallbackArg;
void wifi_irqTxStart()
{
	REG_WIFI_IF = WIFI_IRQ_TX_START;
	//hmm?
	/*if(REG_WIFI_CHIPID != WIFI_CHIPID_DS_PHAT && 
		(REG_WIFI_RF_PINS & (WIFI_RF_PIN_TXMAIN | WIFI_RF_PIN_TXON)) == (WIFI_RF_PIN_TXMAIN | WIFI_RF_PIN_TXON) &&
		REG_WIFI_82B8)
	{
		int val = REG_WIFI_82B8;
		int i = 0;
		while(val == REG_WIFI_82B8)
		{
			if(i++ == 1000)
			{
				//fatal error?
				break;
			}
		}
	}*/
}

void wifi_irqTxEnd()
{
	REG_WIFI_IF = WIFI_IRQ_TX_END;
	if(sTxDoneCallback)
		sTxDoneCallback(sTxDoneCallbackArg);
}


void wifi_initTx()
{
	sTxDoneCallback = 0;
	REG_WIFI_TXREQ_EN_SET = WIFI_TXREQ_LOC_ALL;
}

void wifi_setTxDoneCallback(wifi_tx_done_callback_t callback, void* arg)
{
	sTxDoneCallback = callback;
	sTxDoneCallbackArg = arg;
}