#include "vram.h"
#include "sd_access.h"
#include "gamePatches.h"

static const u8 sSomeBuggedMixer[16] =
	{0xF0, 0x1F, 0x2D, 0xE9, 0x0F, 0x00, 0xB0, 0xE8, 0x03, 0x32, 0xA0, 0xE1, 0x01, 0x20, 0x82, 0xE0};

static const u8 sDbzLoGUPatch1[0x24] = 
	{0x0A, 0x1C, 0x40, 0x0B, 0xE0, 0x21, 0x09, 0x05, 0x41, 0x18, 0x07, 0x31, 0x00, 0x23, 0x08, 0x78,
	 0x10, 0x70, 0x01, 0x33, 0x01, 0x32, 0x01, 0x39, 0x07, 0x2B, 0xF8, 0xD9, 0x00, 0x20, 0x70, 0xBC,
	 0x02, 0xBC, 0x08, 0x47
	};

static const u8 sDbzLoGUPatch2[0x28] = 
	{0x70, 0xB5, 0x00, 0x04, 0x0A, 0x1C, 0x40, 0x0B, 0xE0, 0x21, 0x09, 0x05, 0x41, 0x18, 0x07, 0x31,
	 0x00, 0x23, 0x10, 0x78, 0x08, 0x70, 0x01, 0x33, 0x01, 0x32, 0x01, 0x39, 0x07, 0x2B, 0xF8, 0xD9,
	 0x00, 0x20, 0x70, 0xBC, 0x02, 0xBC, 0x08, 0x47
	};

// static const u8 sStarWarsNewDroidArmyPatch[40] = {
// 	0x00, 0x30, 0xA0, 0xE1, 0xFF, 0x7F, 0x83, 0xE8, 0x40, 0x30, 0x80, 0xE2,
// 	0xFF, 0x7F, 0x93, 0xE8, 0x1E, 0xFF, 0x2F, 0xE1, 0x40, 0x30, 0x80, 0xE2,
// 	0xFF, 0x7F, 0x83, 0xE8, 0x00, 0x30, 0xA0, 0xE1, 0xFF, 0x7F, 0x93, 0xE8,
// 	0x1E, 0xFF, 0x2F, 0xE1
// };


extern "C" void gptc_banjoPilotFix();
extern "C" void gptc_americanBassFix();

