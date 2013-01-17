#ifndef _LINUX_BITOPS_H
#define _LINUX_BITOPS_H

/* we don't care the efficiency, so let's implement in stupid but easy ways */

static inline unsigned long hweight_long(unsigned long w)
{
	int i, c = 0;
	for (i = 0; i < sizeof(w) * 8; i++) {
		c += w & 1;
		w >>= 1;
	}
	return c;
}

static inline unsigned int hweight32(unsigned int w)
{
	int i, c = 0;
	for (i = 0; i < sizeof(w) * 8; i++) {
		c += w & 1;
		w >>= 1;
	}
	return c;
}

static inline unsigned long find_first_bit(const unsigned long *addr,
					   unsigned long size)
{
	int i;
	unsigned long v = *addr++;
	for (i = 1; i <= size; i++) {
		if (v & 1)
			return i;
		if (i % (sizeof(unsigned long) * 8))
			v >>= 1;
		else
			v = *addr++;
	}
	return size + 1;
}

#endif
