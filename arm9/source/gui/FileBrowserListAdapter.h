#pragma once
#include "fat/ff.h"
#include "FileBrowserListEntry.h"
#include "core/ListAdapter.h"

class FileBrowserListAdapter : public ListAdapter
{
	const NtftFont* _font;
	FILINFO** _entries;
	int             _entryCount;
protected:
	void OnBindElementHolder(ElementHolder* elementHolder, int position);

public:
	explicit FileBrowserListAdapter(const NtftFont* font, FILINFO** entries, int entryCount)
		: _font(font), _entries(entries), _entryCount(entryCount)
	{
	}

	ElementHolder* CreateElementHolder();

	void DestroyElementHolder(ElementHolder* elementHolder);

	int GetItemCount()
	{
		return _entryCount;
	}

	int GetElementHeight()
	{
		return 36;
	}
};
