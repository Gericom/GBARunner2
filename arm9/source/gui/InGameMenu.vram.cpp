#include <nds/arm9/sprite.h>
#include <nds/arm9/background.h>
#include "vram.h"
#include "vramheap.h"
#include "sd_access.h"
#include "UIContext.h"
#include "FileBrowser.h"
#include "SettingsScreen.h"
#include "LinkConfigScreen.h"
#include "settings.h"
#include "../../common/fifo.h"
#include "../emu/romGpio.h"
#include "../gbaBoot.h"
#include "IconPlay_nbfc.h"
#include "IconRestart_nbfc.h"
#include "IconCable_nbfc.h"
#include "IconPower_nbfc.h"
#include "../cp15.h"
#include "InGameMenu.h"

struct igm_action_t
{
	const char* name;
};

static const igm_action_t sActions[] =
{
    { "Resume" },
    { "Reset" },
    { "Linking Settings" },
    { "Quit to rom browser" }
};

static u16 sIconObjAddrs[sizeof(sActions) / sizeof(igm_action_t)];

class IgmActionListAdapter : public ListAdapter
{
    const NtftFont* _font;
protected:
	void OnBindElementHolder(ElementHolder* elementHolder, int position)
    {
        SingleLineIconListEntry* element = (SingleLineIconListEntry*)elementHolder->GetItemElement();
        element->SetName(sActions[position].name);
        element->SetIcon(sIconObjAddrs[position]);
    }

public:
	static void LoadCommonData(UIManager& uiManager);

	explicit IgmActionListAdapter(const NtftFont* font)
		: _font(font)
	{
	}

	ElementHolder* CreateElementHolder()
    {
        return new ElementHolder(new SingleLineIconListEntry(_font));
    }

	void DestroyElementHolder(ElementHolder* elementHolder)
    {
        delete (SingleLineIconListEntry*)elementHolder->GetItemElement();
        delete elementHolder;
    }

	int GetItemCount() { return sizeof(sActions) / sizeof(igm_action_t); }

	int GetElementHeight() { return 32;	}
};


extern "C" void initialize_cache();

static u32 sOldMainDispCnt;

static void prepare()
{
    //disable display capture
    REG_DISPCAPCNT &= ~DCAP_ENABLE;

    //mute sound
    REG_SEND_FIFO = 0xAA5500FF;
    REG_SEND_FIFO = 0;

    //backup main display control
    sOldMainDispCnt = REG_DISPCNT;

    //reconfigure vram
    VRAM_C_CR = 0x80;
    VRAM_D_CR = 0x84;    
	VRAM_E_CR = 0x80;
	VRAM_F_CR = 0x80;
	VRAM_G_CR = 0x80;
    VRAM_H_CR = 0x81;
	VRAM_I_CR = 0x00;

    //copy screenshot
    u16* pDst = VRAM_C;
    if(gEmuSettingCenterMask)
    {
        for(int y = 0; y < 16; y++)
            for(int x = 0; x < 256; x++)
                *pDst++ = 0;
        for(int y = 0; y < 160; y++)
        {
            for(int x = 0; x < 8; x++)
                *pDst++ = 0;
            for(int x = 0; x < 240; x++)
                *pDst++ = SPRITE_GFX_SUB[128 * 256 + y * 256 + x];
            for(int x = 0; x < 8; x++)
                *pDst++ = 0;
        }
        for(int y = 0; y < 16; y++)
            for(int x = 0; x < 256; x++)
                *pDst++ = 0;
    }
    else
        arm9_memcpy16(VRAM_C, &SPRITE_GFX_SUB[128 * 256], 256 * 192);

    while (*((vu16*)0x04000004) & 1);
	while (!(*((vu16*)0x04000004) & 1));
    *(vu16*)0x04000304 = (*(vu16*)0x04000304) | 0x8000;
    REG_DISPCNT = 0xA0000;

    REG_MASTER_BRIGHT = gEmuSettingGbaColors ? 0x8008 : 0;
    REG_MASTER_BRIGHT_SUB = 0;
}

static void finalize()
{
    REG_MASTER_BRIGHT_SUB = 0x801F;
    while (*((vu16*)0x04000004) & 1);
	while (!(*((vu16*)0x04000004) & 1));

    u16* pDst = VRAM_C;
    if(gEmuSettingCenterMask)
    {
        pDst += 16 * 256;
        for(int y = 0; y < 160; y++)
        {
            pDst += 8;
            for(int x = 0; x < 240; x++)
                SPRITE_GFX_SUB[128 * 256 + y * 256 + x] = *pDst++;
            pDst += 8;
        }
        pDst += 16 * 256;
    }
    else
        arm9_memcpy16(&SPRITE_GFX_SUB[128 * 256], VRAM_C, 256 * 192);

    while (*((vu16*)0x04000004) & 1);
	while (!(*((vu16*)0x04000004) & 1));

    gbab_setupGfx();

    REG_DISPCNT = sOldMainDispCnt;
    
    //unmute sound and resync
    REG_SEND_FIFO = 0xAA5500FF;
    REG_SEND_FIFO = 0x7F | 0x80000000;

    vram_cd_t* vramcd_uncached = (vram_cd_t*)(((u32)vram_cd) + UNCACHED_OFFSET);
    vramcd_uncached->openMenuIrqFlag = 0;

    gbab_setupCache();
}

