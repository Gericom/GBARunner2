#include "vram.h"
#include "vramheap.h"
#include "SettingsScreen.h"
#include "SettingsItemListAdapter.h"

void SettingsItemListAdapter::OnBindElementHolder(ElementHolder* elementHolder, int position)
{
	SettingsItemListEntry* element = (SettingsItemListEntry*)elementHolder->GetItemElement();
	element->SetSettingsItem(&_items[position]);
}

ElementHolder* SettingsItemListAdapter::CreateElementHolder()
{
	return new ElementHolder(new SettingsItemListEntry(_titleFont, _subTitleFont));
}

void SettingsItemListAdapter::DestroyElementHolder(ElementHolder* elementHolder)
{
	delete (SettingsItemListEntry*)elementHolder->GetItemElement();
	delete elementHolder;
}