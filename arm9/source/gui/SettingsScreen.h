#pragma once
#include "core/UIManager.h"
#include "core/PaletteManager.h"
#include "core/ListAdapter.h"
#include "core/InputRepeater.h"
#include "core/ListRecycler.h"
#include "SingleLineIconListEntry.h"
#include "SettingsCategoryListAdapter.h"
#include "SettingsItemListEntry.h"
#include "Toolbar.h"
#include "UIContext.h"

struct settings_item_t
{
    SettingsItemMode mode;
    const char* title;
    const char* subTitle;
	u32* pValue;
};

struct settings_category_t
{
	const char* name;
	SettingsCategoryListAdapter::CategoryIcon icon;
	int itemCount;
	settings_item_t* items;
};

class SettingsScreen
{
	UIContext* _uiContext;

	ListRecycler* _listRecycler;
	ListAdapter* _adapter;

	InputRepeater _inputRepeater;

	int _selectedEntry;

	u16 _vramState;

	const settings_category_t* _selectedCategory;

	void GotoCategory(const settings_category_t* category);
public:
	SettingsScreen(UIContext* uiContext)
        : _uiContext(uiContext), _listRecycler(NULL), _adapter(NULL), _inputRepeater(0x3F3, 20, 8), _selectedEntry(0)
	{
	}

	~SettingsScreen()
	{
		if (_listRecycler)
			delete _listRecycler;
		if (_adapter)
			delete _adapter;
	}

	void Run();
};