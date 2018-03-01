#pragma once
// -----------------------------------------------------------------
//
// Based on https://github.com/nothings/stb/blob/master/stretchy_buffer.h
// written by Sean Barret (https://nothings.org/)
//
// Another nice article about this:
// http://ourmachinery.com/post/minimalist-container-library-in-c-part-1/
//
// -----------------------------------------------------------------
struct array_header {
	unsigned int capacity;
	unsigned int size;
};

#define array_free(a) \
	((a) ? free(array_header(a)),0 : 0)

#define array_clear(a) \
	((a) ? array_header(a)->size = 0 : 0)

#define array_header(a) \
    ((array_header*)((char *)(a) - sizeof(array_header)))

#define array_size(a) \
    ((a) ? (array_header((a))->size) : 0)

#define array_capacity(a) \
    ((a) ? (array_header((a))->capacity) : 0)

#define array_full(a) \
    ((a) ? (array_size(a) == array_capacity(a)) : true)

#define array_push(a,v) \
	(array__requires_grow(a,1), \
	(a)[array_header((a))->size++] = (v))

#define array_remove(a,v) \
	(array_size((a)) > 0 ? (a)[v] = (a)[array_header((a))->size-1], --array_header((a))->size : 0 )

#define array_last(a) \
	((a)[array_size(a)-1])

#define array__requires_grow(a,n) \
	(array_full((a)) ? array_resize(a,n) : 0)

#define array_resize(a,n) \
	(*((void **)&(a)) = array__growf((a), (n), sizeof(*(a))))

#include <stdlib.h>

static void * array__growf(void *arr, int increment, int itemsize) {
	int dbl_cur = arr ? 2 * array_capacity(arr) + 8 : 8;
	int min_needed = array_size(arr) + increment;
	int m = dbl_cur > min_needed ? dbl_cur : min_needed;
	char* p = (char*)realloc(arr ? array_header(arr) : 0, itemsize * m + sizeof(array_header));
	if (p) {
		array_header* h = (array_header*)p;
		if (!arr) {
			h->size = 0;
		}
		h->capacity = m;
		return p + sizeof(array_header);
	}
	return 0;
}
