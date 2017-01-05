.global _dldi_start
.equ _dldi_start,					0x03805000
.global _io_dldi
.equ _io_dldi,						(_dldi_start + 0x60)
.global _DLDI_startup_ptr
.equ _DLDI_startup_ptr,				(_io_dldi + 0x8)
.global _DLDI_isInserted_ptr
.equ _DLDI_isInserted_ptr,			(_io_dldi + 0xC)
.global _DLDI_readSectors_ptr
.equ _DLDI_readSectors_ptr,			(_io_dldi + 0x10)
.global _DLDI_writeSectors_ptr
.equ _DLDI_writeSectors_ptr,		(_io_dldi + 0x14)
.global _DLDI_clearStatus_ptr
.equ _DLDI_clearStatus_ptr,			(_io_dldi + 0x18)
.global _DLDI_shutdown_ptr
.equ _DLDI_shutdown_ptr,			(_io_dldi + 0x1C)