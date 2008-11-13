#ifndef __LINUX_SLAB_H
#define __LINUX_SLAB_H

#include "wrapper.h"

typedef unsigned int gfp_t;
#define GFP_KERNEL	0
#define GFP_ATOMIC	0

#define kmalloc(size,gfp)	malloc(size)
#define kzalloc(size,gfp)	calloc(1,size)
#define kcalloc(elem,size,gfp)	calloc(elem,size)
#define kfree(ptr)		free((void*)(ptr))
#define kstrdup(str,x)		strdup(str)

static inline size_t strlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);
	if (size) {
		size_t len = (ret >= size) ? size - 1 : ret;
		memcpy(dest, src, len);
		dest[len] = '\0';
	}
	return ret;
}

#endif /* __LINUX_SLAB_H */
