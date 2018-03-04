#pragma once

class DirectoryEntry
{
	u32 _firstCluster;
	int _isDirectory;
	char _name[256];
protected:
	DirectoryEntry(u32 cluster, int isDirectory, const char* name);

public:
	u32 GetFirstCluster() { return _firstCluster; }
	u32 GetIsDirectory() { return _isDirectory; }
	const char* GetName() { return _name; }
};