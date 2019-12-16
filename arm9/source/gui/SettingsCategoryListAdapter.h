#pragma once
#include "core/ListAdapter.h"

struct settings_category_t;

class SettingsCategoryListAdapter : public ListAdapter
{
public:
	enum CategoryIcon : u16
	{
		SETTINGS_CATEGORY_ICON_PLAYCIRCLE,
		SETTINGS_CATEGORY_ICON_GAMEPAD,
		SETTINGS_CATEGORY_ICON_INFO,
		SETTINGS_CATEGORY_ICON_EYE
	};
private:
	static u16 sPlayCircleObjAddr;
	static u16 sGamepadObjAddr;
	static u16 sInfoObjAddr;
	static u16 sEyeObjAddr;

	const NtftFont* _font;
	const settings_category_t* _items;
    int _itemCount;
protected:
	void OnBindElementHolder(ElementHolder* elementHolder, int position);

public:
	static void LoadCommonData(UIManager& uiManager);

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
