#ifndef __LINUX_MODULE_H
#define __LINUX_MODULE_H

#define MODULE_DESCRIPTION(x)
#define MODULE_LICENSE(x)
#define MODULE_AUTHOR(x)
#define MODULE_SUPPORTED_DEVICE(x)
#define MODULE_ALIAS(x)

/* export as XXX_parameter */
#define module_param(a,b,c) b *a##_parameter = &a
#define module_param_array(a,b,c,d)
#define MODULE_PARM_DESC(a,b)

#define module_init(func)	int call_init_##func(void) { return func(); }
#define module_exit(func)	void call_exit_##func(void) { func(); }

struct module;

#define EXPORT_SYMBOL(x)
#define EXPORT_SYMBOL_GPL(x)

static inline int request_module(const char *name) { return 0; }
#define module_put(x) do { } while (0)
#define module_get(x) do { } while (0)
static inline int try_module_get(struct module *x) { return 1; }

#define THIS_MODULE	NULL

#endif /* __LINUX_MODULE_H */
