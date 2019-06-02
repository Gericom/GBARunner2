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
	FRESULT result = f_open(&vram_cd->fil, "0:/bios.bin", FA_OPEN_EXISTING | FA_READ);
	if (result != FR_OK)
		result = f_open(&vram_cd->fil, "0:/gba/bios.bin", FA_OPEN_EXISTING | FA_READ);
	if (result != FR_OK)
		FatalError("Bios not found!");
	if(vram_cd->fil.obj.objsize != 16 * 1024)
		FatalError("Invalid bios size!");
	UINT br;
	if (f_read(&vram_cd->fil, (void*)0, 16 * 1024, &br) != FR_OK || br != 16 * 1024)
		FatalError("Error while loading bios!");
	f_close(&vram_cd->fil);
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

void FileBrowser::CreateLoadSave(const char* path)
{
	FRESULT result = f_open(&vram_cd->fil, path, FA_OPEN_EXISTING | FA_READ);
	if (result == FR_OK)
	{
		if(vram_cd->fil.obj.objsize != 64 * 1024)
			FatalError("Save file size invalid!\nShould be 64 kb.");
		UINT br;
		if (f_read(&vram_cd->fil, (void*)0x23F0000, 64 * 1024, &br) != FR_OK || br != 64 * 1024)
			FatalError("Error while reading save file!");
		f_close(&vram_cd->fil);
	}
	else
	{
		for (int i = 0; i < 64 * 1024 / 4; i++)
		{
			((uint32_t*)0x23F0000)[i] = 0;
		}
	}
}

void FileBrowser::LoadGame(const char* path)
{
	if (f_open(&vram_cd->fil, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
		FatalError("Error while opening rom!");
	vram_cd->sd_info.gba_rom_size = vram_cd->fil.obj.objsize;
	uint32_t* cluster_table = &vram_cd->gba_rom_cluster_table[0];
	uint32_t  cur_cluster = vram_cd->fil.obj.sclust;
	while (cur_cluster > 2 && cur_cluster < 0x0FFFFFF8)
	{
		*cluster_table = f_clst2sect(&vram_cd->fatFs, cur_cluster);
		cluster_table++;
		cur_cluster = f_getFat(&vram_cd->fil, cur_cluster);
	}
	UINT br;
	if (f_read(&vram_cd->fil, (void*)MAIN_MEMORY_ADDRESS_ROM_DATA, ROM_DATA_LENGTH, &br) != FR_OK)
		FatalError("Error while reading rom!");
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

	CreateLoadSave(nameBuf);
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
