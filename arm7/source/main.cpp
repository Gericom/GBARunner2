#include <nds.h>
#include "timer.h"
#include "sound.h"
#include "fifo.h"

//#define SOUND_CHANNEL_0_SETTINGS	(REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) | REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) | REG_SOUNDXCNT_PAN(64) | REG_SOUNDXCNT_VOLUME(0x7F))	//0x0840007F;

//#define CENTER_AND_MASK_ENABLED

//static void timer_handler()
//{
	//if(*((vu16*)0x04000006) == 160)
		//*((vu32*)0x04000180) = 1 << 13;
		//*((vu16*)0x04000006) = 202;
//}

/*static void fifo_handler()
{
	//irqDisable(IRQ_FIFO_NOT_EMPTY);
	//REG_IME = 0;
	//u32 cmd = REG_RECV_FIFO;
	//REG_IME = 0;
	while(!(*((vu32*)0x04000184) & (1 << 8)))
	{
		u32 cmd = REG_RECV_FIFO;
		vu32 val;
		switch(cmd)
		{
			case 0xAA5500C4://fifo_start_sound_command
				//REG_SOUND[0].CNT |= REG_SOUNDXCNT_E;
				gba_sound_notify_reset();
				break;
			case 0x04000100:
			case 0x04000102:
				{
					while(*((vu32*)0x04000184) & (1 << 8));
					val = REG_RECV_FIFO;
					gba_sound_timer_updated(val & 0xFFFF);
					break;
				}
			case 0x04000104:
			case 0x04000106:
			case 0x04000108:
			case 0x0400010A:
			case 0x0400010C:
			case 0x0400010E:
				{
					while(*((vu32*)0x04000184) & (1 << 8));
					val = REG_RECV_FIFO;
					break;
				}
			//case 0xC5://fifo_stop_sound_command
				//REG_SOUND[0].CNT = SOUND_CHANNEL_0_SETTINGS;
			//	break;
		}
	}
	//REG_IME = 1;
}

void vblank_irq_handler()
{

}*/
/*int frameOffset = 0;
static void hblank_handler()
{
	if(!frameOffset && *((vu16*)0x04000006) == 15)
	{
		*((vu16*)0x04000006) = 0;
		frameOffset = 1;
	}
	else if(frameOffset && *((vu16*)0x04000006) == 159)
	{
		*((vu16*)0x04000006) = 175;
		frameOffset = 0;
	}
}*/

static void vblank_handler()
{

}

extern "C" void my_irq_handler();

