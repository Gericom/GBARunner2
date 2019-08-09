#include <nds.h>
extern "C" void initSystem()
{
	//---------------------------------------------------------------------------------
	register int i;
	// stop timers and dma
	for (i = 0; i<4; i++)
	{
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}

	REG_SCFG_EXT = 0x83000000;	// Disable access to DSi hardware
	REG_SCFG_CLK = 0x80;		// Set NTR clock speed


	// clear video display registers
	dmaFillWords(0, (void*)0x04000000, 0x56);
	dmaFillWords(0, (void*)0x04001008, 0x56);

	videoSetModeSub(0);

	VRAM_CR = 0x80808080;
	VRAM_E_CR = 0x80;
	VRAM_F_CR = 0x80;
	VRAM_G_CR = 0x80;
	VRAM_H_CR = 0x80;
	VRAM_I_CR = 0x80;

	//irqInit();
	//REG_IME = 0;
	//REG_IE = 0;
	//REG_IF = ~0;
	//IRQ_HANDLER = dummy_irq_because_libnds_wants_it;
	//REG_IME = 1;

	dmaFillWords(0, BG_PALETTE, (2 * 1024));	// clear main and sub palette
	dmaFillWords(0, OAM, 2 * 1024);			// clear main and sub OAM
	dmaFillWords(0, VRAM, 656 * 1024);		// clear all VRAM

	VRAM_E_CR = 0;
	VRAM_F_CR = 0;
	VRAM_G_CR = 0;
	VRAM_H_CR = 0;
	VRAM_I_CR = 0;
}