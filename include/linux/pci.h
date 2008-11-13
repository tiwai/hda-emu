#ifndef __LINUX_PCI_H
#define __LINUX_PCI_H

#include "wrapper.h"

struct pci_dev {
	unsigned short vendor;
	unsigned short device;
	unsigned short subsystem_vendor;
	unsigned short subsystem_device;
	unsigned short revision;
};

#endif /* __LINUX_PCI_H */
