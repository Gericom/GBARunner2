#pragma once

typedef signed short s16;
typedef signed int s32;

typedef unsigned short u16;
typedef unsigned int u32;

typedef volatile s16 vs16;
typedef volatile s32 vs32;

typedef volatile u16 vu16;
typedef volatile u32 vu32;

#define FALSE   0
#define TRUE    1

#include "ahbm.h"
#include "apbp.h"
#include "btdmp.h"
#include "dma.h"
#include "icu.h"
#include "timer.h"
#include "cpu.h"