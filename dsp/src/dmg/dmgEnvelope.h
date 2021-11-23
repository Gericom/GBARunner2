#pragma once

typedef struct
{
    vu16 length;
    vu16 duty;
    vu16 stepTime;
    vu16 initVolume;
    vu16 curVolume;
    vu16 direction;
    vu16 dead;
    vu16 nextStep;
} dmga_envelope_t;

u16 dmga_resetEnvelope(dmga_envelope_t* env);
u16 dmga_writeEnvelope(dmga_envelope_t* env, u16 value);
void dmga_updateEnvelope(dmga_envelope_t* env);