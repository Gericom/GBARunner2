#include "vram.h"
#include "vramheap.h"
#include "SettingsCategoryListAdapter.h"

void SettingsCategoryListAdapter::OnBindElementHolder(ElementHolder* elementHolder, int position)
{
	SettingsCategoryListEntry* element = (SettingsCategoryListEntry*)elementHolder->GetItemElement();
	element->SetName(_items[position].name);
	element->SetIcon(_items[position].icon);
}

ElementHolder* SettingsCategoryListAdapter::CreateElementHolder()
{
	return new ElementHolder(new SettingsCategoryListEntry(_font));
}

void SettingsCategoryListAdapter::DestroyElementHolder(ElementHolder* elementHolder)
{
	delete (SettingsCategoryListEntry*)elementHolder->GetItemElement();
	delete elementHolder;
}