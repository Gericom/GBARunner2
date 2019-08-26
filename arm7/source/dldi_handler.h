#ifndef __DLDI_HANDLER_H__
#define __DLDI_HANDLER_H__

#include "../../common/sd_vram.h"

bool dldi_handler_init();
void dldi_handler_read_sectors(sec_t sector, sec_t numSectors, void* buffer);
void dldi_handler_write_sectors(sec_t sector, sec_t numSectors, const void* buffer);

#endif