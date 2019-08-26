#include "vram.h"
#include "vramheap.h"
#include "FileBrowserListAdapter.h"

void FileBrowserListAdapter::OnBindElementHolder(ElementHolder* elementHolder, int position)
{
	FileBrowserListEntry* element = (FileBrowserListEntry*)elementHolder->GetItemElement();
	element->SetName(_entries[position]->fname);
	if (_entries[position]->fattrib & AM_DIR)
		element->SetEntryType(FileBrowserListEntry::FILE_BROWSER_ENTRY_TYPE_FOLDER);
	else
		element->SetEntryType(FileBrowserListEntry::FILE_BROWSER_ENTRY_TYPE_GAME);
}

ElementHolder* FileBrowserListAdapter::CreateElementHolder()
{
	return new ElementHolder(new FileBrowserListEntry(_font));
}

void FileBrowserListAdapter::DestroyElementHolder(ElementHolder* elementHolder)
{
	delete (FileBrowserListEntry*)elementHolder->GetItemElement();
	delete elementHolder;
}
