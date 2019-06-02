#pragma once
#include "core/UIElement.h"
#include "core/NtftFont.h"
#include "uiutil.h"

class Dialog : public UIElement
{
	int _x;
	int _y;
	int _width;
	int _height;

	XBGR1555 _bgColor;

	const NtftFont* _titleFont;
	char            _titleText[64];
	XBGR1555        _titleTextColor;
	u16             _titleTextObjAddr;

	const NtftFont* _bodyFont;
	char            _bodyText[256];
	XBGR1555        _bodyTextColor;
	u16             _bodyTextObjAddr;

	union
	{
		struct
		{
			vu32 colorInvalidated : 1;
			vu32 titleInvalidated : 1;
			vu32 bodyInvalidated : 1;
			vu32 : 29;
		};

		vu32 flags;
	}        _flags;

public:
	Dialog(const NtftFont* titleFont, const char* titleText, const NtftFont* bodyFont, const char* bodyText)
	{
		int w = 182;//default dialog width
		int btw, bth;
		bodyFont->MeasureString(bodyText, btw, bth);
		int h = 42 + 18 + bth - 5;
		_width = (w + 7) & ~7;
		_height = (h + 7) & ~7;
		_x = 128 - (_width >> 1);
		_y = 96 - (_height >> 1);
		SetBGColor(0x7FFF);
		SetTitleFont(titleFont);
		SetTitleText(titleText);
		SetTitleTextColor(RGB5(4, 4, 4));
		SetBodyFont(bodyFont);
		SetBodyText(bodyText);
		SetBodyTextColor(RGB5(12, 12, 12));
	}

	Dialog(int             width, int             height, u16    bgColor,
	         const NtftFont* titleFont, const char* titleText, u16 titleTextColor,
	         const NtftFont* bodyFont, const char*  bodyText, u16  bodyTextColor)
		: _x(128 - (width >> 1)), _y(96 - (height >> 1)), _width(width), _height(height)
	{
		SetBGColor(bgColor);
		SetTitleFont(titleFont);
		SetTitleText(titleText);
		SetTitleTextColor(titleTextColor);
		SetBodyFont(bodyFont);
		SetBodyText(bodyText);
		SetBodyTextColor(bodyTextColor);
	}

	void Initialize(UIManager& uiManager);
	void Update(UIManager& uiManager);
	void VBlank(UIManager& uiManager);

	void SetBGColor(u16 color)
	{
		_bgColor.color = color;
		_flags.colorInvalidated = 1;
	}

	void SetTitleFont(const NtftFont* font)
	{
		_titleFont = font;
		_flags.titleInvalidated = true;
	}

	void SetTitleText(const char* titleText);

	void SetTitleTextColor(u16 color)
	{
		_titleTextColor.color = color;
		_flags.colorInvalidated = true;
	}

	void SetBodyFont(const NtftFont* font)
	{
		_bodyFont = font;
		_flags.bodyInvalidated = true;
	}

	void SetBodyText(const char* bodyText);

	void SetBodyTextColor(u16 color)
	{
		_bodyTextColor.color = color;
		_flags.colorInvalidated = 1;
	}
};
