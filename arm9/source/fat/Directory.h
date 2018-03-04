#pragma once

#include "DirectoryEntry.h"

class DirectoryEnumerator;

class Directory : public DirectoryEntry
{
public:
	Directory(u32 cluster, const char* name)
		: DirectoryEntry(cluster, true, name)
	{ }

	DirectoryEnumerator* GetEnumerator();
	DirectoryEntry* GetEntryByPath(const char* path);
};