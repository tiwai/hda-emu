#ifndef _LINUX_SORT_H
#define _LINUX_SORT_H

#include <stdlib.h>

static inline void sort(void *base, size_t num, size_t size,
			int (*cmp)(const void *, const void *),
			void (*swap)(void *, void *, int))
{
	qsort(base, num, size, cmp);
}

#endif
