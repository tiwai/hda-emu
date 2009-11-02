#ifndef __LINUX_BITMAP_H
#define __LINUX_BITMAP_H

#include "wrapper.h"

#define BITS_PER_LONG		(sizeof(long) * 8)
#define BITS_TO_LONGS(bits)	(((bits) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#define BIT(nr)			(1UL << (nr))
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE		8

#define DECLARE_BITMAP(name,bits) \
	unsigned long name[BITS_TO_LONGS(bits)]

static inline void set_bit(unsigned int nr, unsigned long *addr)
{
	addr[BIT_WORD(nr)] |= BIT_MASK(nr);
}

static inline void clear_bit(unsigned int nr, unsigned long *addr)
{
	addr[BIT_WORD(nr)] &= ~BIT_MASK(nr);
}

static inline int test_bit(unsigned int nr, unsigned long *addr)
{
	return (addr[BIT_WORD(nr)] & BIT_MASK(nr)) != 0;
}

static inline int test_and_set_bit(unsigned int nr, unsigned long *addr)
{
	int oldval = test_bit(nr, addr);
	set_bit(nr, addr);
	return oldval;
}

#endif
