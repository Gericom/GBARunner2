#include <nds/arm9/sprite.h>
#include <nds/arm9/background.h>
#include "vram.h"
#include "vramheap.h"
#include "vector.h"
#include "gui/core/UIManager.h"
#include "gui/Toolbar.h"
#include "gui/core/NtftFont.h"
#include "RobotoMedium13_ntft.h"
#include "RobotoRegular11_ntft.h"
#include "BarShadow_nbfp.h"
#include "BarShadow_nbfc.h"
#include "BarShadow_nbfs.h"
#include "gui/FileBrowserListEntry.h"
#include "Dialog.h"
#include "core/ListRecycler.h"
#include "FileBrowserListAdapter.h"
#include "core/InputRepeater.h"
#include "qsort.h"
#include "crc16.h"
#include "save/Save.h"
#include "bios.h"
#include "FileBrowser.h"

static int compDirEntries(const FILINFO*& dir1, const FILINFO*& dir2)
{
	if (dir1->fattrib & AM_DIR && !(dir2->fattrib & AM_DIR))
		return -1;
	if (!(dir1->fattrib & AM_DIR) && dir2->fattrib & AM_DIR)
		return 1;
	return strcasecmp(dir1->fname, dir2->fname);
}

void FileBrowser::LoadBios()
{
	switch (bios_load())
	{
		case BIOS_LOAD_RESULT_OK:
			break;
		case BIOS_LOAD_RESULT_ERROR:
			FatalError("Error while loading bios!");
			break;
		case BIOS_LOAD_RESULT_NOT_FOUND:
			FatalError("Bios not found!");
			break;
		case BIOS_LOAD_RESULT_INVALID:
			FatalError("Bios invalid!");
			break;
	}
}

void FileBrowser::LoadFolder(const char* path)
{
	if (f_opendir(&vram_cd->dir, path) != FR_OK)
		FatalError("Error while reading directory!");
	int     entryCount = 0;
	FILINFO info;
	while (true)
	{
		if (f_readdir(&vram_cd->dir, &info) != FR_OK)
			FatalError("Error while reading directory!");
		if (info.fattrib & (AM_SYS | AM_HID))
			continue;
		if (info.fname[0] == 0)
			break;
		uint8_t* point_ptr = (uint8_t*)strrchr(info.fname, '.');
		if (info.fattrib & AM_DIR || (point_ptr && !strcasecmp((char*)point_ptr, ".gba")))
		{
			_sortedEntries[entryCount] = &_entries[entryCount];
			arm9_memcpy16((u16*)&_entries[entryCount++], (u16*)&info, sizeof(FILINFO) >> 1);
			if (entryCount == 64)
				break;
		}
	}
	f_closedir(&vram_cd->dir);
	qsort(_sortedEntries, entryCount, sizeof(FILINFO*), (int(*)(const void*, const void*))compDirEntries);
	_entryCount = entryCount;
	if (_listRecycler)
	{
		_uiManager.RemoveElement(_listRecycler);
		delete _listRecycler;
	}
	if (_adapter)
		delete _adapter;
	_uiManager.GetSubObjManager().SetState(_vramState);
	_adapter = new FileBrowserListAdapter(_robotoRegular11, _sortedEntries, _entryCount);
	_listRecycler = new ListRecycler(0, 36, 256, 156, 5, _adapter);
	_selectedEntry = 0;
	_listRecycler->SetSelectedIdx(0);
	_uiManager.AddElement(_listRecycler);
}

