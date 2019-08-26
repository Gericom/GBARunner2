#pragma once

typedef struct
{
    //0x04000120
    union
    {
        struct
        {
            u32 sioData32;
            u32 : 32;//padding
        };
        u16 sioMulti[4];
    };    
    //0x04000128
    u16 sioCnt;
    //0x0400012A
    union
    {
        u16 sioMultiSend;
        u16 sioData8;
    };
    u16 rcnt;

    u16 sioCntRead;

	u8 sioIrqFlag;

    u8 padding[3];

    u8 masterMac[6];
    u16 padding2;

    u8 slaveMac[6];
    u16 padding3;
} sio_work_t;