#pragma once

#include "DirectoryEntry.h"

class File : public DirectoryEntry
{
	u32 _fileSize;
public:
	File(u32 cluster, const char* name, u32 fileSize)
		: DirectoryEntry(cluster, false, name), _fileSize(fileSize)
	{ }

	u32 GetFileSize() const { return _fileSize; }
};