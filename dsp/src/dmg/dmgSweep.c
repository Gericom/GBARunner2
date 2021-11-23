#include "../teak/teak.h"
#include "dmgSweep.h"

void dmga_resetSweep(dmga_sweep_t* sweep)
{
    sweep->step = sweep->time;
    sweep->enabled = sweep->step != 8 || sweep->shift;
    sweep->occurred = 0;
}

u16 dmga_writeSweep(dmga_sweep_t* sweep, u16 val)
{
    sweep->shift = val & 7;
    u16 oldDir = sweep->direction;
    sweep->direction = (val >> 3) & 1;
    u16 on = 1;
    if(sweep->occurred && oldDir && !sweep->direction)
        on = 0;
    sweep->occurred = 0;
    sweep->time = (val >> 4) & 7;
    if(!sweep->time)
        sweep->time = 8;
    return on;
}

u16 dmga_updateSweep(dmga_sweep_t* sweep, u16 initial, u16* chanFreq)
{
    if(initial || sweep->time != 8)
    {
        u16 freq = sweep->realFreq;
        if(sweep->direction)
        {
            freq -= freq >> sweep->shift;
            if(!initial && freq >= 0)
            {
                sweep->realFreq = freq;
                *chanFreq = freq;
            }
        }
        else
        {
            freq += freq >> sweep->shift;
            if(freq < 2048)
            {
                if(!initial && sweep->shift)
                {
                    sweep->realFreq = freq;
                    *chanFreq = freq;
                    if(!dmga_updateSweep(sweep, 1, chanFreq))
                        return 0;
                }
            }
            else
                return 0;
        }
        sweep->occurred = 1;
    }
    sweep->step = sweep->time;
    return 1;
}