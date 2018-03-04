#pragma once

class DirectoryEntry
{
	u32 _firstCluster;
	int _isDirectory;
	char _name[256];
protected:
	DirectoryEntry(u32 cluster, int isDirectory, const char* name);

public:
	u32 GetFirstCluster() const { return _firstCluster; }
	u32 GetIsDirectory() const { return _isDirectory; }
	const char* GetName() const { return _name; }
};