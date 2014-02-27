#ifndef __LINUX_DEVICE_H
#define __LINUX_DEVICE_H

struct device {
	struct device *parent;
	void *driver_data;
};

/* dev_err() & co */
#define dev_emerg(dev, fmt, args...)	pr_emerg(fmt, ##args)
#define dev_alert(dev, fmt, args...)	pr_alert(fmt, ##args)
#define dev_crit(dev, fmt, args...)	pr_crit(fmt, ##args)
#define dev_err(dev, fmt, args...)	pr_err(fmt, ##args)
#define dev_warn(dev, fmt, args...)	pr_warning(fmt, ##args)
#define dev_notice(dev, fmt, args...)	pr_notice(fmt, ##args)
#define dev_info(dev, fmt, args...)	pr_info(fmt, ##args)
#define dev_dbg(dev, fmt, args...)	pr_debug(fmt, ##args)

#endif /* __LINUX_DEVICE_H */
