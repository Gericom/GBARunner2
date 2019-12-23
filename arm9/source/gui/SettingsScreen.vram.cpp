#include <nds/arm9/sprite.h>
#include <nds/arm9/background.h>
#include "sd_access.h"
#include "vram.h"
#include "vramheap.h"
#include "vector.h"
#include "gui/core/UIManager.h"
#include "gui/Toolbar.h"
#include "Dialog.h"
#include "core/ListRecycler.h"
#include "core/InputRepeater.h"
#include "SettingsCategoryListAdapter.h"
#include "SettingsItemListAdapter.h"
#include "SettingsItemListEntry.h"
#include "SingleLineIconListEntry.h"
#include "UIContext.h"
#include "settings.h"
#include "SettingsScreen.h"

#define STRINGIFY2(x)	#x
#define STRINGIFY(x)	STRINGIFY2(x)

static PUT_IN_VRAM settings_item_t sEmulationItems[] =
{    
    //{ SETTINGS_ITEM_MODE_CHECK, "Enable autosaving", "Writes back the save to sd after a game saves", &gEmuSettingAutoSave },
	{ SETTINGS_ITEM_MODE_CHECK, "Enable DS main memory i-cache", "Boosts speed, but causes timing bugs in a few games", &gEmuSettingMainMemICache },    
	{ SETTINGS_ITEM_MODE_CHECK, "Enable wram i-cache", "Boosts speed, but some games may crash", &gEmuSettingWramICache },
    { SETTINGS_ITEM_MODE_CHECK, "Skip bios intro", "Directly boot the game without playing the intro", &gEmuSettingSkipIntro }
};

static PUT_IN_VRAM settings_item_t sDisplayItems[] =
{    
	{ SETTINGS_ITEM_MODE_CHECK, "Display game on bottom screen", "", &gEmuSettingUseBottomScreen },
    { SETTINGS_ITEM_MODE_CHECK, "Enable border frame", "Enables the usage of gba border frames", &gEmuSettingFrame },
    { SETTINGS_ITEM_MODE_CHECK, "Enable center and mask", "Centers the game with a border. Adds 1 frame delay", &gEmuSettingCenterMask },
	{ SETTINGS_ITEM_MODE_CHECK, "Simulate GBA colors", "Darkens the colors to simulate a GBA screen", &gEmuSettingGbaColors }
};

// static PUT_IN_VRAM settings_item_t sInputItems[] =
// {    
// 	{ SETTINGS_ITEM_MODE_SIMPLE, "GBA A button", "DS A button", NULL },
//     { SETTINGS_ITEM_MODE_SIMPLE, "GBA B button", "DS B button", NULL },
//     { SETTINGS_ITEM_MODE_SIMPLE, "GBA L button", "DS L button", NULL },
//     { SETTINGS_ITEM_MODE_SIMPLE, "GBA R button", "DS R button", NULL },
//     { SETTINGS_ITEM_MODE_SIMPLE, "GBA START button", "DS START button", NULL },
//     { SETTINGS_ITEM_MODE_SIMPLE, "GBA SELECT button", "DS SELECT button", NULL }
// };

#ifndef GIT_COMMIT_DATE
#define GIT_COMMIT_DATE unavailable
#endif
#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH unavailable
#endif
#ifndef GIT_BRANCH
#define GIT_BRANCH unavailable
#endif

static PUT_IN_VRAM settings_item_t sInfoItems[] =
{    
	{ SETTINGS_ITEM_MODE_SIMPLE, "Commit date", STRINGIFY(GIT_COMMIT_DATE), NULL },
	{ SETTINGS_ITEM_MODE_SIMPLE, "Commit hash", STRINGIFY(GIT_COMMIT_HASH), NULL },
    { SETTINGS_ITEM_MODE_SIMPLE, "Branch", STRINGIFY(GIT_BRANCH), NULL },
    { SETTINGS_ITEM_MODE_SIMPLE, "DLDI Cpu", 
#ifdef ARM7_DLDI
		"ARM 7"
#else
		"ARM 9"
#endif
		, NULL },
	{ SETTINGS_ITEM_MODE_SIMPLE, "Version", 
#if defined(USE_DSI_16MB)
		"DSi"
#elif defined(USE_3DS_32MB)
		"3DS"
#else
		"DS"
#endif
		, NULL }
};