extern "C" bool igm_execute()
{
    prepare();

    UIContext* uiContext = new UIContext();
	uiContext->GetUIManager().Update();
	while (*((vu16*)0x04000004) & 1);
	while (!(*((vu16*)0x04000004) & 1));
	uiContext->GetUIManager().VBlank();

    bool wasQuit = false;
    bool reboot = false;

    int next = 0;
	while(true)
	{
		uiContext->ResetVram();
        if(next == 0)
		{
            if(!wasQuit)
            {
                InGameMenu* igm = new InGameMenu(uiContext);
                next = igm->Run();
                delete igm;
                if(next == 0)
                {
                    wasQuit = true;
                    reboot = true;
                    for(int i = 0; i < 256 * 192; i++)
                        VRAM_C[i] = 0;
                }
            }
            else
            {
                FileBrowser* fileBrowser = new FileBrowser(uiContext);
                next = fileBrowser->Run();
                delete fileBrowser;
            }
		}
		else if(next == 1)
		{
			SettingsScreen* settings = new SettingsScreen(uiContext);
			settings->Run();
			delete settings;
#ifndef ISNITRODEBUG
			if(!settings_save())
				uiContext->FatalError("Couldn't save settings!");
#endif
			next = 0;
		}
		else if(next == 2)
			break;
        else if(next == 3)
        {
            reboot = true;
            break;
        }
        else if(next == 4)
        {
            LinkConfigScreen* linkConfig = new LinkConfigScreen(uiContext);
			linkConfig->Run();
			delete linkConfig;
			next = 0;
        }
	}
	delete uiContext;

    finalize();

    if(reboot)
    {
        initialize_cache();
        rio_init(RIO_NONE);
    }

    return reboot;
}

int InGameMenu::Run()
{
    int next = 2;

    _uiContext->GetToolbar().SetTitle("GBARunner2");
	_uiContext->GetToolbar().SetShowBackButton(true);
	_uiContext->GetToolbar().SetShowSettingsButton(true);
    _uiContext->GetUIManager().Update();
	while (*((vu16*)0x04000004) & 1);
	while (!(*((vu16*)0x04000004) & 1));
	_uiContext->GetUIManager().VBlank();
    SingleLineIconListEntry::LoadCommonData(_uiContext->GetUIManager());

    sIconObjAddrs[0] = _uiContext->GetUIManager().GetSubObjManager().Alloc(IconPlay_nbfc_size);
	for (int i = 0; i < IconPlay_nbfc_size / 2; i++)
		SPRITE_GFX_SUB[(sIconObjAddrs[0] >> 1) + i] = ((u16*)IconPlay_nbfc)[i];

    sIconObjAddrs[1] = _uiContext->GetUIManager().GetSubObjManager().Alloc(IconRestart_nbfc_size);
	for (int i = 0; i < IconRestart_nbfc_size / 2; i++)
		SPRITE_GFX_SUB[(sIconObjAddrs[1] >> 1) + i] = ((u16*)IconRestart_nbfc)[i];

    sIconObjAddrs[2] = _uiContext->GetUIManager().GetSubObjManager().Alloc(IconCable_nbfc_size);
	for (int i = 0; i < IconCable_nbfc_size / 2; i++)
		SPRITE_GFX_SUB[(sIconObjAddrs[2] >> 1) + i] = ((u16*)IconCable_nbfc)[i];
        
    sIconObjAddrs[3] = _uiContext->GetUIManager().GetSubObjManager().Alloc(IconPower_nbfc_size);
	for (int i = 0; i < IconPower_nbfc_size / 2; i++)
		SPRITE_GFX_SUB[(sIconObjAddrs[3] >> 1) + i] = ((u16*)IconPower_nbfc)[i];

    _vramState = _uiContext->GetUIManager().GetSubObjManager().GetState();

    //clear everything except the toolbar
	_uiContext->GetUIManager().Update();
	_uiContext->GetUIManager().VBlank();

    _adapter = new IgmActionListAdapter(_uiContext->GetRobotoRegular11());
    _listRecycler = new ListRecycler(0, 36, 256, 156, 5, _adapter);
	_selectedEntry = 0;
	_listRecycler->SetSelectedIdx(0);
	_uiContext->GetUIManager().AddElement(_listRecycler);

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
            if(_selectedEntry == 0)
                next = 2;
            else if(_selectedEntry == 1)
                next = 3;
            else if(_selectedEntry == 2)
                next = 4;
            else if(_selectedEntry == 3)
                next = 0;

            while(!(*((vu16*)0x04000130) & 1));
			break;
		}
		else if (_inputRepeater.GetTriggeredKeys() & KEY_B)
		{
            next = 2;
            while(!(*((vu16*)0x04000130) & 2));
            break;
		}
		else if (_inputRepeater.GetTriggeredKeys() & KEY_R)
		{
			next = 1;
			break;
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

    return next;
}