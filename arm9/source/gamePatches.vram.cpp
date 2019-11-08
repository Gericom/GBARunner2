#include "vram.h"
#include "sd_access.h"
#include "gamePatches.h"

static const u8 sSomeBuggedMixer[16] =
	{0xF0, 0x1F, 0x2D, 0xE9, 0x0F, 0x00, 0xB0, 0xE8, 0x03, 0x32, 0xA0, 0xE1, 0x01, 0x20, 0x82, 0xE0};

static const u8 sMP2000SoundInit[16] =
	{0x53, 0x6D, 0x73, 0x68, 0x70, 0xB5, 0x14, 0x48, 0x02, 0x21, 0x49, 0x42, 0x08, 0x40, 0x13, 0x49};

static const u8 sMP2000SoundInit2[16] =
	{0x53, 0x6D, 0x73, 0x68, 0x70, 0xB5, 0x1F, 0x48, 0x02, 0x21, 0x49, 0x42, 0x08, 0x40, 0x1E, 0x49};

static const u8 sMP2000SoundInit3[16] =
	{0x53, 0x6D, 0x73, 0x68, 0xF0, 0xB5, 0x47, 0x46, 0x80, 0xB4, 0x18, 0x48, 0x02, 0x21, 0x49, 0x42};
    
extern "C" void gptc_banjoPilotFix();

void gptc_patchRom()
{
	//fix a soundmixer that has ldm with writeback and rb in rlist
	u32* buggedMixer = gptc_findSignature(sSomeBuggedMixer);
	if (buggedMixer)
		buggedMixer[1] = 0xE890000F;

	//todo: it seems not all games have this same signature
	u32* mp2000Init = gptc_findSignature(sMP2000SoundInit);
	if (!mp2000Init)
		mp2000Init = gptc_findSignature(sMP2000SoundInit2);
	if (!mp2000Init)
	{
		mp2000Init = gptc_findSignature(sMP2000SoundInit3);
		if (mp2000Init)
			mp2000Init++;
	}
	if (mp2000Init)
	{
		//this is a music player 2000 game
		u16* pLdrSoundArea = (u16*)((u32)mp2000Init + 0x16);
		u16  ldrSoundArea = *pLdrSoundArea;
		u32* pSoundAreaPtr = (u32*)(((u32)pLdrSoundArea & ~3) + 4 + ((ldrSoundArea & 0xFF) << 2));
		u32  oldVal = *pSoundAreaPtr;
		//relocate SoundArea to uncached main memory
		if (oldVal >= 0x02000000 && oldVal < 0x04000000)
			*pSoundAreaPtr = (u32)&vram_cd->mp2000SoundArea[0] | 0x00800000;
	}

	u32 gameTitle0 = *(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xA0);
	u32 gameTitle1 = *(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xA4);
	u32 gameTitle2 = *(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xA8);
	u32 gameCode = *(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xAC);
	if(gameTitle0 == 0x4A4E4142 && gameTitle1 == 0x4950204F && gameTitle2 == 0x544F4C && (gameCode == 0x504A4142 || gameCode == 0x454A4142))
	{
		//Banjo-Pilot (Europe) (En,Fr,De,Es,It) and Banjo-Pilot (USA)
		//Prevent race condition from happening
		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2134) == 0x4861089B)
		{
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2134) = 0x4800; //ldr r0,= gptc_banjoPilotFix+1
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2136) = 0x4700; //bx r0
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2138) = (u32)&gptc_banjoPilotFix + 1;
		}
	}
	else if (gameTitle0 == 0x4C415256 && gameTitle1 == 0x3320594C && gameTitle2 == 0x00000000 && gameCode == 0x45525641)
	{
		//V-Rally 3 (USA) (En,Fr,Es)
		//remove writeback from ldm
		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x6BBD0) == 0xE8B00001)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x6BBD0) = 0xE8900001;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x77BD4) == 0xE8B00001)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x77BD4) = 0xE8900001;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1FC698) == 0xE8B2001C)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1FC698) = 0xE892001C;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1FC71C) == 0xE8B2001C)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1FC71C) = 0xE892001C;
	}
	else if (gameTitle0 == 0x4C415256 && gameTitle1 == 0x3320594C && gameTitle2 == 0x00000000 && gameCode == 0x4A525641)
	{
		//V-Rally 3 (Japan)
		//remove writeback from ldm
		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x6CEC8) == 0xE8B00001)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x6CEC8) = 0xE8900001;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x78E50) == 0xE8B00001)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x78E50) = 0xE8900001;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1FD508) == 0xE8B2001C)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1FD508) = 0xE892001C;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1FD58C) == 0xE8B2001C)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1FD58C) = 0xE892001C;
	}
	else if (gameTitle0 == 0x41522D56 && gameTitle1 == 0x20594C4C && gameTitle2 == 0x00000033 && gameCode == 0x50525641)
	{
		//V-Rally 3 (Europe) (En,Fr,De,Es,It)
		//remove writeback from ldm
		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x6BBA0) == 0xE8B00001)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x6BBA0) = 0xE8900001;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x77B0C) == 0xE8B00001)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x77B0C) = 0xE8900001;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1FC1C4) == 0xE8B2001C)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1FC1C4) = 0xE892001C;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1FC248) == 0xE8B2001C)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1FC248) = 0xE892001C;
	}
	/*else if(gameTitle0 == 0x4E534944 && gameTitle1 == 0x4F565945 && gameTitle2 == 0x3130304C && gameCode == 0x4543444D)
	{
		//Game Boy Advance Video - Disney Channel Collection - Volume 1 (USA)
		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xC5C) == 0x4478)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xC5C) = 0x4470;
		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xCF4) == 0x94)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xCF4) = 0x9D;
	}*/
}

u32* gptc_findSignature(const u8* signature)
{
	u32* pRom = (u32*)MAIN_MEMORY_ADDRESS_ROM_DATA;
	bool found = false;
	for (int i = 0; i < ROM_DATA_LENGTH; i += 4)
	{
		if (pRom[0] == ((u32*)signature)[0] && pRom[1] == ((u32*)signature)[1] &&
			pRom[2] == ((u32*)signature)[2] && pRom[3] == ((u32*)signature)[3])
		{
			found = true;
			break;
		}
		pRom++;
	}
	if (!found)
		return NULL;
	return pRom;
}
