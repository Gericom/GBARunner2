#ifndef __SD_ACCESS_H__
#define __SD_ACCESS_H__

#define READ_U16_SAFE(addr)		(((uint8_t*)(addr))[0] | (((uint8_t*)(addr))[1] << 8))
#define READ_U32_SAFE(addr)		(((uint8_t*)(addr))[0] | (((uint8_t*)(addr))[1] << 8) | (((uint8_t*)(addr))[2] << 16) | (((uint8_t*)(addr))[3] << 24))

typedef uint32_t sec_t;

typedef bool (* FN_MEDIUM_STARTUP)(void) ;
typedef bool (* FN_MEDIUM_ISINSERTED)(void) ;
typedef bool (* FN_MEDIUM_READSECTORS)(sec_t sector, sec_t numSectors, void* buffer) ;
typedef bool (* FN_MEDIUM_WRITESECTORS)(sec_t sector, sec_t numSectors, const void* buffer) ;
typedef bool (* FN_MEDIUM_CLEARSTATUS)(void) ;
typedef bool (* FN_MEDIUM_SHUTDOWN)(void) ;

typedef enum
{
	MBR_PARTITION_TYPE_EMPTY = 0,
	MBR_PARTITION_TYPE_FAT12 = 1,
	MBR_PARTITION_TYPE_FAT16_SMALL = 4,
	MBR_PARTITION_TYPE_EXTENDED_PARTITION = 5,
	MBR_PARTITION_TYPE_FAT16_BIG = 6,
	MBR_PARTITION_TYPE_FAT32 = 11,
	MBR_PARTITION_TYPE_FAT32_LBA = 12,
	MBR_PARTITION_TYPE_FAT16_LBA = 14,
	MBR_PARTITION_TYPE_EXTENDED_PARTITION_LBA = 15
} MBR_PARTITION_TYPE;

typedef struct
{
	uint8_t address[3];
} PACKED chs_t;

typedef struct
{
	uint8_t active;	//0x80 = active (bootable), 0x00 = inactive
	chs_t chs_partition_start;
	uint8_t partition_type;
	chs_t chs_partition_end;
	sec_t lba_partition_start;
	sec_t lba_partition_size;
} PACKED partition_t;

typedef struct
{
	uint8_t non_usefull_stuff[0x1BE];
	partition_t partitions[4];
	uint16_t signature;	//should be 0xAA55
} PACKED mbr_t;

typedef struct
{
	uint8_t drive_nr;
	uint8_t cur_head;
	uint8_t boot_sig;//0x29 indicates that the following values are valid
	uint32_t volume_id;//UNALIGNED! USE READ_U32_SAFE
	char volume_label[11];
	char fs_type[8];
} PACKED bootsect_cmn_data_t;

typedef struct
{
	uint8_t x86jmp[3];
	char oemname[8];
	uint16_t sector_size;//UNALIGNED! USE READ_U16_SAFE
	uint8_t nr_sector_per_cluster;
	uint16_t nr_reserved_sectors;
	uint8_t nr_fats;
	uint16_t fat16_nr_dir_entries_root;//UNALIGNED! USE READ_U16_SAFE (and only for fat16)
	uint16_t total_nr_sectors_short;//UNALIGNED! USE READ_U16_SAFE (use total_nr_sectors_long if zero)
	uint8_t media_descriptor;
	uint16_t fat16_nr_sectors_per_fat;//only for fat16
	uint16_t nr_sectors_per_cylinder;
	uint16_t nr_heads;
	uint32_t nr_hidden_sectors;
	uint32_t total_nr_sectors_long;
	union
	{
		struct
		{
			bootsect_cmn_data_t fat16_cmn_data;
			uint8_t fat16_boot_code[448];
		};
		struct
		{
			uint32_t fat32_nr_sectors_per_fat;//only for fat32
			uint16_t fat32_fat_flags;//only for fat32
			uint16_t fat32_fs_version;//only for fat32
			uint32_t fat32_root_dir_cluster;//only for fat32
			uint16_t fat32_fs_info_sector;//only for fat32
			uint16_t fat32_boot_sector_backup_sector;//only for fat32
			uint8_t fat32_reserved[12];//only for fat32
			bootsect_cmn_data_t fat32_cmn_data;
			uint8_t fat32_boot_code[372];
		};
	};
	uint16_t signature;	//should be 0xAA55
} PACKED bootsect_t;

