#pragma once

#include "DirectoryEntry.h"

class File : public DirectoryEntry
{
public:
	File(u32 cluster, const char* name)
		: DirectoryEntry(cluster, false, name)
	{ }
};