#pragma once
#include "FileBrowserListEntry.h"
#include "core/ListAdapter.h"

class FileBrowserListAdapter : public ListAdapter
{
	const NtftFont* _font;
	FILINFO** _entries;
	int             _entryCount;
protected:
	void OnBindElementHolder(ElementHolder* elementHolder, int position)
	{
		FileBrowserListEntry* element = (FileBrowserListEntry*)elementHolder->GetItemElement();
		element->SetName(_entries[position]->fname);
		if (_entries[position]->fattrib & AM_DIR)
			element->SetEntryType(FileBrowserListEntry::FILE_BROWSER_ENTRY_TYPE_FOLDER);
		else
			element->SetEntryType(FileBrowserListEntry::FILE_BROWSER_ENTRY_TYPE_GAME);
	}

public:
	explicit FileBrowserListAdapter(const NtftFont* font, FILINFO** entries, int entryCount)
		: _font(font), _entries(entries), _entryCount(entryCount)
	{
	}

	ElementHolder* CreateElementHolder()
	{
		return new ElementHolder(new FileBrowserListEntry(_font));
	}

	void DestroyElementHolder(ElementHolder* elementHolder)
	{
		delete (FileBrowserListEntry*)elementHolder->GetItemElement();
		delete elementHolder;
	}

	int GetItemCount()
	{
		return _entryCount;
	}

	int GetElementHeight()
	{
		return 36;
	}
};
