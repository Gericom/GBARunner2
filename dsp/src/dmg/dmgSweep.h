#pragma once

typedef struct
{
    u16 shift;
    u16 time;
    u16 step;
    u16 direction;
    u16 enabled;
    u16 occurred;
    u16 realFreq;
} dmga_sweep_t;

void dmga_resetSweep(dmga_sweep_t* sweep);
u16 dmga_writeSweep(dmga_sweep_t* sweep, u16 val);
u16 dmga_updateSweep(dmga_sweep_t* sweep, u16 initial, u16* chanFreq);