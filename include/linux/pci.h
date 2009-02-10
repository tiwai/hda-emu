#ifndef __LINUX_PCI_H
#define __LINUX_PCI_H

#include "wrapper.h"
#include <linux/device.h>

struct pci_dev {
	unsigned short vendor;
	unsigned short device;
	unsigned short subsystem_vendor;
	unsigned short subsystem_device;
	unsigned short revision;
	struct device dev;
};

#define PCI_SUBSYSTEM_VENDOR_ID	0x2c
#define PCI_SUBSYSTEM_ID	0x2e

static inline int pci_read_config_word(struct pci_dev *dev, int where, u16 *val)
{
	switch (where) {
	case PCI_SUBSYSTEM_VENDOR_ID:
		*val = dev->subsystem_vendor;
		break;
	case PCI_SUBSYSTEM_ID:
		*val = dev->subsystem_device;
		break;
	}
	return 0;
}

#endif /* __LINUX_PCI_H */
