
address_read_table_32bit_dtcm = 0x1000086C
address_read_table_16bit_dtcm = 0x10000974
address_read_table_8bit_dtcm = 0x10000B80

address_write_table_32bit_dtcm = 0x10000140
address_write_table_16bit_dtcm = 0x10000248
address_write_table_8bit_dtcm = 0x10000454

sd_cluster_cache = 0x06840000

sd_data_base = 0x06840000
sd_is_cluster_cached_table = (sd_data_base + (224 * 1024)) @(96 * 1024))
sd_cluster_cache_info = (sd_is_cluster_cached_table + (16 * 1024))
sd_sd_info = (sd_cluster_cache_info + (256 * 8 + 4)) @0x0685C404
