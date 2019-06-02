#pragma once

#include "UIElement.h"

class ListEntry : public UIElement
{
protected:
	s16 _offsetX;
	s16 _offsetY;
	int _selected;

public:
	ListEntry()
		: _offsetX(0), _offsetY(0), _selected(false)
	{
	}

	void GetOffset(s16& x, s16& y) const
	{
		x = _offsetX;
		y = _offsetY;
	}

	void SetOffset(s16 x, s16 y)
	{
		_offsetX = x;
		_offsetY = y;
	}

	bool GetSelected() const { return _selected; }
	void SetSelected(bool selected) { _selected = selected; }
};
