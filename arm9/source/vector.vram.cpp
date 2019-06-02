#include "vram.h"
#include "vramheap.h"

#include "vector.h"

void vector_init(vector *v)
{
    v->data = NULL;
    v->size = 0;
    v->count = 0;
}

int vector_count(vector *v)
{
    return v->count;
}

void vector_add(vector *v, void *e)
{
    if (v->size == 0) {
        v->size = 10;
        v->data = (void**)vramheap_alloc(sizeof(void*) * v->size);

        //memset(v->data, '\0', sizeof(void*) * v->size);
    }

    if (v->size == v->count) {
        v->size *= 2;
        v->data = (void**)vramheap_realloc(v->data, sizeof(void*) * v->size);
    }

    v->data[v->count] = e;
    v->count++;
}

void vector_set(vector *v, int index, void *e)
{
    if (index >= v->count) {
        return;
    }

    v->data[index] = e;
}

void *vector_get(vector *v, int index)
{
    if (index >= v->count) {
        return NULL;
    }

    return v->data[index];
}

void vector_delete(vector *v, int index)
{
    if (index >= v->count) {
        return;
    }

    //for (int i = index, j = index; i < v->count; i++) {
    //    v->data[j] = v->data[i];
    //    j++;
    //}
	for (int i = index; i < v->count; i++) {
        v->data[i] = v->data[i + 1];
    }

    v->count--;
}

void vector_free(vector *v)
{
	if(v->data != NULL)
	{
		vramheap_free(v->data);
		vector_init(v);
	}
}