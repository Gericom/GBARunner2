#ifndef __LOCK_H__
#define __LOCK_H__

extern "C" void lock_lock(volatile u8* lock);
extern "C" void lock_unlock(volatile u8* lock);

#endif