void FileBrowser::CreateLoadSave(const char* path, const save_type_t* saveType)
{
	if (saveType)
		vram_cd->save_work.saveSize = saveType->size;
	else
		vram_cd->save_work.saveSize = 64 * 1024;
	vram_cd->save_work.save_state = SAVE_WORK_STATE_CLEAN;
	if (f_open(&vram_cd->fil, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
	{
		if (saveType && (saveType->type & SAVE_TYPE_TYPE_MASK) == SAVE_TYPE_FLASH)
		{
			for (int i = 0; i < vram_cd->save_work.saveSize >> 2; i++)
				((uint32_t*)MAIN_MEMORY_ADDRESS_SAVE_DATA)[i] = 0xFFFFFFFF;
		}
		else
		{
			for (int i = 0; i < vram_cd->save_work.saveSize >> 2; i++)
				((uint32_t*)MAIN_MEMORY_ADDRESS_SAVE_DATA)[i] = 0;
		}

#ifdef ISNITRODEBUG
		vram_cd->save_work.save_enabled = 0;
		return;
#else
		if (f_open(&vram_cd->fil, path, FA_CREATE_NEW | FA_WRITE) != FR_OK)
			FatalError("Error creating save file!");

		UINT bw;
		if (f_write(&vram_cd->fil, (void*)MAIN_MEMORY_ADDRESS_SAVE_DATA, vram_cd->save_work.saveSize, &bw) != FR_OK ||
			bw != vram_cd->save_work.saveSize)
			FatalError("Error creating save file!");
		f_close(&vram_cd->fil);
		if (f_open(&vram_cd->fil, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
			FatalError("Error creating save file!");
#endif
	}

	if (saveType && (saveType->type & SAVE_TYPE_TYPE_MASK) == SAVE_TYPE_EEPROM && vram_cd->fil.obj.objsize == 512)
		vram_cd->save_work.saveSize = 512;

	if (vram_cd->fil.obj.objsize < vram_cd->save_work.saveSize)
		FatalError("Save file too small!");

	uint32_t* cluster_table = &vram_cd->save_work.save_fat_table[0];
	uint32_t  cur_cluster = vram_cd->fil.obj.sclust;
	while (cur_cluster >= 2 && cur_cluster != 0xFFFFFFFF)
	{
		*cluster_table = f_clst2sect(&vram_cd->fatFs, cur_cluster);
		cluster_table++;
		cur_cluster = f_getFat(&vram_cd->fil, cur_cluster);
	}
	*cluster_table = 0;

	UINT br;
	if (f_read(&vram_cd->fil, (void*)MAIN_MEMORY_ADDRESS_SAVE_DATA, vram_cd->save_work.saveSize, &br) != FR_OK ||
		br != vram_cd->save_work.saveSize)
		FatalError("Error while reading save file!");
	f_close(&vram_cd->fil);

	vram_cd->save_work.fat_table_crc = crc16(0xFFFF, vram_cd->save_work.save_fat_table,
	                                         sizeof(vram_cd->save_work.save_fat_table));
#ifdef ISNITRODEBUG
	vram_cd->save_work.save_enabled = 0;
#else
	vram_cd->save_work.save_enabled = 1;
#endif
}

void FileBrowser::LoadGame(const char* path)
{
	if (f_open(&vram_cd->fil, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
		FatalError("Error while opening rom!");
	vram_cd->sd_info.gba_rom_size = vram_cd->fil.obj.objsize;
	uint32_t* cluster_table = &vram_cd->gba_rom_cluster_table[0];
	uint32_t  cur_cluster = vram_cd->fil.obj.sclust;
	while (cur_cluster >= 2 && cur_cluster != 0xFFFFFFFF)
	{
		*cluster_table = f_clst2sect(&vram_cd->fatFs, cur_cluster);
		cluster_table++;
		cur_cluster = f_getFat(&vram_cd->fil, cur_cluster);
	}
	UINT br;
	if (f_read(&vram_cd->fil, (void*)MAIN_MEMORY_ADDRESS_ROM_DATA, ROM_DATA_LENGTH, &br) != FR_OK)
		FatalError("Error while reading rom!");

	const save_type_t* saveType = save_findTag();
	if (saveType != NULL)
	{
		if (saveType->patchFunc != NULL)
			saveType->patchFunc(saveType);
	}

	f_close(&vram_cd->fil);

	char nameBuf[256];
	for (int i = 0; i < 256; i++)
	{
		char c = path[i];
		nameBuf[i] = c;
		if (c == 0)
			break;
	}

	char* long_name_ptr = strrchr(nameBuf, '.');
	long_name_ptr[1] = 's';
	long_name_ptr[2] = 'a';
	long_name_ptr[3] = 'v';
	long_name_ptr[4] = '\0';

	CreateLoadSave(nameBuf, saveType);
}

void FileBrowser::FatalError(const char* error)
{
	Dialog* dialog = new Dialog(_robotoMedium13, "Error", _robotoRegular11, error);
	_uiManager.AddElement(dialog);
	_uiManager.GetSubObjPalManager().DimRows(0x3FFF);
	_bgPalMan.DimRows(0x7FFF);
	while (1)
	{
		_uiManager.Update();
		while (*((vu16*)0x04000004) & 1);
		while (!(*((vu16*)0x04000004) & 1));
		_bgPalMan.Apply(BG_PALETTE_SUB);
		_uiManager.VBlank();
	}
}

void FileBrowser::Run()
{
	REG_DISPCNT_SUB = DISPLAY_BG1_ACTIVE | DISPLAY_BG2_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D |
		DISPLAY_SPR_1D_SIZE_32 | MODE_0_2D;

	//toolbar shadow
	for (int i = 0; i < 16; i++)
		_bgPalMan.palette[i].color = ((u16*)BarShadow_nbfp)[i];
	for (int i = 0; i < (64 >> 1); i++)
		BG_GFX_SUB[i] = ((u16*)BarShadow_nbfc)[i];
	for (int i = 0; i < 2048 / 2; i++)
		BG_GFX_SUB[(0x3800 >> 1) + i] = ((u16*)BarShadow_nbfs)[i];
	REG_BG1CNT_SUB = BG_32x32 | BG_PRIORITY_2 | BG_COLOR_16 | BG_MAP_BASE(7);
	REG_BLDCNT_SUB = BLEND_ALPHA | BLEND_SRC_BG1 | BLEND_DST_BG0 | BLEND_DST_SPRITE | BLEND_DST_BACKDROP;
	REG_BLDALPHA_SUB = 4 | (12 << 8);
	_bgPalMan.palette[0].color = RGB5(31, 31, 31);

	//dialogue bg
	BG_GFX_SUB[32] = 0x1112;
	for (int i = 0; i < (64 >> 1) - 1; i++)
		BG_GFX_SUB[33 + i] = 0x1111;
	REG_BG2CNT_SUB = BG_32x32 | BG_PRIORITY_2 | BG_COLOR_16 | BG_MAP_BASE(8);
	_bgPalMan.palette[15 * 16 + 1].color = RGB5(31, 31, 31);
	_bgPalMan.palette[15 * 16 + 2].color = RGB5(157 >> 3, 157 >> 3, 157 >> 3);

	_robotoMedium13 = new NtftFont(RobotoMedium13_ntft);
	_robotoRegular11 = new NtftFont(RobotoRegular11_ntft);
	Toolbar::LoadCommonData(_uiManager);
	Toolbar* toolbar = new Toolbar(RGB5(103 >> 3, 58 >> 3, 183 >> 3), 0x7FFF, _robotoMedium13, "GBARunner2", 0x7FFF);
	FileBrowserListEntry::LoadCommonData(_uiManager);
	_uiManager.AddElement(toolbar);
	_vramState = _uiManager.GetSubObjManager().GetState();

	if (f_mount(&vram_cd->fatFs, "", 1) != FR_OK)
		FatalError("Couldn't mount sd card!");

	LoadBios();
	f_chdir("/");
	if (f_stat("gba", NULL) == FR_OK)
		f_chdir("gba");
	LoadFolder(".");
	while (1)
	{
		_inputRepeater.Update(~*((vu16*)0x04000130));
		if (_inputRepeater.GetTriggeredKeys() & (1 << 6))
		{
			if (_selectedEntry > 0)
				_selectedEntry--;
		}
		else if (_inputRepeater.GetTriggeredKeys() & (1 << 7))
		{
			if (_selectedEntry < _entryCount - 1)
				_selectedEntry++;
		}
		else if (_inputRepeater.GetTriggeredKeys() & 1)
		{
			if (_sortedEntries[_selectedEntry]->fattrib & AM_DIR)
			{
				f_chdir(_sortedEntries[_selectedEntry]->fname);
				LoadFolder(".");
			}
			else
			{
				LoadGame(_sortedEntries[_selectedEntry]->fname);
				break;
			}
		}
		else if (_inputRepeater.GetTriggeredKeys() & 2)
		{
			f_chdir("..");
			LoadFolder(".");
		}
		_listRecycler->SetSelectedIdx(_selectedEntry);
		_uiManager.Update();
		while (*((vu16*)0x04000004) & 1);
		while (!(*((vu16*)0x04000004) & 1));
		_bgPalMan.Apply(BG_PALETTE_SUB);
		_uiManager.VBlank();
	}
	_uiManager.GetSubOamManager().Clear();
	_uiManager.GetSubOamManager().Apply(OAM_SUB);
	REG_BLDCNT_SUB = 0;
	REG_BLDALPHA_SUB = 0;
	delete toolbar;
}
