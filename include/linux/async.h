#ifndef __LINUX_ASYNC_H
#define __LINUX_ASYNC_H

typedef u64 async_cookie_t;
typedef void (*async_func_t) (void *data, async_cookie_t cookie);
struct async_domain {
	int count;
};

#define ASYNC_DOMAIN(_name) \
	struct async_domain _name = { .count = 0 }
#define ASYNC_DOMAIN_EXCLUSIVE(_name) \
	struct async_domain _name = { .count = 0 }

static inline async_cookie_t async_schedule(async_func_t func, void *data)
{
	func(data, 0);
	return 0;
}

static inline async_cookie_t async_schedule_domain(async_func_t func, void *data,
						   struct async_domain *domain)
{
	func(data, 0);
	domain->count++;
	return 0;
}

static inline void async_unregister_domain(struct async_domain *domain) {}
static inline void async_synchronize_full(void) {}
static inline void async_synchronize_full_domain(struct async_domain *domain) {}
static inline void async_synchronize_cookie(async_cookie_t cookie) {}
static inline  void async_synchronize_cookie_domain(async_cookie_t cookie,
					    struct async_domain *domain) {}
static inline bool current_is_async(void) { return false; }

#endif /* __LINUX_ASYNC_H */
