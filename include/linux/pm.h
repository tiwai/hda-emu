#ifndef __LINUX_PM_H
#define __LINUX_PM_H

struct device;

/* stripped version */
struct dev_pm_ops {
	int (*suspend)(struct device *dev);
	int (*resume)(struct device *dev);
	int (*runtime_suspend)(struct device *dev);
	int (*runtime_resume)(struct device *dev);
	int (*runtime_idle)(struct device *dev);
};

#define SET_SYSTEM_SLEEP_PM_OPS(suspend_fn, resume_fn) \
	.suspend = suspend_fn, \
	.resume = resume_fn,

#define SET_RUNTIME_PM_OPS(suspend_fn, resume_fn, idle_fn) \
	.runtime_suspend = suspend_fn, \
	.runtime_resume = resume_fn, \
	.runtime_idle = idle_fn,

#endif /* __LINUX_PM_H */
