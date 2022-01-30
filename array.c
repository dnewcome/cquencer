// https://stackoverflow.com/a/3536261

#include <stdlib.h>
#include "cseq.h"

#define array_t struct event* 

typedef struct {
  array_t *array;
  size_t used;
  size_t size;
} Array;

void initArray(Array *a, size_t initialSize) {
  a->array = malloc(initialSize * sizeof(array_t));
  a->used = 0;
  a->size = initialSize;
}

void insertArray(Array *a, array_t element) {
  if (a->used == a->size) {
    a->size *= 2;
    a->array = realloc(a->array, a->size * sizeof(array_t));
  }
  a->array[a->used++] = element;
}

void freeArray(Array *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}