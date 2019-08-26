#include <nds/arm9/sprite.h>
#include "vram.h"
#include "sd_access.h"
#include "UIElement.h"
#include "UIManager.h"

void UIManager::Update()
{
	_subOamManager.Clear();
	UIElement* element = NULL;
	while ((element = _elements.GetNext(element)) != NULL)
		element->Update(*this);
}

void UIManager::VBlank()
{
	_subOamManager.Apply(OAM_SUB);
	_subObjPalManager.Apply(SPRITE_PALETTE_SUB);
	UIElement* element = NULL;
	while ((element = _elements.GetNext(element)) != NULL)
		element->VBlank(*this);
}
