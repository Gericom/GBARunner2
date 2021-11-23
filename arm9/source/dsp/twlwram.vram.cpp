#include "vram.h"
#include "twlwram.h"

u32 twr_getBlockAddress(TWRWramBlock block)
{
    switch(block)
    {
        case TWR_WRAM_BLOCK_A:
            return TWR_WRAM_BASE + (((REG_MBK6 & TWR_MBK6_START_ADDR_MASK) >> TWR_MBK6_START_ADDR_SHIFT) << TWR_WRAM_A_SLOT_SHIFT);
        case TWR_WRAM_BLOCK_B:
            return TWR_WRAM_BASE + (((REG_MBK7 & TWR_MBK7_START_ADDR_MASK) >> TWR_MBK7_START_ADDR_SHIFT) << TWR_WRAM_BC_SLOT_SHIFT);
        case TWR_WRAM_BLOCK_C:
            return TWR_WRAM_BASE + (((REG_MBK8 & TWR_MBK8_START_ADDR_MASK) >> TWR_MBK8_START_ADDR_SHIFT) << TWR_WRAM_BC_SLOT_SHIFT);
    }
    return 0;
}

void twr_setBlockMapping(TWRWramBlock block, u32 start, u32 length, TWRWramBlockImageSize imageSize)
{
    start -= TWR_WRAM_BASE;
    u32 end;
    switch(block)
    {
        case TWR_WRAM_BLOCK_A:        
            start >>= TWR_WRAM_A_SLOT_SHIFT;
            length >>= TWR_WRAM_A_SLOT_SHIFT;
            end = start + length;
            REG_MBK6 = (start << TWR_MBK6_START_ADDR_SHIFT) | (imageSize << TWR_MBK6_IMAGE_SIZE_SHIFT) | (end << TWR_MBK6_END_ADDR_SHIFT);
            break;
        case TWR_WRAM_BLOCK_B:
            start >>= TWR_WRAM_BC_SLOT_SHIFT;
            length >>= TWR_WRAM_BC_SLOT_SHIFT;
            end = start + length;
            REG_MBK7 = (start << TWR_MBK7_START_ADDR_SHIFT) | (imageSize << TWR_MBK7_IMAGE_SIZE_SHIFT) | (end << TWR_MBK7_END_ADDR_SHIFT);
            break;
        case TWR_WRAM_BLOCK_C:
            start >>= TWR_WRAM_BC_SLOT_SHIFT;
            length >>= TWR_WRAM_BC_SLOT_SHIFT;
            end = start + length;
            REG_MBK8 = (start << TWR_MBK8_START_ADDR_SHIFT) | (imageSize << TWR_MBK8_IMAGE_SIZE_SHIFT) | (end << TWR_MBK8_END_ADDR_SHIFT);
            break;
    }
}

void twr_mapWramASlot(int slot, TWRWramASlotMaster master, int offset, bool enable)
{
    if(slot < 0 || slot > 3 || offset < 0 || offset > 3)
        return;
    REG_MBK1[slot] = enable ? (TWR_WRAM_A_SLOT_ENABLE | master | TWR_WRAM_A_SLOT_OFFSET(offset)) : 0;
}

void twr_mapWramBSlot(int slot, TWRWramBSlotMaster master, int offset, bool enable)
{
    if(slot < 0 || slot > 7 || offset < 0 || offset > 7)
        return;
    REG_MBK2[slot] = enable ? (TWR_WRAM_BC_SLOT_ENABLE | master | TWR_WRAM_BC_SLOT_OFFSET(offset)) : 0;
}

void twr_mapWramCSlot(int slot, TWRWramCSlotMaster master, int offset, bool enable)
{
    if(slot < 0 || slot > 7 || offset < 0 || offset > 7)
        return;
    REG_MBK4[slot] = enable ? (TWR_WRAM_BC_SLOT_ENABLE | master | TWR_WRAM_BC_SLOT_OFFSET(offset)) : 0;
}