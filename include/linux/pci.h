#ifndef __LINUX_PCI_H
#define __LINUX_PCI_H

#include "wrapper.h"
#include <linux/device.h>

struct pci_dev {
	struct pci_bus	*bus;
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

/*
 * FIXME: dummy functions for ca0132
 */

static inline void writeb(unsigned char val, void *addr) {}
static inline unsigned char readb(void *addr) { return 0; }
static inline void writew(unsigned short val, void *addr) {}
static inline unsigned short readw(void *addr)  { return 0; }
static inline void writel(unsigned int val, void *addr) {}
static inline unsigned int readl(void *addr) { return 0; }

static inline void *pci_iomap(struct pci_dev *pci, int bar, int offset)
{
	return (void *)1UL;
}

static inline void iounmap(void *addr) {}
static inline void pci_iounmap(struct pci_dev *pci, void *addr) {}

#define dev_is_pci(d) true
#define	to_pci_dev(n) container_of(n, struct pci_dev, dev)

#endif /* __LINUX_PCI_H */
