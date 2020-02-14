#include "vram.h"
#include "patchUtil.h"

u32 pcu_makeArmBranch(u32 instAddr, u32 target)
{
    return 0xEA000000 | (((target >> 2) - (instAddr >> 2) - 2) & 0xFFFFFF);
}