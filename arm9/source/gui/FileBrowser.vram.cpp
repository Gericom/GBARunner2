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
#include "settings.h"
#include "gbaBoot.h"
#include "FileBrowser.h"

static int compDirEntries(const FILINFO*& dir1, const FILINFO*& dir2)
{
	if (dir1->fattrib & AM_DIR && !(dir2->fattrib & AM_DIR))
		return -1;
	if (!(dir1->fattrib & AM_DIR) && dir2->fattrib & AM_DIR)
		return 1;
	return strcasecmp(dir1->fname, dir2->fname);
}

void FileBrowser::LoadFolder(const char* path)
{
	if (f_opendir(&vram_cd->dir, path) != FR_OK)
		_uiContext->FatalError("Error while reading directory!");
	int     entryCount = 0;
	FILINFO info;
	while (true)
	{
		if (f_readdir(&vram_cd->dir, &info) != FR_OK)
			_uiContext->FatalError("Error while reading directory!");
		//First check end of directory, otherwise attributes are not valid!	
		if (info.fname[0] == 0)
			break;
		//Don't show system and hidden files
		if (info.fattrib & (AM_SYS | AM_HID))
			continue;
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
	//now find the id for each entry
	for(int i = 0; i < _entryCount; i++)
	{
		if(_sortedEntries[i]->fattrib & AM_DIR)
			continue;
		if (f_open(&vram_cd->fil, _sortedEntries[i]->fname, FA_OPEN_EXISTING | FA_READ) != FR_OK)
			_ids[i] = 0xFFFFFFFF;
		else
		{
			f_lseek(&vram_cd->fil, 0xAC);
			u32 id = 0;
			UINT br;
			f_read(&vram_cd->fil, (void*)&id, 4, &br);
			f_close(&vram_cd->fil);
			_ids[i] = id;
		}
	}
	if (_listRecycler)
	{
		_uiContext->GetUIManager().RemoveElement(_listRecycler);
		delete _listRecycler;
	}
	if (_adapter)
		delete _adapter;

	//clear everything except the toolbar
	_uiContext->GetUIManager().Update();
	_uiContext->GetUIManager().VBlank();

	_uiContext->GetUIManager().GetSubObjManager().SetState(_vramState);
	_adapter = new FileBrowserListAdapter(_uiContext->GetRobotoRegular11(), _sortedEntries, _entryCount);
	_listRecycler = new ListRecycler(0, 36, 256, 156, 5, _adapter);
	_selectedEntry = 0;
	_listRecycler->SetSelectedIdx(0);
	_uiContext->GetUIManager().AddElement(_listRecycler);

	//load the text graphics for next menu
	_uiContext->GetUIManager().VBlank();
	InvalidateCover();
}

void FileBrowser::LoadGame(const char* path)//, u32 id)
{
	if(_coverLoadState == COVER_LOAD_STATE_LOAD)
		f_close(&vram_cd->fil);

	//gbab_loadFrame(id);

	switch(gbab_loadRom(path))
	{
		case ROM_LOAD_RESULT_ROM_READ_ERR:
			_uiContext->FatalError("Error while reading rom!");
			break;
		case ROM_LOAD_RESULT_SAVE_CREATE_ERR:
			_uiContext->FatalError("Error creating save file!");
			break;
		case ROM_LOAD_RESULT_SAVE_TOO_SMALL:
			_uiContext->FatalError("Save file too small!");
			break;
		case ROM_LOAD_RESULT_SAVE_READ_ERR:
			_uiContext->FatalError("Error while reading save file!");
			break;
	}
}

static bool loadCover(const char* path)
{
	if (f_open(&vram_cd->fil, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
		return false;
	UINT br;
	f_read(&vram_cd->fil, (void*)MAIN_MEMORY_ADDRESS_ROM_DATA, 256 * 192 * 2, &br);
	f_close(&vram_cd->fil);
	return true;
}

void FileBrowser::UpdateCover()
{
	switch(_coverLoadState)
	{
		case COVER_LOAD_STATE_IDLE:
			break;

		case COVER_LOAD_STATE_OPEN:
		{
			if(!(_sortedEntries[_selectedEntry]->fattrib & AM_DIR))
				_gameId = _ids[_selectedEntry];
			char coverPath[] = "/_gba/covers/A/B/ABCD.bin";
			const char* path;
			if (_sortedEntries[_selectedEntry]->fattrib & AM_DIR)
				path = "/_gba/covers/folder.bin";
			else if (_gameId == 0xFFFFFFFF)
				path = "/_gba/covers/unknown.bin";
			else
			{				
				coverPath[13] = _gameId & 0xFF;
				coverPath[15] = (_gameId >> 8) & 0xFF;
				coverPath[17] = _gameId & 0xFF;
				coverPath[18] = (_gameId >> 8) & 0xFF;
				coverPath[19] = (_gameId >> 16) & 0xFF;
				coverPath[20] = (_gameId >> 24) & 0xFF;
				path = coverPath;
			}
			
			if (f_open(&vram_cd->fil, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
			{
				if(_sortedEntries[_selectedEntry]->fattrib & AM_DIR)
				{
					_coverLoadState = COVER_LOAD_STATE_IDLE;
					break;
				}
				if (_gameId == 0xFFFFFFFF || f_open(&vram_cd->fil, "/_gba/covers/unknown.bin", FA_OPEN_EXISTING | FA_READ) != FR_OK)
				{
					_coverLoadState = COVER_LOAD_STATE_IDLE;
					break;
				}
			}
			_coverLoadState = COVER_LOAD_STATE_LOAD;
			break;
		}
		case COVER_LOAD_STATE_LOAD:
		{
			UINT br;
			f_read(&vram_cd->fil, (void*)MAIN_MEMORY_ADDRESS_ROM_DATA, 256 * 192 * 2, &br);
			f_close(&vram_cd->fil);
			_coverLoadState = COVER_LOAD_STATE_COPY;
			break;
		}
		case COVER_LOAD_STATE_COPY:
			break;
	}
}

void FileBrowser::InvalidateCover()
{
	if(_coverLoadState == COVER_LOAD_STATE_LOAD)
		f_close(&vram_cd->fil);
	_coverLoadState = COVER_LOAD_STATE_OPEN;
}

int FileBrowser::Run()
{
	int next = 2;
	_uiContext->GetToolbar().SetTitle("GBARunner2");
	_uiContext->GetToolbar().SetShowBackButton(false);
	_uiContext->GetToolbar().SetShowSettingsButton(true);
	_uiContext->GetUIManager().Update();
	while (*((vu16*)0x04000004) & 1);
	while (!(*((vu16*)0x04000004) & 1));
	_uiContext->GetUIManager().VBlank();
	FileBrowserListEntry::LoadCommonData(_uiContext->GetUIManager());
	_vramState = _uiContext->GetUIManager().GetSubObjManager().GetState();

	f_chdir("/");
	if (f_stat("gba", NULL) == FR_OK)
		f_chdir("gba");
	LoadFolder(".");
	while (1)
	{		
		_inputRepeater.Update(~keys_read());
		if (_inputRepeater.GetTriggeredKeys() & KEY_UP)
		{
			if (_selectedEntry > 0)
			{
				_selectedEntry--;
				InvalidateCover();
			}
		}
		else if (_inputRepeater.GetTriggeredKeys() & KEY_DOWN)
		{
			if (_selectedEntry < _entryCount - 1)
			{
				_selectedEntry++;
				InvalidateCover();
			}
		}
		else if (_inputRepeater.GetTriggeredKeys() & KEY_A)
		{
			if (_sortedEntries[_selectedEntry]->fattrib & AM_DIR)
			{
				f_chdir(_sortedEntries[_selectedEntry]->fname);
				LoadFolder(".");
			}
			else
			{
				LoadGame(_sortedEntries[_selectedEntry]->fname);//, _ids[_selectedEntry]);
				break;
			}
		}
		else if (_inputRepeater.GetTriggeredKeys() & KEY_B)
		{
			f_chdir("..");
			LoadFolder(".");
		}
		else if (_inputRepeater.GetTriggeredKeys() & KEY_R)
		{
			next = 1;
			break;
		}
		_listRecycler->SetSelectedIdx(_selectedEntry);
		_uiContext->GetUIManager().Update();
		UpdateCover();
		while (*((vu16*)0x04000004) & 1);
		while (!(*((vu16*)0x04000004) & 1));
		_uiContext->GetBGPalManager().Apply(BG_PALETTE_SUB);
		_uiContext->GetUIManager().VBlank();
		if(_coverLoadState == COVER_LOAD_STATE_COPY)
		{
			arm9_memcpy16((u16*)0x06940000, (u16*)MAIN_MEMORY_ADDRESS_ROM_DATA, 256 * 192);
			_coverLoadState = COVER_LOAD_STATE_IDLE;
		}
	}
	if (_listRecycler)
		_uiContext->GetUIManager().RemoveElement(_listRecycler);
		
	//clear everything except the toolbar
	_uiContext->GetUIManager().Update();
	_uiContext->GetUIManager().VBlank();
	return next;
}
