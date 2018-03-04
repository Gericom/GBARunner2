#pragma once

#include "vram.h"
#include "../../../common/sd_vram.h"

class DirectoryEntry;
class Directory;

class DirectoryEnumerator
{
	u32 _curCluster;
	dir_entry_t* _dirBuf;
	int _offset;
public:
	explicit DirectoryEnumerator(const Directory* dir);
	~DirectoryEnumerator();

	DirectoryEntry* GetNext();
};