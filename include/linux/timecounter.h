#ifndef _LINUX_TIMECOUNTER_H
#define _LINUX_TIMECOUNTER_H

struct cyclecounter {
	cycle_t (*read)(const struct cyclecounter *cc);
	cycle_t mask;
	u32 mult;
	u32 shift;
};

struct timecounter {
	const struct cyclecounter *cc;
	cycle_t cycle_last;
	u64 nsec;
	u64 mask;
	u64 frac;
};

static inline void timecounter_init(struct timecounter *tc,
				    const struct cyclecounter *cc,
				    u64 start_tstamp)
{
	tc->cc = cc;
	tc->cycle_last = cc->read(cc);
	tc->nsec = start_tstamp;
	tc->mask = (1ULL << cc->shift) - 1;
	tc->frac = 0;
}

#endif /* _LINUX_TIMECOUNTER_H */
