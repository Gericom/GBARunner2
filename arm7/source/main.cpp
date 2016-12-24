#include <nds.h>
#include "sound.h"

#define SOUND_CHANNEL_0_SETTINGS	(REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) | REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) | REG_SOUNDXCNT_PAN(64) | REG_SOUNDXCNT_VOLUME(0x7F))	//0x0840007F;

#define REG_SEND_FIFO	(*((vu32*)0x04000188))
#define REG_RECV_FIFO	(*((vu32*)0x04100000))

int main()
{
	irqInit();
	//enable the arm7-arm9 fifo
	*((vu32*)0x04000184) = 0x8000;
	//wait for the arm9 sync command
	do
	{
		while(*((vu32*)0x04000184) & (1 << 8));
	} while(REG_RECV_FIFO != 0xAA5555AA);
	//send done command to arm9
	REG_SEND_FIFO = 0x55AAAA55;

	gba_sound_init();
	
	//REG_SOUND[0].SAD = 0x23F8000;
	//REG_SOUND[0].TMR = (u16)-1594; //-1253
	//REG_SOUND[0].PNT = 0;
	//REG_SOUND[0].LEN = 396 * 10;
	//REG_SOUND[0].CNT = SOUND_CHANNEL_0_SETTINGS;
	//fifo loop
	vu32 val;
	while(1)
	{
		while(*((vu32*)0x04000184) & (1 << 8));
		u32 cmd = REG_RECV_FIFO;
		//if((cmd >> 16) != 0xAA55)
		//	continue;
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
	return 0;
}