#pragma once
#include "core/UIManager.h"
#include "FileBrowserListAdapter.h"
#include "core/InputRepeater.h"
#include "core/ListRecycler.h"
class NtftFont;

class FileBrowser
{
	int      _entryCount;
	FILINFO  _entries[64];
	FILINFO* _sortedEntries[64];

	PaletteManager _bgPalMan;
	UIManager      _uiManager;

	NtftFont* _robotoMedium13;
	NtftFont* _robotoRegular11;

	ListRecycler*           _listRecycler;
	FileBrowserListAdapter* _adapter;

	InputRepeater _inputRepeater;

	int _selectedEntry;

	u16 _vramState;

	void LoadBios();
	void LoadFolder(const char* path);
	void CreateLoadSave(const char* path);
	void LoadGame(const char* path);
	void FatalError(const char* error);
public:
	FileBrowser()
		: _listRecycler(NULL), _adapter(NULL), _inputRepeater(0x3F3, 20, 8), _selectedEntry(0)
	{
	}

	~FileBrowser()
	{
		if (_listRecycler)
			delete _listRecycler;
		if (_adapter)
			delete _adapter;
	}

	void Run();
};
