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
    //{ SETTINGS_ITEM_MODE_CHECK, "Enable autosaving", "Writes back the save to sd after a game saves", NULL, &gEmuSettingAutoSave },
	{ SETTINGS_ITEM_MODE_CHECK, "Enable DS main memory i-cache", "Boosts speed, but causes timing bugs in a few games", NULL, &gEmuSettingMainMemICache },
	{ SETTINGS_ITEM_MODE_CHECK, "Enable wram i-cache", "Boosts speed, but some games may crash", NULL, &gEmuSettingWramICache },
	{ SETTINGS_ITEM_MODE_CHECK, "Skip bios intro", "Directly boot the game without playing the intro", NULL, &gEmuSettingSkipIntro },
	{ SETTINGS_ITEM_MODE_CHECK, "Use saves directory", "Store save files in saves subdirectory (like TWiLightMenu++)", NULL, &gEmuSettingUseSavesDir },
};

static PUT_IN_VRAM settings_item_t sDisplayItems[] =
{    
	{ SETTINGS_ITEM_MODE_CHECK, "Display game on bottom screen", "", NULL, &gEmuSettingUseBottomScreen },
    { SETTINGS_ITEM_MODE_CHECK, "Enable border frame", "Enables the usage of gba border frames", NULL, &gEmuSettingFrame },
    { SETTINGS_ITEM_MODE_CHECK, "Enable center and mask", "Centers the game with a border. Adds 1 frame delay", NULL, &gEmuSettingCenterMask },
	{ SETTINGS_ITEM_MODE_CHECK, "Simulate GBA colors", "Darkens the colors to simulate a GBA screen", NULL, &gEmuSettingGbaColors }
};

static PUT_IN_VRAM const char* sDSButtonNames[] =
{
	"A Button",
	"B Button",
	"SELECT Button",
	"START Button",
	"D-Pad Right",
	"D-Pad Left",
	"D-Pad Up",
	"D-Pad Down",
	"R Button",
	"L Button",
	"X Button",
	"Y Button",
	"Screen Touch"
};

static PUT_IN_VRAM settings_item_t sInputItems[] =
{    
	{ SETTINGS_ITEM_MODE_SIMPLE_ENUM, "GBA A Button", NULL, sDSButtonNames, &gInputSettings.buttonA },
    { SETTINGS_ITEM_MODE_SIMPLE_ENUM, "GBA B Button", NULL, sDSButtonNames, &gInputSettings.buttonB },
    { SETTINGS_ITEM_MODE_SIMPLE_ENUM, "GBA L Button", NULL, sDSButtonNames, &gInputSettings.buttonL },
    { SETTINGS_ITEM_MODE_SIMPLE_ENUM, "GBA R Button", NULL, sDSButtonNames, &gInputSettings.buttonR },
    { SETTINGS_ITEM_MODE_SIMPLE_ENUM, "GBA START Button", NULL, sDSButtonNames, &gInputSettings.buttonStart },
    { SETTINGS_ITEM_MODE_SIMPLE_ENUM, "GBA SELECT Button", NULL, sDSButtonNames, &gInputSettings.buttonSelect }
};

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
	{ SETTINGS_ITEM_MODE_SIMPLE, "Commit date", STRINGIFY(GIT_COMMIT_DATE), NULL, NULL },
	{ SETTINGS_ITEM_MODE_SIMPLE, "Commit hash", STRINGIFY(GIT_COMMIT_HASH), NULL, NULL },
    { SETTINGS_ITEM_MODE_SIMPLE, "Branch", STRINGIFY(GIT_BRANCH), NULL, NULL },
    { SETTINGS_ITEM_MODE_SIMPLE, "DLDI Cpu", 
#ifdef ARM7_DLDI
		"ARM 7"
#else
		"ARM 9"
#endif
		, NULL, NULL },
	{ SETTINGS_ITEM_MODE_SIMPLE, "Version", 
#if defined(USE_DSI_16MB)
		"DSi"
#elif defined(USE_3DS_32MB)
		"3DS"
#else
		"DS"
#endif
		, NULL, NULL }
};

