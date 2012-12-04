#ifndef _LINUX_FIRMWARE_H
#define _LINUX_FIRMWARE_H

#include <linux/device.h>

struct firmware {
	size_t size;
	const u8 *data;
};

static inline int
request_firmware(const struct firmware **fw, const char *name,
		 struct device *device)
{
	return -ENODEV;
}

static inline void release_firmware(const struct firmware *fw) {}

#define MODULE_FIRMWARE(name)

#endif
