#include "vram.h"
#include "vramheap.h"
#include "SettingsScreen.h"
#include "IconPlayCircle_nbfc.h"
#include "IconGamepad_nbfc.h"
#include "IconInformation_nbfc.h"
#include "IconEye_nbfc.h"
#include "SingleLineIconListEntry.h"
#include "SettingsCategoryListAdapter.h"

u16 SettingsCategoryListAdapter::sPlayCircleObjAddr;
u16 SettingsCategoryListAdapter::sGamepadObjAddr;
u16 SettingsCategoryListAdapter::sInfoObjAddr;
u16 SettingsCategoryListAdapter::sEyeObjAddr;

void SettingsCategoryListAdapter::LoadCommonData(UIManager& uiManager)
{
	sPlayCircleObjAddr = uiManager.GetSubObjManager().Alloc(IconPlayCircle_nbfc_size);
	for (int i = 0; i < IconPlayCircle_nbfc_size / 2; i++)
		SPRITE_GFX_SUB[(sPlayCircleObjAddr >> 1) + i] = ((u16*)IconPlayCircle_nbfc)[i];
	sGamepadObjAddr = uiManager.GetSubObjManager().Alloc(IconGamepad_nbfc_size);
	for (int i = 0; i < IconGamepad_nbfc_size / 2; i++)
		SPRITE_GFX_SUB[(sGamepadObjAddr >> 1) + i] = ((u16*)IconGamepad_nbfc)[i];
	sInfoObjAddr = uiManager.GetSubObjManager().Alloc(IconInformation_nbfc_size);
	for (int i = 0; i < IconInformation_nbfc_size / 2; i++)
	 	SPRITE_GFX_SUB[(sInfoObjAddr >> 1) + i] = ((u16*)IconInformation_nbfc)[i];
	sEyeObjAddr = uiManager.GetSubObjManager().Alloc(IconEye_nbfc_size);
	for (int i = 0; i < IconEye_nbfc_size / 2; i++)
	 	SPRITE_GFX_SUB[(sEyeObjAddr >> 1) + i] = ((u16*)IconEye_nbfc)[i];
}

void SettingsCategoryListAdapter::OnBindElementHolder(ElementHolder* elementHolder, int position)
{
	SingleLineIconListEntry* element = (SingleLineIconListEntry*)elementHolder->GetItemElement();
	element->SetName(_items[position].name);
	switch (_items[position].icon)
	{
		case SETTINGS_CATEGORY_ICON_PLAYCIRCLE:
			element->SetIcon(sPlayCircleObjAddr);
			break;
		case SETTINGS_CATEGORY_ICON_GAMEPAD:
			element->SetIcon(sGamepadObjAddr);
			break;
		case SETTINGS_CATEGORY_ICON_INFO:
			element->SetIcon(sInfoObjAddr);
			break;
		case SETTINGS_CATEGORY_ICON_EYE:
			element->SetIcon(sEyeObjAddr);
			break;
	}
}

ElementHolder* SettingsCategoryListAdapter::CreateElementHolder()
{
	return new ElementHolder(new SingleLineIconListEntry(_font));
}

void SettingsCategoryListAdapter::DestroyElementHolder(ElementHolder* elementHolder)
{
	delete (SingleLineIconListEntry*)elementHolder->GetItemElement();
	delete elementHolder;
}