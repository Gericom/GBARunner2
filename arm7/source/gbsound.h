#pragma once

extern "C" void gbs_frameSeqTick();

void gbs_init();
void gbs_writeReg(u8 reg, u8 val);
void gbs_setMixVolume(int mixVolume);