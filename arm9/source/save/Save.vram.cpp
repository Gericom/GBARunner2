#include "vram.h"
#include "sd_access.h"
#include "EepromSave.h"
#include "FlashSave.h"
#include "SramSave.h"
#include "Save.h"

#define SAVE_TYPE_COUNT		25

static const save_type_t sSaveTypes[SAVE_TYPE_COUNT] =
{
	{"EEPROM_V111", 12, SAVE_TYPE_EEPROM_V111, 512, eeprom_patchV111},
	{"EEPROM_V120", 12, SAVE_TYPE_EEPROM_V120, 8 * 1024, eeprom_patchV120},
	{"EEPROM_V121", 12, SAVE_TYPE_EEPROM_V121, 8 * 1024, eeprom_patchV120},
	{"EEPROM_V122", 12, SAVE_TYPE_EEPROM_V122, 8 * 1024, eeprom_patchV120},
	{"EEPROM_V124", 12, SAVE_TYPE_EEPROM_V124, 8 * 1024, eeprom_patchV124},
	{"EEPROM_V125", 12, SAVE_TYPE_EEPROM_V125, 8 * 1024, eeprom_patchV124},
	{"EEPROM_V126", 12, SAVE_TYPE_EEPROM_V126, 8 * 1024, eeprom_patchV126},

	{"FLASH_V120", 11, SAVE_TYPE_FLASH_V120, 64 * 1024, flash_patchV120},
	{"FLASH_V121", 11, SAVE_TYPE_FLASH_V121, 64 * 1024, flash_patchV120},
	{"FLASH_V123", 11, SAVE_TYPE_FLASH_V123, 64 * 1024, flash_patchV123},
	{"FLASH_V124", 11, SAVE_TYPE_FLASH_V124, 64 * 1024, flash_patchV123},
	{"FLASH_V125", 11, SAVE_TYPE_FLASH_V125, 64 * 1024, flash_patchV123},
	{"FLASH_V126", 11, SAVE_TYPE_FLASH_V126, 64 * 1024, flash_patchV126},
	{"FLASH512_V130", 14, SAVE_TYPE_FLASH512_V130, 64 * 1024, flash_patch512V130},
	{"FLASH512_V131", 14, SAVE_TYPE_FLASH512_V131, 64 * 1024, flash_patch512V130},
	{"FLASH512_V133", 14, SAVE_TYPE_FLASH512_V133, 64 * 1024, flash_patch512V130},
	{"FLASH1M_V102", 13, SAVE_TYPE_FLASH1M_V102, 128 * 1024, flash_patch1MV102},
	{"FLASH1M_V103", 13, SAVE_TYPE_FLASH1M_V103, 128 * 1024, NULL}, //flash_patch1MV102 },

	//Fast sram does not require patching
	{"SRAM_F_V100", 12, SAVE_TYPE_SRAM_F_V100, 32 * 1024, NULL},
	{"SRAM_F_V102", 12, SAVE_TYPE_SRAM_F_V102, 32 * 1024, NULL},
	{"SRAM_F_V103", 12, SAVE_TYPE_SRAM_F_V103, 32 * 1024, NULL},

	{"SRAM_V110", 10, SAVE_TYPE_SRAM_V110, 32 * 1024, sram_patchV110},
	{"SRAM_V111", 10, SAVE_TYPE_SRAM_V111, 32 * 1024, sram_patchV111},
	{"SRAM_V112", 10, SAVE_TYPE_SRAM_V112, 32 * 1024, sram_patchV111},
	{"SRAM_V113", 10, SAVE_TYPE_SRAM_V113, 32 * 1024, sram_patchV111},
};

const save_type_t* save_findTag()
{
	//scan the rom for the save type tag
	f_rewind(&vram_cd->fil);
	UINT read;
	u32  curAddr = 0;
	if (f_read(&vram_cd->fil, vram_cd->cluster_cache, 128 * 1024, &read) != FR_OK)
		return NULL;
	int searchBufPtr = 0;
	while (curAddr < vram_cd->sd_info.gba_rom_size)
	{
		if (searchBufPtr == 0)
		{
			if (f_read(&vram_cd->fil, vram_cd->cluster_cache + 128 * 1024, 128 * 1024, &read) != FR_OK)
				return NULL;
		}
		else if (searchBufPtr == 128 * 1024)
		{
			if (f_read(&vram_cd->fil, vram_cd->cluster_cache, 128 * 1024, &read) != FR_OK)
				return NULL;
		}

		u32      fst = *(u32*)&vram_cd->cluster_cache[searchBufPtr];
		SaveType type = SAVE_TYPE_NONE;
		if (fst == 0x53414C46)
		{
			//FLAS
			type = SAVE_TYPE_FLASH;
		}
		else if (fst == 0x4D415253)
		{
			//SRAM
			type = SAVE_TYPE_SRAM;
		}
		else if (fst == 0x52504545)
		{
			//EEPR
			type = SAVE_TYPE_EEPROM;
		}

		if (type != SAVE_TYPE_NONE)
		{
			for (int i = 0; i < SAVE_TYPE_COUNT; i++)
			{
				if ((sSaveTypes[i].type & SAVE_TYPE_TYPE_MASK) != type)
					continue;
				bool found = true;
				for (int j = 4; j < ((sSaveTypes[i].tagLength + 3) & ~3); j += 4)
				{
					if (*(u32*)&sSaveTypes[i].tag[j] != *(u32*)&vram_cd->cluster_cache[(searchBufPtr + j) & 0xFFFFF])
					{
						found = false;
						break;
					}
				}
				if (found)
				{
					f_lseek(&vram_cd->fil, curAddr);
					return &sSaveTypes[i];
				}
			}
		}
		searchBufPtr = (searchBufPtr + 4) & 0xFFFFF;
		curAddr += 4;
	}
	return NULL;
}

void save_injectJump(u32* location, void* jumpTarget)
{
	if (((u32)location & 2) != 0)
	{
		*(u16*)location = 0x0000;
		location = (u32*)((u32)location + 2);
	}
	location[0] = 0x00004778; //bx pc; nop
	location[1] = 0xE51FF004; //ldr pc,= address
	location[2] = (u32)jumpTarget;
}