void gptc_patchRom()
{
	//fix a soundmixer that has ldm with writeback and rb in rlist
	u32* buggedMixer = gptc_findSignature(sSomeBuggedMixer);
	if (buggedMixer)
	{
		buggedMixer[1] = 0xE890000F;
		//some games contain it twice??
		buggedMixer = gptc_findSignature(sSomeBuggedMixer);
		if (buggedMixer)
			buggedMixer[1] = 0xE890000F;
	}

	u32 gameCode = *(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xAC);
	if(gameCode == 0x45474C41)
	{
		//Dragon Ball Z - The Legacy of Goku (USA)
		//Fix "game cannot be played on hardware found" error
		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x356) == 0x7002)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x356) = 0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x35E) == 0x7043)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x35E) = 0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x37E) == 0x7001)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x37E) = 0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x382) == 0x7041)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x382) = 0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xE27E) == 0xB0A2)
		{
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xE27E) = 0x400;

			for(int i = 0; i < sizeof(sDbzLoGUPatch1); i += 4)
				*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xE280 + i) = *(u32*)&sDbzLoGUPatch1[i];

			for(int i = 0; i < sizeof(sDbzLoGUPatch2); i += 4)
				*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xE32C + i) = *(u32*)&sDbzLoGUPatch2[i];
		}
	}
	else if(gameCode == 0x50474C41)
	{
		//Dragon Ball Z - The Legacy of Goku (Europe)
		//Fix "game cannot be played on hardware found" error
		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x33C) == 0x7119)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x33C) = 0x46C0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x340) == 0x7159)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x340) = 0x46C0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x356) == 0x705A)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x356) = 0x46C0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x35A) == 0x7002)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x35A) = 0x46C0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x35E) == 0x7042)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x35E) = 0x46C0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x384) == 0x7001)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x384) = 0x46C0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x388) == 0x7041)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x388) = 0x46C0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x494C) == 0x7002)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x494C) = 0x46C0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x4950) == 0x7042)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x4950) = 0x46C0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x4978) == 0x7001)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x4978) = 0x46C0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x497C) == 0x7041)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x497C) = 0x46C0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x988E) == 0x7028)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x988E) = 0x46C0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x9992) == 0x7068)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x9992) = 0x46C0;
	}
	else if(gameCode == 0x45424442)
	{
		//Dragon Ball Z - Taiketsu (USA)
		//Fix "game cannot be played on this hardware" error
		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2BD54) == 0x7818)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2BD54) = 0x2000;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2BD60) == 0x7810)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2BD60) = 0x2000;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2BD80) == 0x7839703A)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2BD80) = 0x21001C00;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2BD8C) == 0x78307030)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2BD8C) = 0x20001C00;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2BDAC) == 0x7008)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2BDAC) = 0x1C00;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2BDB2) == 0x7008)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x2BDB2) = 0x1C00;
	}
	else if(gameCode == 0x50424442)
	{
		//Dragon Ball Z - Taiketsu (Europe)
		//Fix "game cannot be played on this hardware" error
		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3FE08) == 0x7818)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3FE08) = 0x2000;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3FE14) == 0x7810)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3FE14) = 0x2000;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3FE34) == 0x7839703A)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3FE34) = 0x21001C00;

		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3FE40) == 0x78307030)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3FE40) = 0x20001C00;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3FE58) == 0x7008)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3FE58) = 0x1C00;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3FE66) == 0x7008)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3FE66) = 0x1C00;
	}
	else if(gameCode == 0x45334742)
	{
		//Dragon Ball Z - Buu's Fury (USA)
		//Fix "game will not run on this hardware" error
		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x8B66) == 0x7032)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x8B66) = 0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x8B6A) == 0x7072)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x8B6A) = 0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x8B86) == 0x7008)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x8B86) = 0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x8B8C) == 0x7031)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x8B8C) = 0;

		if (*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x8B90) == 0x7071)
			*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x8B90) = 0;
	}
	else if(gameCode == 0x504A4142 || gameCode == 0x454A4142)
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
	else if (gameCode == 0x45525641)
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
	else if (gameCode == 0x4A525641)
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
	else if (gameCode == 0x50525641)
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
	else if(gameCode == 0x45455241 || gameCode == 0x50455241 || gameCode == 0x4A455241 ||
			gameCode == 0x50324D41 || gameCode == 0x45324541 || gameCode == 0x4A324541 ||
			gameCode == 0x50583341 || gameCode == 0x45583341 || gameCode == 0x4A583341 || gameCode == 0x50423641 || gameCode == 0x45423641 || gameCode == 0x4A423641 ||
			gameCode == 0x50423442 || gameCode == 0x45423442 || gameCode == 0x4A423442 || gameCode == 0x50573442 || gameCode == 0x45573442 || gameCode == 0x4A573442)
	{
		//Mega Man Battle Network 1, 2, 3 and 4
		//Don't change sp of fiq, abt and und mode
		for(int i = 0; i < 0x24; i+=4)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xCC + i) = 0;
	}
	else if(gameCode == 0x45424141)
	{
		//Game crashes because of a race condition (caused by timing differences) related to a buffer
		//that contains some values for display related io registers. It it initialized by zeros,
		//and later the actual values are written, but before that it already makes a copy to io,
		//which disables vblank irqs and makes the game crash. This code ensures the irq is not
		//disabled, which fixes the entire game.
		*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x13C) = 0x4800; //ldr r0,= gptc_americanBassFix+1
		*(u16*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x13E) = 0x4700; //bx r0
		*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x140) = (u32)&gptc_americanBassFix + 1;
	}
#if defined(USE_DSI_16MB) || defined(USE_3DS_32MB)
	//These games all have the same problem (same code)
	//They uses abort mode for some things, idk why, but make it use undefined mode instead because abt mode is sacred in gbarunner2
	//Yet to do:
	//	- 2 Games in 1 - Bionicle + Knights' Kingdom
	//	- Lego Bionicle - Maze of Shadows
	//	- Medal of Honor - Infiltrator [UE]
	//Maybe there are more?
	else if(gameCode == 0x504E4B42 || gameCode == 0x454E4B42)
	{
		//Lego Knights' Kingdom
		*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xD0) = 0xE3A0001B;
		*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3ECB48) = 0xE3A0001B;
		*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3ECB84) = 0xE3A0009B;
	}
	else if(gameCode == 0x58574142)
	{
		//Alex Rider - Stormbreaker (Europe)
		*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x11C) = 0xE3A0001B;
		*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3C5978) = 0xE3A0001B;
		*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3C59B4) = 0xE3A0009B;
	}
	else if(gameCode == 0x58574142)
	{
		//Alex Rider - Stormbreaker (USA)
		*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x11C) = 0xE3A0001B;
		*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3D73C0) = 0xE3A0001B;
		*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x3D73FC) = 0xE3A0009B;
	}
#endif
	/*else if(gameCode == 0x45573241 || gameCode == 0x50573241)
	{
		//Star Wars - The New Droid Army
		for(int i = 0; i < sizeof(sStarWarsNewDroidArmyPatch); i += 4)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x53D0 + i) = *(u32*)&sStarWarsNewDroidArmyPatch[i];
	}*/
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
