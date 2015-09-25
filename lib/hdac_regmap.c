#include "../dist/sound/hda/hdac_regmap.c"

/*
 * regmap wrapper implementations
 */

/* dumb value management by linked list */
struct regmap_val;

struct regmap_val {
	int reg;
	int val;
	struct regmap_val *next;
};

struct regmap {
	struct device *dev;
	const struct regmap_bus *bus;
	void *bus_context;
	const struct regmap_config *config;
	bool cache_dirty;
	bool cache_only;
	bool cache_bypass;
	struct regmap_val *val;
	struct regmap_val *val_last;
};

struct regmap *regmap_init(struct device *dev,
			   const struct regmap_bus *bus,
			   void *bus_context,
			   const struct regmap_config *config)
{
	struct regmap *map;

	map = calloc(1, sizeof(*map));
	if (!map)
		exit(1);
	map->dev = dev;
	map->bus = bus;
	map->bus_context = bus_context;
	map->config = config;
	return map;
}

void regmap_exit(struct regmap *map)
{
	struct regmap_val *vp, *next;

	for (vp = map->val; vp; vp = next) {
		next = vp->next;
		free(vp);
	}

	free(map);
}

static struct regmap_val *cache_find_reg(struct regmap *map, unsigned int reg)
{
	struct regmap_val *vp;

	for (vp = map->val; vp; vp = vp->next) {
		if (vp->reg == reg)
			return vp;
	}
	return NULL;
}

static bool cache_read_reg(struct regmap *map, unsigned int reg, unsigned int *val)
{
	struct regmap_val *vp = cache_find_reg(map, reg);

	if (!vp)
		return false;
	*val = vp->val;
	return true;
}

static void cache_write_reg(struct regmap *map, unsigned int reg, unsigned int val)
{
	struct regmap_val *vp = cache_find_reg(map, reg);

	if (vp) {
		vp->val = val;
		return;
	}

	vp = malloc(sizeof(*vp));
	if (!vp)
		exit(1);
	vp->reg = reg;
	vp->val = val;
	vp->next = NULL;
	if (!map->val)
		map->val = vp;
	else
		map->val_last->next = vp;
	map->val_last = vp;
}

int regmap_write(struct regmap *map, unsigned int reg, unsigned int val)
{
	int err;

	if (!map->config->writeable_reg(map->dev, reg))
		return -EINVAL;
	if (map->cache_only)
		err = 0;
	else
		err = map->config->reg_write(map->bus_context, reg, val);
	if (!err && !map->cache_bypass &&
	    !map->config->volatile_reg(map->dev, reg))
		cache_write_reg(map, reg, val);
	return err;
}

int regmap_read(struct regmap *map, unsigned int reg, unsigned int *val)
{
	int err;

	if (!map->config->readable_reg(map->dev, reg))
		return -EINVAL;
	if (!map->cache_bypass && cache_read_reg(map, reg, val))
		return 0;
	if (map->cache_only)
		return -EBUSY;
	err = map->config->reg_read(map->bus_context, reg, val);
	if (!err && !map->cache_bypass &&
	    !map->config->volatile_reg(map->dev, reg))
		cache_write_reg(map, reg, *val);
	return err;
}

int regcache_sync(struct regmap *map)
{
	struct regmap_val *vp;
	int err;

	if (!map->cache_dirty)
		return 0;
	for (vp = map->val; vp; vp = vp->next) {
		if (!map->config->writeable_reg(map->dev, vp->reg))
			continue;
		err = map->config->reg_write(map->bus_context, vp->reg, vp->val);
		if (err < 0)
			return err;
	}
	map->cache_dirty = false;
	return 0;
}

int regcache_sync_region(struct regmap *map, unsigned int min,
			 unsigned int max)
{
	struct regmap_val *vp;
	int err;

	if (!map->cache_dirty)
		return 0;
	for (vp = map->val; vp; vp = vp->next) {
		if (vp->reg < min || vp->reg > max)
			continue;
		if (!map->config->writeable_reg(map->dev, vp->reg))
			continue;
		err = map->config->reg_write(map->bus_context, vp->reg, vp->val);
		if (err < 0)
			return err;
	}
	return 0;
}

void regcache_cache_only(struct regmap *map, bool enable)
{
	map->cache_only = enable;
}

void regcache_cache_bypass(struct regmap *map, bool enable)
{
	map->cache_bypass = enable;
}

void regcache_mark_dirty(struct regmap *map)
{
	map->cache_dirty = true;
}

