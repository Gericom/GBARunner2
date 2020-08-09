#pragma once

extern "C" u32 mpu_getICacheRegions();
extern "C" void mpu_setICacheRegions(u32 regions);
extern "C" void ic_invalidateAll();