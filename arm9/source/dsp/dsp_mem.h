#pragma once

#define DSP_MEM_ADDR_TO_DSP(addr)       ((u16)(((u32)(addr)) >> 1))
#define DSP_MEM_ADDR_TO_CPU(addr)       ((u32)((addr) << 1))

#define DSP_MEM_32BIT_TO_DSP(x)         (u32)(((u32)(x) >> 16) | ((u32)(x) << 16))