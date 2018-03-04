#include "vram.h"
#include "vramheap.h"
#include "sd_access.h"
#include "fat.h"
#include "Directory.h"
#include "File.h"
#include "DirectoryEnumerator.h"

PUT_IN_VRAM DirectoryEnumerator::DirectoryEnumerator(Directory* directory)
	: _curCluster(directory->GetFirstCluster()), _offset(0)
{
	_dirBuf = (dir_entry_t*)vramheap_alloc(vram_cd->sd_info.nr_sectors_per_cluster * 512);
}

PUT_IN_VRAM DirectoryEnumerator::~DirectoryEnumerator()
{
	vramheap_free(_dirBuf);
}

PUT_IN_VRAM DirectoryEntry* DirectoryEnumerator::GetNext()
{
	if (_offset < 0)
		return NULL;

	bool found_long_name = false;
	uint8_t name_buffer[256];
	for (int i = 0; i < 256; i++)
		name_buffer[i] = 0;
	int i = _offset;
	while (true)
	{		
		if (i == 0)
		{
			u32 sec = get_sector_from_cluster(_curCluster);
			read_sd_sectors_safe(sec, vram_cd->sd_info.nr_sectors_per_cluster, _dirBuf);
		}
		for (; i < vram_cd->sd_info.nr_sectors_per_cluster * 512 / 32; i++)
		{
			dir_entry_t* cur_dir_entry = &_dirBuf[i];

			if (cur_dir_entry->regular_entry.record_type == 0xE5)
			{
				//erased
			}
			else if ((cur_dir_entry->attrib & DIR_ATTRIB_LONG_FILENAME) == DIR_ATTRIB_LONG_FILENAME)
			{
				//construct name				
				int name_part_order = cur_dir_entry->long_name_entry.order & ~0x40;
				if (name_part_order > 0 && name_part_order <= 20)
				{
					store_long_name_part(name_buffer, cur_dir_entry, (name_part_order - 1) * 13);
					if (name_part_order == 1)
						found_long_name = true;
				}
			}
			else if (cur_dir_entry->attrib & (DIR_ATTRIB_VOLUME_ID | DIR_ATTRIB_HIDDEN | DIR_ATTRIB_SYSTEM))
			{
				//skip VOLUME_ID, HIDDEN or SYSTEM entry
				for (int j = 0; j < 256 / 2; j++)
					*(uint16_t*)(name_buffer + j * 2) = 0x0000;
				continue;
			}
			else if (cur_dir_entry->regular_entry.record_type == 0)
			{
				_offset = -1;
				return NULL;
			}
			else
			{
				int len = 0;
				if (!found_long_name)
				{
					for (int j = 0; j < 8; j++)
					{
						name_buffer[j] = cur_dir_entry->regular_entry.short_name[j];
						if (name_buffer[j] != ' ')
							len = j + 1;
					}
					if (cur_dir_entry->regular_entry.short_name[8] != ' ')
					{
						name_buffer[len++] = '.';
						for (int j = 0; j < 3; j++)
						{
							if (cur_dir_entry->regular_entry.short_name[8 + j] == ' ')
								break;
							name_buffer[len++] = cur_dir_entry->regular_entry.short_name[8 + j];
						}
					}
					name_buffer[len] = '\0';
				}
				name_buffer[255] = 0;

				_offset = i + 1;

				u32 clusterNr = cur_dir_entry->regular_entry.cluster_nr_bottom | (cur_dir_entry->regular_entry.cluster_nr_top << 16);
				if ((cur_dir_entry->attrib & DIR_ATTRIB_DIRECTORY) == DIR_ATTRIB_DIRECTORY)
					return new Directory(clusterNr, (char*)name_buffer);
				else
					return new File(clusterNr, (char*)name_buffer);
			}
		}
		//follow the chain
		uint32_t next = get_cluster_fat_value_simple(_curCluster);
		if (next >= 0x0FFFFFF8)
		{
			_offset = -1;
			return NULL;
		}
		_curCluster = next;
		i = 0;
	}
	_offset = -1;
	return NULL;
}