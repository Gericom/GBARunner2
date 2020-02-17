#pragma once

#define PMIC_REG_CONTROL        0
#define PMIC_REG_BATTERY        1
#define PMIC_REG_MICAMP_POWER   2
#define PMIC_REG_MICAMP_GAIN    3
#define PMIC_REG_USG            4
#define PMIC_REG_TWL            0x10

#define PMIC_XFER_WRITE         0
#define PMIC_XFER_READ          (1 << 7)

#define PMIC_CONTROL_LED_ON             0
#define PMIC_CONTROL_LED_BLINK_SLOW     1
#define PMIC_CONTROL_LED_BLINK_FAST     3

#define PMIC_MICAMP_GAIN_20DB   0
#define PMIC_MICAMP_GAIN_40DB   1
#define PMIC_MICAMP_GAIN_80DB   2
#define PMIC_MICAMP_GAIN_160DB  3

u8 pmic_doTransfer(u8 reg, u8 cmd);

static inline u8 pmic_readReg(u8 reg)
{
    return pmic_doTransfer(reg | PMIC_XFER_READ, 0);
}

static inline void pmic_writeReg(u8 reg, u8 val)
{
    pmic_doTransfer(reg | PMIC_XFER_WRITE, val);
}

static inline bool pmic_getSoundAmpPower()
{
    return pmic_readReg(PMIC_REG_CONTROL) & 1;
}

void pmic_setSoundAmpPower(bool powerOn);

static inline bool pmic_getLowerBacklightPower()
{
    return (pmic_readReg(PMIC_REG_CONTROL) >> 2) & 1;
}

void pmic_setLowerBacklightPower(bool powerOn);

static inline bool pmic_getUpperBacklightPower()
{
    return (pmic_readReg(PMIC_REG_CONTROL) >> 3) & 1;
}

void pmic_setUpperBacklightPower(bool powerOn);

static inline u8 pmic_getLedConfig()
{
    return (pmic_readReg(PMIC_REG_CONTROL) >> 4) & 3;
}

void pmic_setLedConfig(u8 ledConfig);

static inline void pmic_powerOff()
{
    pmic_writeReg(PMIC_REG_CONTROL, 0x40);
}

static inline bool pmic_getIsBatteryLow()
{
    return pmic_readReg(PMIC_REG_BATTERY) & 1;
}

static inline bool pmic_getMicAmpPower()
{
    return pmic_readReg(PMIC_REG_MICAMP_POWER) & 1;
}

static inline void pmic_setMicAmpPower(bool powerOn)
{
    pmic_writeReg(PMIC_REG_MICAMP_POWER, powerOn ? 1 : 0);
}

static inline u8 pmic_getMicAmpGain()
{
    return pmic_readReg(PMIC_REG_MICAMP_GAIN) & 3;
}

static inline void pmic_setMicAmpGain(u8 gain)
{
    pmic_writeReg(PMIC_REG_MICAMP_GAIN, gain & 3);
}

static inline bool pmic_getBacklightBrightness()
{
    return pmic_readReg(PMIC_REG_USG) & 3;
}

void pmic_setBacklightBrightness(u8 level);

static inline bool pmic_getForceMaxBacklightOnCharge()
{
    return (pmic_readReg(PMIC_REG_USG) >> 2) & 1;
}

void pmic_setForceMaxBacklightOnCharge(bool force);

static inline bool pmic_getIsChargerConnected()
{
    return (pmic_readReg(PMIC_REG_USG) >> 3) & 1;
}