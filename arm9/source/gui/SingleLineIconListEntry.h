#pragma once

#include "core/ListEntry.h"
#include "core/NtftFont.h"

class SingleLineIconListEntry : public ListEntry
{
private:
	const NtftFont* _font;
	//CategoryIcon _icon;
	u16 _iconObjAddr;
	u16       _nameObjAddr;
	char      _name[128];
	u16      _nameInvalidated;
public:	
	static void LoadCommonData(UIManager& uiManager);

    explicit SingleLineIconListEntry(const NtftFont* font)
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

	void SetIcon(u16 iconObjAddr) { _iconObjAddr = iconObjAddr; }
	//void SetIcon(CategoryIcon icon) { _icon = icon; }
};