#pragma once

void dmga_init(void);
void dmga_sample(s16* pSamp);
void dmga_writeReg(u16 reg, u16 val);
void dmga_setMixVolume(int mixVolume);