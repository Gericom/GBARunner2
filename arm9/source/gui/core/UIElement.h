#pragma once

#include "vram.h"
#include "LinkedList.h"
class UIManager;

class UIElement : public LinkedListEntry<UIElement>
{
public:
	virtual ~UIElement() 
	{
		
	}

	virtual void Initialize(UIManager& uiManager) = 0;
	virtual void Update(UIManager& uiManager) = 0;
	virtual void VBlank(UIManager& uiManager) = 0;
};