/* stripped version of regmap implementation for hda-emu */

#ifndef __LINUX_REGMAP_H
#define __LINUX_REGMAP_H

#include <linux/err.h>

enum regcache_type {
	REGCACHE_NONE,
	REGCACHE_RBTREE,
	REGCACHE_COMPRESSED,
	REGCACHE_FLAT,
};

enum regmap_endian {
	/* Unspecified -> 0 -> Backwards compatible default */
	REGMAP_ENDIAN_DEFAULT = 0,
	REGMAP_ENDIAN_BIG,
	REGMAP_ENDIAN_LITTLE,
	REGMAP_ENDIAN_NATIVE,
};

struct remgap;
struct regmap_bus;

struct regmap_config {
	const char *name;
	int reg_bits;
	int val_bits;
	bool (*writeable_reg)(struct device *dev, unsigned int reg);
	bool (*readable_reg)(struct device *dev, unsigned int reg);
	bool (*volatile_reg)(struct device *dev, unsigned int reg);
	int (*reg_read)(void *context, unsigned int reg, unsigned int *val);
	int (*reg_write)(void *context, unsigned int reg, unsigned int val);
	unsigned int max_register;
	enum regcache_type cache_type;
	bool use_single_rw;
	bool use_single_read;
	bool use_single_write;
};

struct regmap *regmap_init(struct device *dev,
			   const struct regmap_bus *bus,
			   void *bus_context,
			   const struct regmap_config *config);

void regmap_exit(struct regmap *map);
int regmap_write(struct regmap *map, unsigned int reg, unsigned int val);
int regmap_read(struct regmap *map, unsigned int reg, unsigned int *val);

int regcache_sync(struct regmap *map);
int regcache_sync_region(struct regmap *map, unsigned int min,
			 unsigned int max);
void regcache_cache_only(struct regmap *map, bool enable);
void regcache_cache_bypass(struct regmap *map, bool enable);
void regcache_mark_dirty(struct regmap *map);

#endif /* __LINUX_REGMAP_H */
