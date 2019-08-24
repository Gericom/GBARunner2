#pragma once
#include "core/ListAdapter.h"

struct settings_item_t;

class SettingsItemListAdapter : public ListAdapter
{
	const NtftFont* _titleFont;
	const NtftFont* _subTitleFont;
    const settings_item_t* _items;
    int _itemCount;
protected:
	void OnBindElementHolder(ElementHolder* elementHolder, int position);

public:
	explicit SettingsItemListAdapter(const NtftFont* titleFont, const NtftFont* subTitleFont, const settings_item_t* items, int itemCount)
		: _titleFont(titleFont), _subTitleFont(subTitleFont), _items(items), _itemCount(itemCount)
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
		return 42;
	}
};
