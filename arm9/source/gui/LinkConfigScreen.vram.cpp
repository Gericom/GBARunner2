#include <nds/arm9/sprite.h>
#include <nds/arm9/background.h>
#include "sd_access.h"
#include "vram.h"
#include "vramheap.h"
#include "../../common/fifo.h"
#include "vector.h"
#include "gui/core/UIManager.h"
#include "gui/Toolbar.h"
#include "SettingsScreen.h"
#include "Dialog.h"
#include "core/ListRecycler.h"
#include "core/InputRepeater.h"
#include "SettingsCategoryListAdapter.h"
#include "SettingsItemListAdapter.h"
#include "SettingsItemListEntry.h"
#include "SingleLineIconListEntry.h"
#include "UIContext.h"
#include "LinkConfigScreen.h"

static u32 sLinkingEnabled = false;

void LinkConfigScreen::Run()
{
    _uiContext->GetToolbar().SetTitle("Linking Settings");
	_uiContext->GetToolbar().SetShowBackButton(true);
	_uiContext->GetToolbar().SetShowSettingsButton(false);
	_uiContext->GetUIManager().Update();
	while (*((vu16*)0x04000004) & 1);
	while (!(*((vu16*)0x04000004) & 1));
	_uiContext->GetUIManager().VBlank();
	SettingsCategoryListAdapter::LoadCommonData(_uiContext->GetUIManager());
	SettingsItemListEntry::LoadCommonData(_uiContext->GetUIManager());
    _vramState = _uiContext->GetUIManager().GetSubObjManager().GetState();
    SettingsItemListEntry* enableWifiBox = new SettingsItemListEntry(_uiContext->GetRobotoRegular11(), _uiContext->GetRobotoRegular9());
    settings_item_t enableWifiSetting =
    {
        SETTINGS_ITEM_MODE_CHECK,
        "Enable wifi linking",
        NULL,
        NULL,
        &sLinkingEnabled
    };
    enableWifiBox->SetOffset(0, 36 + 5);
    enableWifiBox->SetSettingsItem(&enableWifiSetting);
    enableWifiBox->SetSelected(true);
    _uiContext->GetUIManager().AddElement(enableWifiBox);    
    while(true)
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
			sLinkingEnabled = !sLinkingEnabled;
            if(sLinkingEnabled)
                REG_SEND_FIFO = 0xAA560000;
            else
                REG_SEND_FIFO = 0xAA560001;
		}
		else if (_inputRepeater.GetTriggeredKeys() & KEY_B)
		{
			while(!(*((vu16*)0x04000130) & KEY_B));
			break;
		}
		//_listRecycler->SetSelectedIdx(_selectedEntry);
		_uiContext->GetUIManager().Update();
		while (*((vu16*)0x04000004) & 1);
		while (!(*((vu16*)0x04000004) & 1));
		_uiContext->GetBGPalManager().Apply(BG_PALETTE_SUB);
		_uiContext->GetUIManager().VBlank();
    }
    _uiContext->GetUIManager().RemoveElement(enableWifiBox);    
    delete enableWifiBox;
    //clear everything except the toolbar
	_uiContext->GetUIManager().Update();
	_uiContext->GetUIManager().VBlank();
}