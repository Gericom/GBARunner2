#ifndef VECTOR_H__
#define VECTOR_H__

typedef struct vector_ vector;

void vector_init(vector*);
int vector_count(vector*);
void vector_add(vector*, void*);
void vector_set(vector*, int, void*);
void *vector_get(vector*, int);
void vector_delete(vector*, int);
void vector_free(vector*);

struct vector_ {
    void** data;
    int size;
    int count;

	void* operator[] (int idx) { return vector_get(this, idx); }
};

#endif