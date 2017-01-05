#ifndef __DLDI_HANDLER_H__
#define __DLDI_HANDLER_H__

#include "../../common/sd_vram.h"

typedef bool (* FN_MEDIUM_STARTUP)(void) ;
typedef bool (* FN_MEDIUM_ISINSERTED)(void) ;
typedef bool (* FN_MEDIUM_READSECTORS)(sec_t sector, sec_t numSectors, void* buffer) ;
typedef bool (* FN_MEDIUM_WRITESECTORS)(sec_t sector, sec_t numSectors, const void* buffer) ;
typedef bool (* FN_MEDIUM_CLEARSTATUS)(void) ;
typedef bool (* FN_MEDIUM_SHUTDOWN)(void) ;

bool dldi_handler_init();
void dldi_handler_read_sectors(sec_t sector, sec_t numSectors, void* buffer);

#endif