static const settings_category_t sCategories[] =
{
    { "\"Emulation\" Settings", SettingsCategoryListAdapter::SETTINGS_CATEGORY_ICON_PLAYCIRCLE, sizeof(sEmulationItems) / sizeof(settings_item_t), sEmulationItems },
	{ "Display Settings", SettingsCategoryListAdapter::SETTINGS_CATEGORY_ICON_EYE, sizeof(sDisplayItems) / sizeof(settings_item_t), sDisplayItems },
    //{ "Input Settings", SettingsCategoryListAdapter::SETTINGS_CATEGORY_ICON_GAMEPAD, sizeof(sInputItems) / sizeof(settings_item_t), sInputItems },
    { "About GBARunner2", SettingsCategoryListAdapter::SETTINGS_CATEGORY_ICON_INFO, sizeof(sInfoItems) / sizeof(settings_item_t), sInfoItems }
};

void SettingsScreen::GotoCategory(const settings_category_t* category)
{
	if (_listRecycler)
	{
		_uiContext->GetUIManager().RemoveElement(_listRecycler);
		delete _listRecycler;
	}
	if (_adapter)
		delete _adapter;
	_uiContext->GetUIManager().GetSubObjManager().SetState(_vramState);

	//clear everything except the toolbar
	_uiContext->GetUIManager().Update();
	_uiContext->GetUIManager().VBlank();

	_selectedCategory = category;
	if(_selectedCategory)
	{
		_adapter = new SettingsItemListAdapter(_uiContext->GetRobotoRegular11(), _uiContext->GetRobotoRegular9(), _selectedCategory->items, _selectedCategory->itemCount);
		_listRecycler = new ListRecycler(0, 36, 256, 156, 5, _adapter);
		_selectedEntry = 0;
		_listRecycler->SetSelectedIdx(0);
		_uiContext->GetUIManager().AddElement(_listRecycler);
		_uiContext->GetToolbar().SetTitle(_selectedCategory->name);
	}
	else
	{
		_adapter = new SettingsCategoryListAdapter(_uiContext->GetRobotoRegular11(), sCategories, sizeof(sCategories) / sizeof(settings_category_t));
		_listRecycler = new ListRecycler(0, 36, 256, 156, 5, _adapter);
		_selectedEntry = 0;
		_listRecycler->SetSelectedIdx(0);
		_uiContext->GetUIManager().AddElement(_listRecycler);
		_uiContext->GetToolbar().SetTitle("Settings");
	}
	//load the text graphics for next menu
	_uiContext->GetUIManager().VBlank();
}

void SettingsScreen::Run()
{
	_uiContext->GetToolbar().SetTitle("Settings");
	_uiContext->GetToolbar().SetShowBackButton(true);
	_uiContext->GetToolbar().SetShowSettingsButton(false);
	_uiContext->GetUIManager().Update();
	while (*((vu16*)0x04000004) & 1);
	while (!(*((vu16*)0x04000004) & 1));
	_uiContext->GetUIManager().VBlank();
	SettingsCategoryListAdapter::LoadCommonData(_uiContext->GetUIManager());
	SettingsItemListEntry::LoadCommonData(_uiContext->GetUIManager());
	SingleLineIconListEntry::LoadCommonData(_uiContext->GetUIManager());
	_vramState = _uiContext->GetUIManager().GetSubObjManager().GetState();	
    GotoCategory(NULL);
	while (1)
	{
		_inputRepeater.Update(~*((vu16*)0x04000130));
		if (_inputRepeater.GetTriggeredKeys() & (1 << 6))
		{
			if (_selectedEntry > 0)
				_selectedEntry--;
		}
		else if (_inputRepeater.GetTriggeredKeys() & (1 << 7))
		{
			if (_selectedEntry < _adapter->GetItemCount() - 1)
				_selectedEntry++;
		}
		else if (_inputRepeater.GetTriggeredKeys() & 1)
		{
			if(_selectedCategory)
			{
				if(_selectedCategory->items[_selectedEntry].mode == SETTINGS_ITEM_MODE_CHECK)
					*_selectedCategory->items[_selectedEntry].pValue = !*_selectedCategory->items[_selectedEntry].pValue;
			}
			else
				GotoCategory(&sCategories[_selectedEntry]);
		}
		else if (_inputRepeater.GetTriggeredKeys() & 2)
		{
			if(_selectedCategory)
				GotoCategory(NULL);
			else
			{
				while(!(*((vu16*)0x04000130) & 2));
				break;
			}
		}
		_listRecycler->SetSelectedIdx(_selectedEntry);
		_uiContext->GetUIManager().Update();
		while (*((vu16*)0x04000004) & 1);
		while (!(*((vu16*)0x04000004) & 1));
		_uiContext->GetBGPalManager().Apply(BG_PALETTE_SUB);
		_uiContext->GetUIManager().VBlank();
	}
	if (_listRecycler)
		_uiContext->GetUIManager().RemoveElement(_listRecycler);
	
	//clear everything except the toolbar
	_uiContext->GetUIManager().Update();
	_uiContext->GetUIManager().VBlank();
}