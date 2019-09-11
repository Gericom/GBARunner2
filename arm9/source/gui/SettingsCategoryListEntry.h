#pragma once

#include "core/ListEntry.h"
#include "core/NtftFont.h"

class SettingsCategoryListEntry : public ListEntry
{
	static u16 sPlayCircleObjAddr;
	static u16 sGamepadObjAddr;
	static u16 sInfoObjAddr;

public:
	enum CategoryIcon : u16
	{
		SETTINGS_CATEGORY_ICON_PLAYCIRCLE,
		SETTINGS_CATEGORY_ICON_GAMEPAD,
		SETTINGS_CATEGORY_ICON_INFO
	};

private:
	const NtftFont* _font;
	CategoryIcon _icon;
	u16       _nameObjAddr;
	char      _name[128];
	u16      _nameInvalidated;
public:	
	static void LoadCommonData(UIManager& uiManager);

    explicit SettingsCategoryListEntry(const NtftFont* font)
		: _font(font)
	{
	}

	void Initialize(UIManager& uiManager);
	void Update(UIManager&     uiManager);
	void VBlank(UIManager&     uiManager);

	void SetFont(const NtftFont* font)
	{
		_font = font;
		_nameInvalidated = true;
	}

	void SetName(const char* name);

	void SetIcon(CategoryIcon icon) { _icon = icon; }
};