#pragma once

#define GBAT_CONTROL_PRESCALE_1       0
#define GBAT_CONTROL_PRESCALE_64      1
#define GBAT_CONTROL_PRESCALE_256     2
#define GBAT_CONTROL_PRESCALE_1024    3

#define GBAT_CONTROL_PRESCALE(x)      (((x) & 3) << 0)
#define GBAT_CONTROL_PRESCALE_MASK    GBAT_CONTROL_PRESCALE(3)

#define GBAT_CONTROL_SLAVE            (1 << 2)
#define GBAT_CONTROL_IRQ              (1 << 6)
#define GBAT_CONTROL_ENABLED          (1 << 7)

typedef struct
{
    vu16 isStarted;
    vu16 curNrOverflows; //number of overflows in the current 47kHz step

    vu16 reload;
    vu16 control;

    vu16 counterHi;
    vu16 counterLo;

    vu16 preScaleCounter;
} gbat_t;

void gbat_initTimer(gbat_t* timer);
void gbat_updateTimer(gbat_t* timer);