int main()
{
	//dmaFillWords(0, (void*)0x04000400, 0x100);

	//REG_SOUNDCNT |= SOUND_ENABLE;
	//writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
	//powerOn(POWER_SOUND);

	REG_IME = 0;
	REG_IE = 0;
	REG_IF = ~0;

	*((vu32*)0x0380FFFC) = (vu32)&my_irq_handler;

	REG_IME = 1;

	//irqInit();

	//REG_IE = 0;
	//REG_IF = ~0;

	//irqSet(IRQ_HBLANK, hblank_handler);
	//irqEnable(IRQ_HBLANK);
	//irqSet(IRQ_VBLANK, vblank_handler);
	//irqEnable(IRQ_VBLANK);

	//enable the arm7-arm9 fifo
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;
	//wait for the arm9 sync command
	do
	{
		while(*((vu32*)0x04000184) & (1 << 8));
	} while(REG_RECV_FIFO != 0xAA5555AA);

	//int oldirq = enterCriticalSection();
	//irqSet(IRQ_HBLANK, hblank_handler);

	gba_sound_init();


	//irqSet(IRQ_VBLANK, vblank_irq_handler);
    //irqEnable(IRQ_VBLANK);

	//irqSet(IRQ_FIFO_NOT_EMPTY, fifo_handler);
	//irqEnable(IRQ_FIFO_NOT_EMPTY);
	//REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;

	//leaveCriticalSection(oldirq);

	//send done command to arm9
	REG_SEND_FIFO = 0x55AAAA55;

#ifdef CENTER_AND_MASK_ENABLED
	REG_TM[2].CNT_H = 0;
	REG_TM[3].CNT_H = 0;
	REG_TM[2].CNT_L = -2130;//TIMER_FREQ(sampleFreq);
	REG_TM[3].CNT_L = 0;
	REG_TM[3].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_CH;
	while(*((vu16*)0x04000006) == 0);
	while(*((vu16*)0x04000006) != 0);
	REG_TM[2].CNT_H = REG_TMXCNT_H_E;
#endif

	//REG_TM[2].CNT_H = 0;
	//irqDisable(IRQ_TIMER2);
	//irqSet(IRQ_TIMER2, timer_handler);
	//REG_TM[2].CNT_L = TIMER_FREQ(60)/64;//4 * 15734);
	////REG_TM[2].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I | REG_TMXCNT_H_PS_64;
	//irqEnable(IRQ_TIMER2);
	
	//REG_SOUND[0].SAD = 0x23F8000;
	//REG_SOUND[0].TMR = (u16)-1594; //-1253
	//REG_SOUND[0].PNT = 0;
	//REG_SOUND[0].LEN = 396 * 10;
	//REG_SOUND[0].CNT = SOUND_CHANNEL_0_SETTINGS;
	//fifo loop
	//vu32 val;
	//int frameOffset = 0;
	int frameState = 0;
	int realLine = 0;
	while(1)
	{
		//swiWaitForVBlank();
		//while(*((vu16*)0x04000006) < 160);
		//*((vu32*)0x04000180) = 1 << 13;
		//while(*((vu16*)0x04000006) >= 160);
		//if(!(*((vu32*)0x04000136) & 1))
		//	*((vu32*)0x04000180) = 1 << 13;
		//oldirq = enterCriticalSection();
		/*if(*((vu16*)0x04000006) >= 160 && *((vu16*)0x04000006) < 192)
		{
			*((vu32*)0x04000180) = 1 << 13;
			while(*((vu16*)0x04000006) < 192);
		}*/
		//leaveCriticalSection(oldirq);
		/*while(*((vu32*)0x04000184) & (1 << 8))
		{
			//if(*((vu16*)0x04000006) == 160)
			//	*((vu16*)0x04000006) = 192;
			if(*((vu16*)0x04000006) == 160)
			{
				*((vu32*)0x04000180) = 1 << 13;
				while(*((vu16*)0x04000006) == 160);
			}
		}*/
#ifdef CENTER_AND_MASK_ENABLED
		while(*((vu32*)0x04000184) & (1 << 8))
		{
			if(*((vu16*)0x04000004) & 2)
			{
				int line = REG_TM[3].CNT_L;
				if(line == 263)
				{
					REG_TM[3].CNT_L = REG_TM[3].CNT_L % 263;
					REG_TM[3].CNT_H = 0;
					REG_TM[3].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_CH;
				}
				line = line % 263;
				//Line 262 seems to reset the lcd so it starts drawing from the top again. Thus, it's important to have line 262 executed at it's physical place
				//This has a neasty side effect when centering; affine bgs are offset in that case :(
				if(line == 262 || line < 15)
					*((vu16*)0x04000006) = 192;
				else if(line == 15)
					*((vu16*)0x04000006) = 0;
				else if(line >= 16 + 159 && line < 190)
					*((vu16*)0x04000006) = 192;
				else if(line == 190)
					*((vu16*)0x04000006) = 191;
				else
					continue;
				while(*((vu16*)0x04000004) & 2);
			}
		}
#else
		while(*((vu32*)0x04000184) & (1 << 8));
		{
			if(!(*((vu32*)0x04000136) & 1))
				gba_sound_resync();
		}
		//{
		//	if(!(*((vu32*)0x04000136) & 1))
		//		*((vu32*)0x04000180) |= (1 << 13);
		//}
#endif
		//{
			/*if(frameState == 0 && *((vu16*)0x04000006) == 0)//*((vu16*)0x04000006) == 15)
			{
				*((vu16*)0x04000006) = 262;
				frameState = 1;
			}
			else if(frameState == 1 && *((vu16*)0x04000006) == 0)//*((vu16*)0x04000006) == 15)
			{
				*((vu16*)0x04000006) = 2;//262;
				frameState = 2;
			}
			else if(frameState == 2 && *((vu16*)0x04000006) == 15)
			{
				*((vu16*)0x04000006) = 0;
				frameState = 3;
			}
			else if(frameState == 3 && *((vu16*)0x04000006) == 159)
			{
				*((vu16*)0x04000006) = 159 + 16;
				frameState = 0;
			}*/

			/*if(frameOffset == 0 && *((vu16*)0x04000006) == 15)
			{
				*((vu16*)0x04000006) = 0;
				frameOffset = 1;
			}
			else if(frameOffset && *((vu16*)0x04000006) == 159)
			{
				*((vu16*)0x04000006) = 175;
				frameOffset = 0;
			}*/
		//}
		/*{
			if(!frameOffset && *((vu16*)0x04000006) == 15)
			{
				*((vu16*)0x04000006) = 0;
				frameOffset = 1;
			}
			else if(frameOffset && *((vu16*)0x04000006) == 159)//176)
			{
				*((vu16*)0x04000006) = 175;//0;
				frameOffset = 0;
			}
			/*if(*((vu16*)0x04000006) == 160)
			{
				*((vu16*)0x04000006) = 190;
				while(*((vu16*)0x04000006) == 160);
			}*/
		//}
		u32 cmd = REG_RECV_FIFO;
		//if((cmd >> 16) != 0xAA55)
		//	continue;
		uint32_t vals[4];
		u32 val;
		switch(cmd)
		{
		case 0xAA5500C4://fifo_start_sound_command
			//REG_SOUND[0].CNT |= REG_SOUNDXCNT_E;
			gba_sound_notify_reset();
			break;
		case 0xAA5500F8:
			while(*((vu32*)0x04000184) & (1 << 8));
			val = REG_RECV_FIFO;
			gba_sound_set_src(val);
			break;
		case 0xAA5500F9:
			while(*((vu32*)0x04000184) & (1 << 8));
				vals[0] = REG_RECV_FIFO;
			while(*((vu32*)0x04000184) & (1 << 8));
				vals[1] = REG_RECV_FIFO;
			while(*((vu32*)0x04000184) & (1 << 8));
				vals[2] = REG_RECV_FIFO;
			while(*((vu32*)0x04000184) & (1 << 8));
				vals[3] = REG_RECV_FIFO;
			/*while(*((vu32*)0x04000184) & (1 << 8));
				vals[4] = REG_RECV_FIFO;
			while(*((vu32*)0x04000184) & (1 << 8));
				vals[5] = REG_RECV_FIFO;
			while(*((vu32*)0x04000184) & (1 << 8));
				vals[6] = REG_RECV_FIFO;
			while(*((vu32*)0x04000184) & (1 << 8));
				vals[7] = REG_RECV_FIFO;*/
			gba_sound_fifo_write16((uint8_t*)&vals[0]);
			break;
		case 0x040000A0:
			while(*((vu32*)0x04000184) & (1 << 8));
			val = REG_RECV_FIFO;
			gba_sound_fifo_write(val);
			break;
		case 0x04000100:
		case 0x04000102:
			{
				while(*((vu32*)0x04000184) & (1 << 8));
				val = REG_RECV_FIFO;
				gba_sound_timer_updated(val & 0xFFFF);
				break;
			}
		case 0x04000104:
		case 0x04000106:
		case 0x04000108:
		case 0x0400010A:
		case 0x0400010C:
		case 0x0400010E:
			{
				while(*((vu32*)0x04000184) & (1 << 8));
				val = REG_RECV_FIFO;
				break;
			}
		//case 0xC5://fifo_stop_sound_command
			//REG_SOUND[0].CNT = SOUND_CHANNEL_0_SETTINGS;
		//	break;
		}
	}
	return 0;
}