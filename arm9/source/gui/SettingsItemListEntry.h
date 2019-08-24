#pragma once

#include "core/ListEntry.h"
#include "core/NtftFont.h"

struct settings_item_t;

enum SettingsItemMode : u16
{
    SETTINGS_ITEM_MODE_SIMPLE,
    SETTINGS_ITEM_MODE_SWITCH,
    SETTINGS_ITEM_MODE_CHECK
};

class SettingsItemListEntry : public ListEntry
{
    static u16 sCheckboxOutlineObjAddr;
    static u16 sCheckboxMarkedObjAddr;

	const NtftFont* _titleFont;
	u16 _titleObjAddr;
	u16 _titleInvalidated;
	//char _title[64];
    const NtftFont* _subTitleFont;
    u16 _subTitleObjAddr;
	u16 _subTitleInvalidated;
    //char _subTitle[64];
    //ItemMode _mode;
    //u16 _checked;
    const settings_item_t* _settingsItem;
public:	
	static void LoadCommonData(UIManager& uiManager);

    explicit SettingsItemListEntry(const NtftFont* titleFont, const NtftFont* subTitleFont)
		: _titleFont(titleFont), _subTitleFont(subTitleFont)
	{
	}

	void Initialize(UIManager& uiManager);
	void Update(UIManager&     uiManager);
	void VBlank(UIManager&     uiManager);

	void SetTitleFont(const NtftFont* font)
	{
		_titleFont = font;
		_titleInvalidated = true;
	}

	//void SetTitle(const char* title);

    void SetSubTitleFont(const NtftFont* font)
	{
		_subTitleFont = font;
		_subTitleInvalidated = true;
	}

    void SetSettingsItem(const settings_item_t* settingsItem) 
    {
        _settingsItem = settingsItem;
        _titleInvalidated = true;
        _subTitleInvalidated = true;
    }

	//void SetSubTitle(const char* subTitle);

    //void SetMode(ItemMode mode) { _mode = mode; }
    //void SetChecked(bool checked) { _checked = checked; }
};