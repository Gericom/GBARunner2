#pragma once
#include "wifi_common.h"

struct wifi_flashdata_t
{
    u16 crc;
    u16 length;
    u8 unk2E;
    u8 wifiVersion;
    u8 unk30[6];
    wifi_macaddr_t macAddress;
    u16 channelMask;
    u16 unk3E;
    u8 rfType;
    u8 rfBitsPerEntry;
    u8 rfEntryCount;
    u8 unk43;
    u16 wifi8146;
    u16 wifi8148;
	u16 wifi814A;
	u16 wifi814C;
	u16 wifi8120;
	u16 wifi8122;
	u16 wifi8154;
	u16 wifi8144;
	u16 wifi8132_1;
	u16 wifi8132_2;
	u16 wifiTxTimestampOffs;
	u16 wifi8142;
	u16 wifiPowersave;
	u16 wifi8124;
	u16 wifi8128;
	u16 wifi8150;
    u8 bbConfig[0x69];
    u8 unkCD;
    u8 rest[0x132];
};

void wifi_initFlashData();