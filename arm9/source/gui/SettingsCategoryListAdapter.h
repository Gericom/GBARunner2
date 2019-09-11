#pragma once
#include "SettingsCategoryListEntry.h"
#include "SettingsScreen.h"
#include "core/ListAdapter.h"

class SettingsCategoryListAdapter : public ListAdapter
{
	const NtftFont* _font;
	const settings_category_t* _items;
    int _itemCount;
protected:
	void OnBindElementHolder(ElementHolder* elementHolder, int position);

public:
	explicit SettingsCategoryListAdapter(const NtftFont* font, const settings_category_t* items, int itemCount)
		: _font(font), _items(items), _itemCount(itemCount)
	{
	}

	ElementHolder* CreateElementHolder();

	void DestroyElementHolder(ElementHolder* elementHolder);

	int GetItemCount()
	{
		return _itemCount;
	}

	int GetElementHeight()
	{
		return 32;
	}
};
