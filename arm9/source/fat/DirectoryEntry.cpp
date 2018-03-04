#include "vram.h"
#include "vramheap.h"
#include "sd_access.h"
#include "DirectoryEntry.h"

PUT_IN_VRAM DirectoryEntry::DirectoryEntry(u32 cluster, int isDirectory, const char* name)
	: _firstCluster(cluster), _isDirectory(isDirectory)
{
	for (int i = 0; i < 256; i++)
	{
		char c = name[i];
		MI_WriteByte(&_name[i], c);
		if (c == 0)
			break;
	}
}