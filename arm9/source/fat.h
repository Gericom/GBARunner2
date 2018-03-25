#ifndef __FAT_H__
#define __FAT_H__

typedef enum
{
	SHORT_NAME = 0,
	LONG_NAME = 1
} SEARCH_TYPE;

uint32_t get_entrys_first_cluster(dir_entry_t* dir_entry);
void store_long_name_part(uint8_t* buffer, dir_entry_t* cur_dir_entry, int position);

int gen_short_name(uint8_t*, uint8_t*, uint32_t);
int write_entries_to_sd(const uint8_t*, const uint8_t*, const int, uint8_t, const uint32_t, const uint32_t, uint32_t);
void find_dir_entry(uint32_t, const char*, dir_entry_t*, SEARCH_TYPE);
uint32_t allocate_clusters(uint32_t first_cluster, int file_size);
#endif 		//__FAT_H__