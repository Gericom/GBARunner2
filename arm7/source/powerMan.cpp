#include <nds.h>
#include "spi.h"
#include "irq.h"
#include "powerMan.h"

u8 pmic_doTransfer(u8 reg, u8 val)
{
    u8 result;
	int oldIrq = irq_masterDisable();
    {
        spi_waitBusy();
        REG_SPI_CNT = SPI_CNT_BUS_ENABLE | SPI_CNT_RATE_1M | SPI_CNT_WIDTH_8BIT | SPI_CNT_CS_HOLD | SPI_CNT_DEV_POWER;
        spi_transferByte(reg);
        REG_SPI_CNT = SPI_CNT_BUS_ENABLE | SPI_CNT_RATE_1M | SPI_CNT_WIDTH_8BIT | SPI_CNT_DEV_POWER;
        result = spi_transferByte(val);
	}
    irq_masterRestore(oldIrq);
	return result;
}

void pmic_setSoundAmpPower(bool powerOn)
{
    int oldIrq = irq_masterDisable();
    {
        u8 val = pmic_readReg(PMIC_REG_CONTROL);
        val = (val & ~1) | (powerOn ? 1 : 0);
        pmic_writeReg(PMIC_REG_CONTROL, val);
    }
    irq_masterRestore(oldIrq); 
}

void pmic_setLowerBacklightPower(bool powerOn)
{
    int oldIrq = irq_masterDisable();
    {
        u8 val = pmic_readReg(PMIC_REG_CONTROL);
        val = (val & ~4) | (powerOn ? 4 : 0);
        pmic_writeReg(PMIC_REG_CONTROL, val);
    }
    irq_masterRestore(oldIrq);     
}

void pmic_setUpperBacklightPower(bool powerOn)
{
    int oldIrq = irq_masterDisable();
    {
        u8 val = pmic_readReg(PMIC_REG_CONTROL);
        val = (val & ~8) | (powerOn ? 8 : 0);
        pmic_writeReg(PMIC_REG_CONTROL, val);
    }
    irq_masterRestore(oldIrq);     
}

void pmic_setLedConfig(u8 ledConfig)
{
    int oldIrq = irq_masterDisable();
    {
        u8 val = pmic_readReg(PMIC_REG_CONTROL);
        val = (val & ~0x30) | ((ledConfig & 3) << 4);
        pmic_writeReg(PMIC_REG_CONTROL, val);
    }
    irq_masterRestore(oldIrq);   
}

void pmic_setBacklightBrightness(u8 level)
{
    int oldIrq = irq_masterDisable();
    {
        u8 val = pmic_readReg(PMIC_REG_USG);
        val = (val & ~3) | (level & 3);
        pmic_writeReg(PMIC_REG_USG, val);
    }
    irq_masterRestore(oldIrq);
}

void pmic_setForceMaxBacklightOnCharge(bool force)
{
    int oldIrq = irq_masterDisable();
    {
        u8 val = pmic_readReg(PMIC_REG_USG);
        val = (val & ~4) | (force ? 4 : 0);
        pmic_writeReg(PMIC_REG_USG, val);
    }
    irq_masterRestore(oldIrq);     
}