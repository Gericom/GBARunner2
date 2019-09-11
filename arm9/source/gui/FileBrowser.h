#pragma once
#include "core/UIManager.h"
#include "FileBrowserListAdapter.h"
#include "core/InputRepeater.h"
#include "core/ListRecycler.h"
#include "fat/ff.h"
#include "save/Save.h"
#include "UIContext.h"
class NtftFont;

class FileBrowser
{
	int      _entryCount;
	FILINFO  _entries[64];
	FILINFO* _sortedEntries[64];

	UIContext* _uiContext;

	ListRecycler*           _listRecycler;
	FileBrowserListAdapter* _adapter;

	InputRepeater _inputRepeater;

	int _selectedEntry;

	u16 _vramState;

	void LoadBios();
	void LoadFolder(const char* path);
	void CreateLoadSave(const char* path, const save_type_t* saveType);
	void LoadGame(const char* path);
public:
	FileBrowser(UIContext* uiContext)
		: _uiContext(uiContext), _listRecycler(NULL), _adapter(NULL), _inputRepeater(0x3F3, 20, 8), _selectedEntry(0)
	{
	}

	~FileBrowser()
	{
		if (_listRecycler)
			delete _listRecycler;
		if (_adapter)
			delete _adapter;
	}

	int Run();
};
