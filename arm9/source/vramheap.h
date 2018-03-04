#ifndef __VRAMHEAP_H__
#define __VRAMHEAP_H__
#include <cstddef>

typedef struct heap_block_t
{
	uint32_t tag;
	uint32_t size;
	heap_block_t* prev;
	heap_block_t* next;
	uint16_t data[0];
};

void vramheap_init();
uint16_t* vramheap_alloc(int size);
uint16_t* vramheap_realloc(void* ptr, int size);
void vramheap_free(void* ptr);
void* operator new(size_t blocksize);
void operator delete(void* block, size_t blocksize);

#endif