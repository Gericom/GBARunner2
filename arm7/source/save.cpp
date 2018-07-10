#include <nds.h>
#include "timer.h"
#include "../../common/sd_vram.h"
#include "save.h"

extern "C" void timer1_overflow_irq()
{
	if (!vram_cd->save_work.save_enabled)
		return;
	switch (vram_cd->save_work.save_state)
	{
		case SAVE_WORK_STATE_CLEAN:
			break;
		case SAVE_WORK_STATE_DIRTY:
			vram_cd->save_work.save_state = SAVE_WORK_STATE_WAIT;
			break;
		case SAVE_WORK_STATE_WAIT:
			vram_cd->save_work.save_state = SAVE_WORK_STATE_SDSAVE;
			//invoke an irq on arm9
			*((vu32*)0x04000180) |= (1 << 13);
			break;
		case SAVE_WORK_STATE_SDSAVE:
			break;
	}		
}

void gba_save_init()
{
	vram_cd->save_work.save_enabled = FALSE;
	vram_cd->save_work.save_state = SAVE_WORK_STATE_CLEAN;
	REG_TM[1].CNT_H = 0;
	REG_TM[1].CNT_L = TIMER_FREQ(2 * 1024);
	REG_TM[1].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I | REG_TMXCNT_H_PS_1024;
	REG_IE |= (1 << 4);
}