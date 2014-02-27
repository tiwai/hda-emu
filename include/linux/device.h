#ifndef __LINUX_DEVICE_H
#define __LINUX_DEVICE_H

struct class;
struct attribute_group;

struct device {
	struct device *parent;
	void *driver_data;
	struct class *class;
	void (*release)(struct device *);
	const struct attribute_group **groups;

	/* XXX */
	void *device_data;
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

/* device management */
struct device;
#define get_device(dev)		/* NOP */
#define put_device(dev)		/* NOP */
static inline int device_add(struct device *dev) { return 0; }
static inline void device_del(struct device *dev) {}
static inline void device_initialize(struct device *dev) {}

#define dev_set_name(dev, name, fmt, args...)
#define dev_set_drvdata(dev, data) do { (dev)->device_data = (data); } while (0)
#define dev_get_drvdata(dev)	(dev)->device_data

#endif /* __LINUX_DEVICE_H */
