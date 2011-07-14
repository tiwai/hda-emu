#ifndef __LINUX_SLAB_H
#define __LINUX_SLAB_H

#include "wrapper.h"

typedef unsigned int gfp_t;
#define GFP_KERNEL	0
#define GFP_ATOMIC	0

#ifdef DEBUG_MALLOC
#define kmalloc(size,gfp)	__hda_malloc(size, __FILE__, __LINE__)
#define kzalloc(size,gfp)	kmalloc(size, gfp)
#define kcalloc(elem,size,gfp)	kmalloc((elem)*(size), gfp)
#define krealloc(ptr,size,gfp)	__hda_realloc(ptr, size, __FILE__, __LINE__)
#define kfree(ptr)		__hda_free((void*)(ptr), __FILE__, __LINE__)
#define kstrdup(str,x)		__hda_strdup(str, __FILE__, __LINE__)
#else
#define kmalloc(size,gfp)	malloc(size)
#define kzalloc(size,gfp)	calloc(1,size)
#define kcalloc(elem,size,gfp)	calloc(elem,size)
#define krealloc(ptr,size,gfp)	realloc(ptr, size)
#define kfree(ptr)		free((void*)(ptr))
#define kstrdup(str,x)		strdup(str)
#endif

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

static inline void *kmemdup(const void *src, size_t size, int gfp)
{
	void *dst = malloc(size);
	if (!dst)
		return NULL;
	memcpy(dst, src, size);
	return dst;
}

#endif /* __LINUX_SLAB_H */
