#ifndef __LINUX_DEVICE_H
#define __LINUX_DEVICE_H

#include <wrapper.h>
#include <linux/list.h>
#include <linux/pm.h>
#include <linux/sysfs.h>

struct device;
struct device_driver;
struct bus_type;
struct class;
struct attribute_group;
struct device_node;

struct device {
	struct kobject kobj;
	struct device *parent;
	void *driver_data;
	struct bus_type	*bus;
	struct device_driver *driver;
	struct class *class;
	void (*release)(struct device *);
	const struct attribute_group **groups;
	struct dev_pm_info power;

	/* hda-emu specific */
	void *device_data;
	bool registered;
	bool pmsuspended;
	bool pmallow;
	int pmcnt;
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

void device_initialize(struct device *dev);
int device_add(struct device *dev);
void device_del(struct device *dev);

#define device_is_registered(dev)	((dev)->registered)
#define device_enable_async_suspend(dev)	/* NOP */

#define dev_set_name(dev, name, fmt, args...)
#define dev_set_drvdata(dev, data) do { (dev)->device_data = (data); } while (0)
#define dev_get_drvdata(dev)	(dev)->device_data

struct device_driver {
	const char		*name;
	struct bus_type		*bus;
	struct module		*owner;
	const char		*mod_name;	/* used for built-in modules */
	int (*probe) (struct device *dev);
	int (*remove) (struct device *dev);
	void (*shutdown) (struct device *dev);
	int (*suspend) (struct device *dev, pm_message_t state);
	int (*resume) (struct device *dev);
	const struct attribute_group **groups;
	const struct dev_pm_ops *pm;

	/* hda-emu specific */
	struct list_head list;
};

int driver_register(struct device_driver *drv);
void driver_unregister(struct device_driver *drv);
int device_attach(struct device *dev);

struct bus_type {
	const char		*name;

	int (*match)(struct device *dev, struct device_driver *drv);
	int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
	int (*probe)(struct device *dev);
	int (*remove)(struct device *dev);
	void (*shutdown)(struct device *dev);

	int (*online)(struct device *dev);
	int (*offline)(struct device *dev);

	int (*suspend)(struct device *dev, pm_message_t state);
	int (*resume)(struct device *dev);
};

int bus_register(struct bus_type *bus);
void bus_unregister(struct bus_type *bus);

/*
 */
struct device_attribute {
	struct attribute	attr;
	ssize_t (*show)(struct device *dev, struct device_attribute *attr,
			char *buf);
	ssize_t (*store)(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count);
};

#define DEVICE_ATTR(_name, _mode, _show, _store) \
	struct device_attribute dev_attr_##_name = __ATTR(_name, _mode, _show, _store)
#define DEVICE_ATTR_RW(_name) \
	struct device_attribute dev_attr_##_name = __ATTR_RW(_name)
#define DEVICE_ATTR_RO(_name) \
	struct device_attribute dev_attr_##_name = __ATTR_RO(_name)

#define devm_kmalloc(dev, size, gfp)	kmalloc(size, gfp)
#define devm_kzalloc(dev, size, gfp)	kzalloc(size, gfp)

#endif /* __LINUX_DEVICE_H */
