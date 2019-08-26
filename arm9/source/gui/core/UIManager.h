#pragma once

#include "LinkedList.h"
#include "UIElement.h"
#include "VramManager.h"
#include "OamManager.h"
#include "PaletteManager.h"

class UIManager
{
	VramManager           _subObjManager;
	OamManager            _subOamManager;
	PaletteManager        _subObjPalManager;
	LinkedList<UIElement> _elements;

public:
	UIManager()
	{
	}

	void AddElement(UIElement* element)
	{
		element->Initialize(*this);
		_elements.Append(element);
	}

	void RemoveElement(UIElement* element)
	{
		_elements.Remove(element);
	}

	void Update();
	void VBlank();

	VramManager&    GetSubObjManager() { return _subObjManager; }
	OamManager&     GetSubOamManager() { return _subOamManager; }
	PaletteManager& GetSubObjPalManager() { return _subObjPalManager; }
};
