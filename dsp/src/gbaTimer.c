#include "teak/teak.h"
#include "../../common/common_defs.s"
#include "gbaTimer.h"

extern u16 gbat_updateTimerAsm(u32* counter, u16 reload, u32 gbaTicks);

void gbat_initTimer(gbat_t* timer)
{
    timer->isStarted = 0;
    timer->curNrOverflows = 0;
    timer->reload = 0;
    timer->control = 0;
    timer->counterLo = 0;
    timer->counterHi = 0;
    timer->preScaleCounter = 0;
}

void gbat_updateTimer(gbat_t* timer)
{
    if(!(timer->control & GBAT_CONTROL_ENABLED))
    {        
        timer->isStarted = 0;
        timer->curNrOverflows = 0;
        return;
    }

    if(!timer->isStarted)
    {
        timer->counterLo = 0;
        timer->counterHi = timer->reload;
        timer->isStarted = 1;
        timer->curNrOverflows = 0;
        timer->preScaleCounter = 0;
    }
    else
    {
        if(timer->control & GBAT_CONTROL_SLAVE)
           return;//todo: implement chaining

#ifdef USE_GBA_ADJUSTED_SYNC
        u32 ticks = 512 << 16;
#else
        u32 ticks = 33594931;
#endif
        switch(timer->control & GBAT_CONTROL_PRESCALE_MASK)
        {
            case GBAT_CONTROL_PRESCALE_64:
                ticks >>= 6;
                break;
            case GBAT_CONTROL_PRESCALE_256:
                ticks >>= 8;
                break;
            case GBAT_CONTROL_PRESCALE_1024:
                ticks >>= 10;
                break;
        }
        // u16 preScaleVal = timer->control & GBAT_CONTROL_PRESCALE_MASK;
        // if(preScaleVal != 0)
        // {
        //     u16 preScale;
        //     switch(preScaleVal)
        //     {
        //         case GBAT_CONTROL_PRESCALE_64:
        //             preScale = 64;
        //             break;
        //         case GBAT_CONTROL_PRESCALE_256:
        //             preScale = 256;
        //             break;
        //         case GBAT_CONTROL_PRESCALE_1024:
        //             preScale = 1024;
        //             break;
        //     }
        //     timer->preScaleCounter++;
        //     if(timer->preScaleCounter >= preScale)
        //         timer->preScaleCounter = 0;
        //     else
        //     {
        //         timer->curNrOverflows = 0;
        //         return;//skip this update step
        //     }
        // }

        //do not process unreasonably high frequencies above ~96kHz, since they
        //should not occur and will work poorly anyway
        //if(timer->reload <= 0xFF51)
            timer->curNrOverflows = gbat_updateTimerAsm(&timer->counterHi, timer->reload, ticks);
        //else
        //    timer->curNrOverflows = 0;
        //Todo: in some cases we may want to generate the timer irq with the dsp, however
        //this may not be very precice at a rate of 47kHz, we will miss irqs as soon as 
        //more than one overflow occurs per step
        //if(timer->control & GBAA_TIMER_CONTROL_IRQ)
    }
}