typedef enum
{
	DIR_ATTRIB_READ_ONLY =	(1 << 0),
	DIR_ATTRIB_HIDDEN =		(1 << 1),
	DIR_ATTRIB_SYSTEM =		(1 << 2),
	DIR_ATTRIB_VOLUME_ID =	(1 << 3),
	DIR_ATTRIB_DIRECTORY =	(1 << 4),
	DIR_ATTRIB_ARCHIVE =	(1 << 5),
	DIR_ATTRIB_LONG_FILENAME = 0xF
} DIR_ATTRIB;

typedef struct
{
	union
	{
		char short_name[11];
		struct
		{
			uint8_t record_type;
			uint8_t invalid_data[10];
		};
	};
	uint8_t attrib;
	uint8_t reserved;
	uint8_t creation_time_tenths_of_seconds;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t last_access_date;
	uint16_t cluster_nr_top;
	uint16_t last_modification_time;
	uint16_t last_modification_date;
	uint16_t cluster_nr_bottom;
	uint32_t file_size;
} PACKED dir_entry_regular_t;

typedef struct
{
	uint8_t order;
	uint16_t name_part_one[5];	//not aligned!
	uint8_t attrib;
	uint8_t long_entry_type;
	uint8_t checksum;
	uint16_t name_part_two[6];
	uint16_t reserved;
	uint16_t name_part_three[2];
} PACKED dir_entry_long_name_t;

typedef union
{
	dir_entry_regular_t regular_entry;
	dir_entry_long_name_t long_name_entry;
	struct
	{
		uint8_t first_part[11];
		uint8_t attrib;
		uint8_t second_part[20];
	};
} PACKED dir_entry_t;

typedef struct
{
	uint32_t gba_rom_size;
	uint32_t cluster_shift;
	uint32_t cluster_mask;
	uint32_t access_counter;

	uint8_t nr_sectors_per_cluster;
	sec_t first_fat_sector;
	sec_t first_cluster_sector;
	uint32_t root_directory_cluster;
} sd_info_t;

typedef struct
{
	uint32_t counter	: 24;
	//uint32_t reserved	: 7;
	uint32_t counter2	: 7;
	uint32_t in_use		: 1;
	uint32_t cluster_index;
} cluster_cache_block_info_t;

typedef struct
{
	cluster_cache_block_info_t cache_block_info[256];//128];//128 blocks at max seems reasonable
	uint32_t total_nr_cacheblocks;
} cluster_cache_info_t;

//vram config
typedef struct
{
	//vram b and c
	uint8_t cluster_cache[256 * 1024];//96 * 1024];
	/*uint8_t gba_rom_is_cluster_cached_table[16 * 1024];	//allows roms up to 64MB
	union
	{
		uint8_t reserved[16 * 1024];//will be used for memory management of the cached clusters
		struct
		{
			cluster_cache_info_t cluster_cache_info;
			sd_info_t sd_info;
		};
	};*/
	//vram d
	uint32_t gba_rom_cluster_table[32 * 1024 / 4];//allows roms up to 32MB
	uint8_t arm9_transfer_region[64 * 1024];//contains data requested by the arm9
	//uint8_t dldi_region[32 * 1024];//contains the dldi code
	uint8_t gba_rom_is_cluster_cached_table[16 * 1024];	//allows roms up to 64MB
	union
	{
		uint8_t reserved[16 * 1024];//will be used for memory management of the cached clusters
		struct
		{
			cluster_cache_info_t cluster_cache_info;
			sd_info_t sd_info;
		};
	};
} vram_cd_t;

//extern sd_info_t gSDInfo;

#endif