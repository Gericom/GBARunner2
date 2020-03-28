#include "../teak/teak.h"
#include "dmgEnvelope.h"

static void updateDead(dmga_envelope_t* env)
{
    if(!env->stepTime)
        env->dead = env->curVolume ? 1 : 2;
    else if(!env->direction && !env->curVolume)
        env->dead = 2;
    else if(env->direction && env->curVolume == 0xF)
        env->dead = 1;
    else
        env->dead = 0;
}

u16 dmga_resetEnvelope(dmga_envelope_t* env)
{
    env->curVolume = env->initVolume;
    updateDead(env);
    if(!env->dead)
        env->nextStep = env->stepTime;
    return env->initVolume || env->direction;
}

u16 dmga_writeEnvelope(dmga_envelope_t* env, u16 value)
{
    env->stepTime = value & 7;
    env->direction = (value >> 3) & 1;
    env->initVolume = (value >> 4) & 0xF;
    updateDead(env);
    return (env->initVolume || env->direction) && env->dead != 2;
}

void dmga_updateEnvelope(dmga_envelope_t* env)
{
    if(env->dead)
        return;
    if(--env->nextStep != 0)
        return;
    s16 volume = env->curVolume;
    if(env->direction)
        volume++;
    else
        volume--;
    if(volume >= 15)
    {
        env->curVolume = 15;
        env->dead = 1;
    }
    else if(volume <= 0)
    {
        env->curVolume = 0;
        env->dead = 2;
    }
    else
    {
        env->curVolume = volume;
        env->nextStep = env->stepTime;
    }
}