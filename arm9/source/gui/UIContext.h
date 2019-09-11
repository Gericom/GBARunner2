#pragma once
#include "gui/core/NtftFont.h"
#include "core/PaletteManager.h"
#include "core/UIManager.h"
#include "Toolbar.h"

class UIContext
{
    PaletteManager _bgPalMan;
	UIManager      _uiManager;

    u16 _initialVramState;    

    NtftFont* _robotoMedium13;
	NtftFont* _robotoRegular11;
	NtftFont* _robotoRegular9;

    Toolbar _toolbar;

public:
    UIContext();        

    ~UIContext()
    {        
        _uiManager.GetSubOamManager().Clear();
        _uiManager.GetSubOamManager().Apply(OAM_SUB);
        REG_BLDCNT_SUB = 0;
        REG_BLDALPHA_SUB = 0;
        delete _robotoRegular9;
        delete _robotoRegular11;
        delete _robotoMedium13;
    }

    void FatalError(const char* error);

    PaletteManager& GetBGPalManager() { return _bgPalMan; }
    UIManager& GetUIManager() { return _uiManager; }

    Toolbar& GetToolbar() { return _toolbar; }

    const NtftFont* GetRobotoMedium13() const { return _robotoMedium13; }
    const NtftFont* GetRobotoRegular11() const { return _robotoRegular11; }
    const NtftFont* GetRobotoRegular9() const { return _robotoRegular9; }

    void ResetVram()
    {
        _uiManager.GetSubObjManager().SetState(_initialVramState);
    }
};