#ifndef __TIMER_H__
#define __TIMER_H__

typedef volatile struct
{
	u16 CNT_L;
	u16 CNT_H;
} timer_t;

#define REG_TM	((timer_t*)0x04000100)

#define REG_TMXCNT_H_PS(x)	(((x) & 3) << 0)

#define REG_TMXCNT_H_PS_1		0
#define REG_TMXCNT_H_PS_64		1
#define REG_TMXCNT_H_PS_256		2
#define REG_TMXCNT_H_PS_1024	3

#define REG_TMXCNT_H_CH		(1 << 2)
#define REG_TMXCNT_H_I		(1 << 6)
#define REG_TMXCNT_H_E		(1 << 7)

#define TIMER_FREQ(x)	(-33513982/(x))

#endif