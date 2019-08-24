#include <nds/arm9/sprite.h>
#include <nds/arm9/background.h>
#include "vram.h"
#include "vramheap.h"
#include "RobotoMedium13_ntft.h"
#include "RobotoRegular11_ntft.h"
#include "RobotoRegular9_ntft.h"
#include "BarShadow_nbfp.h"
#include "BarShadow_nbfc.h"
#include "BarShadow_nbfs.h"
#include "Dialog.h"
#include "UIContext.h"

UIContext::UIContext()
    : _robotoMedium13(new NtftFont(RobotoMedium13_ntft)),
      _robotoRegular11(new NtftFont(RobotoRegular11_ntft)),
      _robotoRegular9(new NtftFont(RobotoRegular9_ntft)),
      _toolbar(RGB5(103 >> 3, 58 >> 3, 183 >> 3), 0x7FFF, _robotoMedium13, "GBARunner2", 0x7FFF)
{
    REG_DISPCNT_SUB = DISPLAY_BG1_ACTIVE | DISPLAY_BG2_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D |
        DISPLAY_SPR_1D_SIZE_32 | MODE_0_2D;

    Toolbar::LoadCommonData(_uiManager);
    _uiManager.AddElement(&_toolbar);

    //toolbar shadow
    for (int i = 0; i < 16; i++)
        _bgPalMan.palette[i].color = ((u16*)BarShadow_nbfp)[i];
    for (int i = 0; i < (64 >> 1); i++)
        BG_GFX_SUB[i] = ((u16*)BarShadow_nbfc)[i];
    for (int i = 0; i < 2048 / 2; i++)
        BG_GFX_SUB[(0x3800 >> 1) + i] = ((u16*)BarShadow_nbfs)[i];
    REG_BG1CNT_SUB = BG_32x32 | BG_PRIORITY_2 | BG_COLOR_16 | BG_MAP_BASE(7);
    REG_BLDCNT_SUB = BLEND_ALPHA | BLEND_SRC_BG1 | BLEND_DST_BG0 | BLEND_DST_SPRITE | BLEND_DST_BACKDROP;
    REG_BLDALPHA_SUB = 4 | (12 << 8);
    _bgPalMan.palette[0].color = RGB5(31, 31, 31);

    //dialogue bg
    BG_GFX_SUB[32] = 0x1112;
    for (int i = 0; i < (64 >> 1) - 1; i++)
        BG_GFX_SUB[33 + i] = 0x1111;
    REG_BG2CNT_SUB = BG_32x32 | BG_PRIORITY_2 | BG_COLOR_16 | BG_MAP_BASE(8);
    _bgPalMan.palette[15 * 16 + 1].color = RGB5(31, 31, 31);
    _bgPalMan.palette[15 * 16 + 2].color = RGB5(157 >> 3, 157 >> 3, 157 >> 3);

    _initialVramState = _uiManager.GetSubObjManager().GetState();
}

void UIContext::FatalError(const char* error)
{
	Dialog* dialog = new Dialog(_robotoMedium13, "Error", _robotoRegular11, error);
	_uiManager.AddElement(dialog);
	_uiManager.GetSubObjPalManager().DimRows(0x3FFF);
	_bgPalMan.DimRows(0x7FFF);
	while (1)
	{
		_uiManager.Update();
		while (*((vu16*)0x04000004) & 1);
		while (!(*((vu16*)0x04000004) & 1));
		_bgPalMan.Apply(BG_PALETTE_SUB);
		_uiManager.VBlank();
	}
}