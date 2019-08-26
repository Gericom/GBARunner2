#include <nds.h>
#include <string.h>
#include "../../../common/sd_vram.h"
#include "../wifi/wifi.h"
#include "../wifi/wifi_tx.h"
#include "sio_multi.h"
#include "sio.h"

sio_arm7_work_t gSioWork;

void sio_updateMode(u16 cnt, u16 rcnt)
{
    int mode;
    if(rcnt & 0x8000)
    {
        if(rcnt & 0x4000)
            mode = SIO_MODE_JOY;
        else
            mode = SIO_MODE_GPIO;
    }
    else
    {
        switch((cnt >> 12) & 3)
        {
            case 0:
                mode = SIO_MODE_NORMAL;
                break;
            case 1:
                mode = SIO_MODE_NORMAL;
                break;
            case 2:
                mode = SIO_MODE_MULTI;
                break;
            case 3:
                mode = SIO_MODE_UART;
                break;
        }
    }
    if(mode != gSioWork.mode)
    {
        gSioWork.mode = mode;
        //mode switch
        switch(mode)
        {
            case SIO_MODE_GPIO:
            case SIO_MODE_NORMAL:
            case SIO_MODE_JOY:
				REG_WIFI_RXFILTER = 0;
				wifi_setTxDoneCallback(NULL, NULL);
                break;
            case SIO_MODE_MULTI:
                sio_multiStart();
                break;
        }
    }
}

static void writeCntReg(u16 val)
{
    sio_updateMode(val, vram_cd->sioWork.rcnt);
    switch(gSioWork.mode)
    {
        case SIO_MODE_GPIO:
        case SIO_MODE_NORMAL:
        case SIO_MODE_JOY:
            break;
        case SIO_MODE_MULTI:
            sio_multiCntWrite(val);
            break;
    }
}

static void writeRcntReg(u16 val)
{
    sio_updateMode(vram_cd->sioWork.sioCnt, val);
}

void sio_writeReg16(u16 addr, u16 val)
{
    if(addr == 0x128)
        writeCntReg(val);
    else if(addr == 0x134)
        writeRcntReg(val);
    else if(gSioWork.mode == SIO_MODE_MULTI && addr == 0x12A)
        sio_multiSendWrite(val);
}

void sio_init()
{
    memset(&gSioWork, 0, sizeof(gSioWork));
	if (WIFI_RAM->firmData.wifiData.macAddress.address[0] == vram_cd->sioWork.slaveMac[0] /*0x00*/ &&
		WIFI_RAM->firmData.wifiData.macAddress.address[1] == vram_cd->sioWork.slaveMac[1] /*0x25*/ &&
		WIFI_RAM->firmData.wifiData.macAddress.address[2] == vram_cd->sioWork.slaveMac[2] /*0xA0*/ &&
		WIFI_RAM->firmData.wifiData.macAddress.address[3] == vram_cd->sioWork.slaveMac[3] /*0x0C*/ &&
		WIFI_RAM->firmData.wifiData.macAddress.address[4] == vram_cd->sioWork.slaveMac[4] /*0x74*/ &&
		WIFI_RAM->firmData.wifiData.macAddress.address[5] == vram_cd->sioWork.slaveMac[5] /*0x3C*/)
	{
		gSioWork.id = SIO_ID_SLAVE_0;
		gSioWork.multiMacs[0].address[0] = vram_cd->sioWork.masterMac[0] /*0xE0*/;
		gSioWork.multiMacs[0].address[1] = vram_cd->sioWork.masterMac[1] /*0xE7*/;
		gSioWork.multiMacs[0].address[2] = vram_cd->sioWork.masterMac[2] /*0x51*/;
		gSioWork.multiMacs[0].address[3] = vram_cd->sioWork.masterMac[3] /*0x6E*/;
		gSioWork.multiMacs[0].address[4] = vram_cd->sioWork.masterMac[4] /*0xB1*/;
		gSioWork.multiMacs[0].address[5] = vram_cd->sioWork.masterMac[5] /*0x92*/;
		gSioWork.multiMacs[1] = WIFI_RAM->firmData.wifiData.macAddress;

		REG_WIFI_BSSID.address16[0] = 0;// gSioWork.multiMacs[0].address16[0];
		REG_WIFI_BSSID.address16[1] = 0;// gSioWork.multiMacs[0].address16[1];
		REG_WIFI_BSSID.address16[2] = 0;// gSioWork.multiMacs[0].address16[2];
	}
	else
	{
		gSioWork.id = SIO_ID_MASTER;
		gSioWork.multiMacs[0] = WIFI_RAM->firmData.wifiData.macAddress;
		gSioWork.multiMacs[1].address[0] = vram_cd->sioWork.slaveMac[0] /*0x00*/;
		gSioWork.multiMacs[1].address[1] = vram_cd->sioWork.slaveMac[1] /*0x25*/;
		gSioWork.multiMacs[1].address[2] = vram_cd->sioWork.slaveMac[2] /*0xA0*/;
		gSioWork.multiMacs[1].address[3] = vram_cd->sioWork.slaveMac[3] /*0x0C*/;
		gSioWork.multiMacs[1].address[4] = vram_cd->sioWork.slaveMac[4] /*0x74*/;
		gSioWork.multiMacs[1].address[5] = vram_cd->sioWork.slaveMac[5] /*0x3C*/;

		REG_WIFI_BSSID.address16[0] = 0;// gSioWork.multiMacs[1].address16[0];
		REG_WIFI_BSSID.address16[1] = 0;// gSioWork.multiMacs[1].address16[1];
		REG_WIFI_BSSID.address16[2] = 0;// gSioWork.multiMacs[1].address16[2];
	}
	
}