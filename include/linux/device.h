#ifndef __LINUX_DEVICE_H
#define __LINUX_DEVICE_H

struct device {
	struct device *parent;
	void *driver_data;
};

#endif /* __LINUX_DEVICE_H */
