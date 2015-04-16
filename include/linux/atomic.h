#ifndef _LINUX_ATOMIC_H
#define _LINUX_ATOMIC_H

typedef struct {
	int counter;
} atomic_t;

#define atomic_read(v)	((v)->counter)
#define atomic_add_return(n, v)	((v)->counter += (n))
#define atomic_sub_return(n, v)	((v)->counter -= (n))
#define atomic_inc(v)	atomic_add_return(1, v)
#define atomic_dec(v)	atomic_sub_return(1, v)
#define atomic_inc_return(v)	atomic_add_return(1, v)
#define atomic_dec_return(v)	atomic_sub_return(1, v)
#define atomic_set(v, i) (((v)->counter) = (i))
static inline int atomic_inc_not_zero(atomic_t *v)
{
	if (v->counter)
		v->counter++;
	return v->counter;
}
static inline int atomic_dec_if_positive(atomic_t *v)
{
	if (!v->counter)
		return -1;
	return --v->counter;
}
#define ATOMIC_INIT(i)	{ (i) }

#endif /* _LINUX_ATOMIC_H */