static const settings_category_t sCategories[] =
{
    { "\"Emulation\" Settings", SettingsCategoryListAdapter::SETTINGS_CATEGORY_ICON_PLAYCIRCLE, sizeof(sEmulationItems) / sizeof(settings_item_t), sEmulationItems, NULL },
	{ "Display Settings", SettingsCategoryListAdapter::SETTINGS_CATEGORY_ICON_EYE, sizeof(sDisplayItems) / sizeof(settings_item_t), sDisplayItems, NULL },
    { "Input Settings", SettingsCategoryListAdapter::SETTINGS_CATEGORY_ICON_GAMEPAD, sizeof(sInputItems) / sizeof(settings_item_t), sInputItems, SettingsScreen::ButtonMapCallback },
    { "About GBARunner2", SettingsCategoryListAdapter::SETTINGS_CATEGORY_ICON_INFO, sizeof(sInfoItems) / sizeof(settings_item_t), sInfoItems, NULL }
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

void SettingsScreen::ButtonMapCallback()
{
	UIManager& uiManager = _uiContext->GetUIManager();
	u16 vramState = uiManager.GetSubObjManager().GetState();
	Dialog* dialog = new Dialog(
		_uiContext->GetRobotoMedium13(), _selectedCategory->items[_selectedEntry].title,
		_uiContext->GetRobotoRegular11(), "Press the button to map\nwithin 4 seconds ...");
	uiManager.AddElement(dialog);
	_uiContext->GetUIManager().VBlank();
	uiManager.GetSubObjPalManager().DimRows(0x3FFF);
	_uiContext->GetBGPalManager().DimRows(0x7FFF);
	int frameCounter = 0;
	while (1)
	{
		_inputRepeater.Update(~keys_read());
		if(_inputRepeater.GetTriggeredKeys())
		{
			u32 key = 31 - __builtin_clz(_inputRepeater.GetTriggeredKeys());
			//dpad, touch and lid
			if (key == 4 || key == 5 || key == 6 || key == 7 || key == 12 || key == 13)
				break;
			if(_selectedCategory->items[_selectedEntry].pValue)
				*_selectedCategory->items[_selectedEntry].pValue = 31 - __builtin_clz(_inputRepeater.GetTriggeredKeys());
			break;
		}
		uiManager.Update();
		while (*((vu16*)0x04000004) & 1);
		while (!(*((vu16*)0x04000004) & 1));
		_uiContext->GetBGPalManager().Apply(BG_PALETTE_SUB);
		uiManager.VBlank();
		REG_DISPCNT_SUB |= DISPLAY_BG2_ACTIVE;
		frameCounter++;
		if(frameCounter == 4 * 60)
			break;
	}
	uiManager.RemoveElement(dialog);
	delete dialog;
	uiManager.GetSubObjPalManager().UndimRows(0x3FFF);
	_uiContext->GetBGPalManager().UndimRows(0x7FFF);
	uiManager.GetSubObjManager().SetState(vramState);
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
		_inputRepeater.Update(~keys_read());
		if (_inputRepeater.GetTriggeredKeys() & KEY_UP)
		{
			if (_selectedEntry > 0)
				_selectedEntry--;
		}
		else if (_inputRepeater.GetTriggeredKeys() & KEY_DOWN)
		{
			if (_selectedEntry < _adapter->GetItemCount() - 1)
				_selectedEntry++;
		}
		else if (_inputRepeater.GetTriggeredKeys() & KEY_A)
		{
			if(_selectedCategory)
			{
				if(_selectedCategory->activateCallback)
					_selectedCategory->activateCallback(this);
				else
				{
					//default callback
					if(_selectedCategory->items[_selectedEntry].mode == SETTINGS_ITEM_MODE_CHECK)
						*_selectedCategory->items[_selectedEntry].pValue = !*_selectedCategory->items[_selectedEntry].pValue;
				}
			}
			else
				GotoCategory(&sCategories[_selectedEntry]);
		}
		else if (_inputRepeater.GetTriggeredKeys() & KEY_B)
		{
			if(_selectedCategory)
				GotoCategory(NULL);
			else
			{
				while(!(*((vu16*)0x04000130) & KEY_B));
				break;
			}
		}
		_listRecycler->SetSelectedIdx(_selectedEntry);
		_uiContext->GetUIManager().Update();
		while (*((vu16*)0x04000004) & 1);
		while (!(*((vu16*)0x04000004) & 1));
		_uiContext->GetBGPalManager().Apply(BG_PALETTE_SUB);
		_uiContext->GetUIManager().VBlank();
		REG_DISPCNT_SUB &= ~DISPLAY_BG2_ACTIVE;
	}
	if (_listRecycler)
		_uiContext->GetUIManager().RemoveElement(_listRecycler);
	
	//clear everything except the toolbar
	_uiContext->GetUIManager().Update();
	_uiContext->GetUIManager().VBlank();
	REG_DISPCNT_SUB &= ~DISPLAY_BG2_ACTIVE;
}