#ifndef __LINUX_SYSFS_H
#define __LINUX_SYSFS_H

/*
 * just dummy definitions to make build hdac_sysfs.c
 */ 

struct kobject;
struct kobj_type;
struct kobj_uevent_env;

struct kobject {
	char *name;
	struct kobj_type *type;
};

#define kobj_to_dev(x)	NULL
#define kobject_put(k)
#define kobject_init(k, v) ((k)->type = (v))
#define kobject_add(k, parent, fmt, ...) 0
static inline struct kobject *kobject_create_and_add(const char *name, struct kobject *kobj)
{
	return calloc(1, sizeof(*kobj));
}

#define kobject_uevent(k, e)

struct kobj_type {
	void (*release)(struct kobject *kobj);
	const struct sysfs_ops *sysfs_ops;
};

typedef int umode_t;

struct attribute {
	char *name;
	umode_t mode;
};

struct attribute_group {
	const char		*name;
	umode_t			(*is_visible)(struct kobject *,
					      struct attribute *, int);
	struct attribute	**attrs;
	struct bin_attribute	**bin_attrs;
};

#define __ATTR(_name, _mode, _show, _store) {				\
	.attr = {.name = #_name, .mode = (_mode) },		\
	.show	= _show,						\
	.store	= _store,						\
}

#define __ATTR_RO(_name) {						\
	.attr	= { .name = #_name, .mode = 0444 },	\
	.show	= _name##_show,						\
}

#define __ATTR_RW(_name) __ATTR(_name, 0644),		\
			 _name##_show, _name##_store)

struct sysfs_ops {
	ssize_t	(*show)(struct kobject *, struct attribute *, char *);
	ssize_t	(*store)(struct kobject *, struct attribute *, const char *, size_t);
};

static inline int sysfs_create_group(struct kobject *kobj,
				     const struct attribute_group *grp) { return 0; }
static inline void sysfs_remove_group(struct kobject *kobj,
				      const struct attribute_group *grp) {}

static inline int add_uevent_var(struct kobj_uevent_env *env, const char *fmt,
				 ...)
{
	return 0;
}

#endif /* __LINUX_SYSFS_H */
