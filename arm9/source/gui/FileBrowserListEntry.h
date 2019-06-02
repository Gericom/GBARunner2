#pragma once
#include "core/UIElement.h"
#include "core/ListEntry.h"

class UIManager;
class NtftFont;

class FileBrowserListEntry : public ListEntry
{
	static u16 sCircleObjAddr;
	static u16 sFolderObjAddr;
	static u16 sGameObjAddr;

public:
	enum EntryType : u16
	{
		FILE_BROWSER_ENTRY_TYPE_FOLDER,
		FILE_BROWSER_ENTRY_TYPE_GAME
	};

private:
	const NtftFont* _font;
	EntryType _entryType;
	u16       _nameObjAddr;
	char      _name[128];
	u16      _nameInvalidated;
public:
	static void LoadCommonData(UIManager& uiManager);

	explicit FileBrowserListEntry(const NtftFont* font)
		: _font(font)
	{
	}

	void Initialize(UIManager& uiManager);
	void Update(UIManager&     uiManager);
	void VBlank(UIManager&     uiManager);

	EntryType GetEntryType() const { return _entryType; }
	void      SetEntryType(EntryType entryType) { _entryType = entryType; }

	void SetFont(const NtftFont* font)
	{
		_font = font;
		_nameInvalidated = true;
	}

	void SetName(const char* name);
};
