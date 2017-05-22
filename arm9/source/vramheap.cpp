#include <nds.h>
#include "vram.h"
#include "sd_access.h"
#include "vramheap.h"

#define HEAP_TAG_USED	(uint32_t)'USED'
#define HEAP_TAG_FREE	(uint32_t)'FREE'

//TODO: make a vram data section to put this in; or make it const or something
static uint16_t* ram_start = NULL;
static uint32_t ram_size = 0;

extern "C" uint32_t __vram_start;
extern "C" uint32_t __vram_end;

#define VRAM_START ((uint32_t)&__vram_start)
#define VRAM_END ((uint32_t)&__vram_end)

static heap_block_t* first_block = NULL;

PUT_IN_VRAM void vramheap_init()
{
	ram_start = (uint16_t*)VRAM_END;
	ram_size = (VRAM_START + 128 * 1024) - VRAM_END;
}

PUT_IN_VRAM static heap_block_t* vramheap_findfreeblock(heap_block_t** last, int size)
{
	heap_block_t* cur = first_block;
	while (cur && !(cur->tag == HEAP_TAG_FREE && cur->size >= size)) {
		*last = cur;
		cur = cur->next;
	}
	return cur;
}

PUT_IN_VRAM static heap_block_t* vramheap_addblock(heap_block_t* prev, int size)
{
	uint8_t* newblock = ((uint8_t*)&prev->data[0]) + prev->size;
	while(((uint32_t)newblock % 32) != 0) newblock++;
	if(((uint32_t)newblock) + size >= ((uint32_t)ram_start) + ram_size)
	{
		//printf("Alloc failure!\n");
		while(1);
	}
	heap_block_t* block = (heap_block_t*)newblock;
	block->tag = HEAP_TAG_USED;
	block->size = size;
	block->prev = prev;
	prev->next = block;
	block->next = NULL;
	return block;
}

PUT_IN_VRAM static heap_block_t* vramheap_mergeblocks(heap_block_t* a, heap_block_t* b)
{
	if(a->tag != HEAP_TAG_FREE || b->tag != HEAP_TAG_FREE)
	{
		//printf("a->tag != PAKRAM_TAG_FREE || b->tag != PAKRAM_TAG_FREE\n");
		return NULL;
	}
	heap_block_t* first = ((uint32_t)a < (uint32_t)b) ? a : b;
	heap_block_t* second = ((uint32_t)a < (uint32_t)b) ? b : a;
	if(first->next != second || second->prev != first)
	{
		//printf("first->next != second || second->prev != first\n");
		return NULL;
	}
	first->next = second->next;
	if(second->next) second->next->prev = first;
	first->size = (((uint32_t)&second->data[0]) + second->size) - ((uint32_t)&first->data[0]);
	second->tag = 0;
	second->size = 0;
	second->prev = NULL;
	second->next = NULL;
	return first;
}

PUT_IN_VRAM static heap_block_t* vramheap_expandblock(heap_block_t* block)
{
	if(block->tag != HEAP_TAG_FREE) return NULL;
	if(block->prev && block->prev->tag == HEAP_TAG_FREE) block = vramheap_mergeblocks(block->prev, block);
	if(block->next && block->next->tag == HEAP_TAG_FREE) block = vramheap_mergeblocks(block, block->next);
	return block;
}

PUT_IN_VRAM static void vramheap_splitblock(heap_block_t* block, int size)
{
	//Don't split if we can not create another block after it
	if(block->size - size <= 0x10) return;
	uint8_t* oldblock = ((uint8_t*)&block->data[0]) + block->size;
	while(((uint32_t)oldblock % 32) != 0) oldblock++;
	uint8_t* newblock = ((uint8_t*)&block->data[0]) + size;
	while(((uint32_t)newblock % 32) != 0) newblock++;
	if((uint32_t)newblock >= (uint32_t)oldblock) return;
	heap_block_t* block2 = (heap_block_t*)newblock;
	block2->tag = HEAP_TAG_FREE;
	block2->size = ((uint32_t)oldblock - (uint32_t)newblock) - 0x10;
	block2->prev = block;
	block2->next = block->next;
	if(block->next) block->next->prev = block2;
	block->next = block2;
	block->size = size;
	//maybe there is a free block after it
	vramheap_expandblock(block2);
}

PUT_IN_VRAM uint16_t* vramheap_alloc(int size)
{
	//16 bit alignment
	if(size & 1) size++;
	if(first_block == NULL)
	{
		first_block = (heap_block_t*)ram_start;
		first_block->tag = HEAP_TAG_USED;
		first_block->size = size;
		first_block->prev = NULL;
		first_block->next = NULL;
		return &first_block->data[0];
	}
	heap_block_t* last = first_block;
	heap_block_t* block = vramheap_findfreeblock(&last, size);
	if(!block) block = vramheap_addblock(last, size);
	else
	{
		block->tag = HEAP_TAG_USED;
		vramheap_splitblock(block, size);
	}
	return &block->data[0];
}

PUT_IN_VRAM uint16_t* vramheap_realloc(void* ptr, int size)
{
	if(ptr == NULL)
		return vramheap_alloc(size);
	heap_block_t* block = (heap_block_t*)(((uint8_t*)ptr) - 0x10);
	int oldsize = block->size;
	if(size == block->size)
		return &block->data[0];
	if(size < block->size)
	{
		vramheap_splitblock(block, size);
		return &block->data[0];
	}
	//try expanding
	/*vramheap_expandblock(block);
	if(block->size == size)
		return &block->data[0];
	if(block->size > size)
	{
		vramheap_splitblock(block, size);
		return &block->data[0];
	}*/
	uint16_t* newblock = vramheap_alloc(size);
	arm9_memcpy16(newblock, &block->data[0], oldsize / 2);
	vramheap_free(&block->data[0]);
	return newblock;
}

PUT_IN_VRAM void vramheap_free(void* ptr)
{
	if(ptr == NULL)
		return;
	heap_block_t* block = (heap_block_t*)(((uint8_t*)ptr) - 0x10);
	block->tag = HEAP_TAG_FREE;
	heap_block_t* newblock = vramheap_expandblock(block);
	//last
	if(newblock->next == NULL)
	{
		if(newblock->prev) newblock->prev->next = NULL;
		else first_block = NULL;//no previous means the heap is empty now, so remove the first block!
		newblock->tag = 0;
		newblock->size = 0;
		newblock->prev = NULL;
		newblock->next = NULL;
	}
}