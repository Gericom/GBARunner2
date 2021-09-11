#pragma once

enum TWRWramBlock
{
    TWR_WRAM_BLOCK_A = 0,
    TWR_WRAM_BLOCK_B = 1,
    TWR_WRAM_BLOCK_C = 2
};

enum TWRWramBlockImageSize
{
    TWR_WRAM_BLOCK_IMAGE_SIZE_32K = 0,
    TWR_WRAM_BLOCK_IMAGE_SIZE_64K,
    TWR_WRAM_BLOCK_IMAGE_SIZE_128K,
    TWR_WRAM_BLOCK_IMAGE_SIZE_256K,
};

#define REG_MBK1 ((vu8*)0x04004040) /* WRAM_A 0..3 */
#define REG_MBK2 ((vu8*)0x04004044) /* WRAM_B 0..3 */
#define REG_MBK3 ((vu8*)0x04004048) /* WRAM_B 4..7 */
#define REG_MBK4 ((vu8*)0x0400404C) /* WRAM_C 0..3 */
#define REG_MBK5 ((vu8*)0x04004050) /* WRAM_C 4..7 */
#define REG_MBK6 (*(vu32*)0x04004054)
#define REG_MBK7 (*(vu32*)0x04004058)
#define REG_MBK8 (*(vu32*)0x0400405C)
#define REG_MBK9 (*(vu32*)0x04004060)

#define TWR_WRAM_BASE               0x03000000

//WRAM A
#define TWR_WRAM_A_SLOT_SIZE        0x10000
#define TWR_WRAM_A_SLOT_SHIFT       16
#define TWR_WRAM_A_SLOT_COUNT       4

#define TWR_WRAM_A_ADDRESS_MAX      0x03FF0000

#define TWR_WRAM_A_SLOT_OFFSET(i)   ((i) << 2)
#define TWR_WRAM_A_SLOT_ENABLE      0x80

enum TWRWramASlotMaster
{
    TWR_WRAM_A_SLOT_MASTER_ARM9 = 0,
    TWR_WRAM_A_SLOT_MASTER_ARM7 = 1
};

#define TWR_MBK6_START_ADDR_MASK    0x00000FF0
#define TWR_MBK6_START_ADDR_SHIFT   4

#define TWR_MBK6_IMAGE_SIZE_SHIFT   12

#define TWR_MBK6_END_ADDR_SHIFT     20

//WRAM B
#define TWR_WRAM_BC_SLOT_SIZE       0x8000
#define TWR_WRAM_BC_SLOT_SHIFT      15
#define TWR_WRAM_BC_SLOT_COUNT      8

#define TWR_WRAM_BC_ADDRESS_MAX     0x03FF8000

#define TWR_WRAM_BC_SLOT_OFFSET(i)   ((i) << 2)
#define TWR_WRAM_BC_SLOT_ENABLE      0x80

enum TWRWramBSlotMaster
{
    TWR_WRAM_B_SLOT_MASTER_ARM9 = 0,
    TWR_WRAM_B_SLOT_MASTER_ARM7 = 1,
    TWR_WRAM_B_SLOT_MASTER_DSP_CODE = 2
};

enum TWRWramCSlotMaster
{
    TWR_WRAM_C_SLOT_MASTER_ARM9 = 0,
    TWR_WRAM_C_SLOT_MASTER_ARM7 = 1,
    TWR_WRAM_C_SLOT_MASTER_DSP_DATA = 2
};

#define TWR_MBK7_START_ADDR_MASK    0x00000FF8
#define TWR_MBK7_START_ADDR_SHIFT   3

#define TWR_MBK7_IMAGE_SIZE_SHIFT   12

#define TWR_MBK7_END_ADDR_SHIFT     19

#define TWR_MBK8_START_ADDR_MASK    0x00000FF8
#define TWR_MBK8_START_ADDR_SHIFT   3

#define TWR_MBK8_IMAGE_SIZE_SHIFT   12

#define TWR_MBK8_END_ADDR_SHIFT     19

u32 twr_getBlockAddress(TWRWramBlock block);
void twr_setBlockMapping(TWRWramBlock block, u32 start, u32 length, TWRWramBlockImageSize imageSize);
void twr_mapWramASlot(int slot, TWRWramASlotMaster master, int offset, bool enable);
void twr_mapWramBSlot(int slot, TWRWramBSlotMaster master, int offset, bool enable);
void twr_mapWramCSlot(int slot, TWRWramCSlotMaster master, int offset, bool enable);