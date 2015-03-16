#ifndef __LINUX_KREF_H
#define __LINUX_KREF_H

struct kref {
	int refcount;
};

#define kref_init(kref)	do { (kref)->refcount = 1; } while (0)
#define kref_get(kref)	do { (kref)->refcount++; } while (0)

static inline int kref_sub(struct kref *kref, unsigned int count,
			   void (*release)(struct kref *kref))
{
	kref->refcount -= count;
	if (!kref->refcount) {
		release(kref);
		return 1;
	}
	return 0;
}

#define kref_put(kref, release)	kref_sub(kref, 1, release)

#endif /* __LINUX_KREF_H */
