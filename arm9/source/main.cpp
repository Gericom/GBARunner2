#include <nds.h>
extern "C" void gba_setup();

uint8_t bios_tmp[16 * 1024];

int main()
{
	REG_IME = 0;
	BG_PALETTE[0] = RGB15(31,0,0);
	REG_DISPCNT = MODE_0_2D;
	//Let's try out if we can redirect the swi calls to our own handler!
	gba_setup();
	while(1);
	return 0;
}