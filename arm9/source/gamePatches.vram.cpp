#include "vram.h"
#include "sd_access.h"
#include "gamePatches.h"

void gptc_patchRom()
{
	u32 gameTitle0 = *(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xA0);
	u32 gameTitle1 = *(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xA4);
	u32 gameTitle2 = *(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xA8);
	u32 gameCode = *(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xAC);
	if (gameTitle0 == 0x49424942 && gameTitle1 == 0x414E4954 && gameTitle2 == 0x00003130 && gameCode == 0x44585542)
	{
		//Bibi und Tina - Ferien auf dem Martinshof (Germany)
		//Sound fix, by removing writeback from ldm
		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x12D9F4) == 0xE8B0000F)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x12D9F4) = 0xE890000F;
	}
	else if (gameTitle0 == 0x53524143 && gameTitle1 == 0x4554414D && gameTitle2 == 0x54414E52 && gameCode == 0x50504342)
	{
		//Cars - Mater-National Championship (Europe) (En,Fr,De,Es,It,Nl)
		//Sound fix, by removing writeback from ldm
		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xB664) == 0xE8B0000F)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xB664) = 0xE890000F;
	}
	else if (gameTitle0 == 0x53524143 && gameTitle1 == 0x4554414D && gameTitle2 == 0x00000052 && gameCode == 0x45504342)
	{
		//Cars - Mater-National Championship (USA) (En,Fr)
		//Sound fix, by removing writeback from ldm
		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xB664) == 0xE8B0000F)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xB664) = 0xE890000F;
	}
	/*else if (gameTitle0 == 0x4C415256 && gameTitle1 == 0x3320594C && gameTitle2 == 0x00000000 && gameCode == 0x45525641)
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
	}*/
	/*else if (gameTitle0 == 0x4159414D && gameTitle1 == 0x45485420 && gameTitle2 == 0x45454220 && gameCode == 0x50454542)
	{
		//Maya the Bee - Sweet Gold (Europe) (En,Fr,De,Es,It)
		//remove writeback from ldm
		if (*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1D0F4) == 0xE8B0000F)
			*(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0x1D0F4) = 0xE890000F;
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
