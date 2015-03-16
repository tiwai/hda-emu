#ifndef __LINUX_PM_RUNTIME_H
#define __LINUX_PM_RUNTIME_H

#include <linux/device.h>

static inline void pm_runtime_enable(struct device *dev) {}
static inline void pm_runtime_set_active(struct device *dev) {}
static inline void pm_runtime_set_autosuspend_delay(struct device *dev, int delay) {}
static inline void pm_runtime_use_autosuspend(struct device *dev) {}
static inline void pm_runtime_dont_use_autosuspend(struct device *dev) {}
static inline void pm_runtime_mark_last_busy(struct device *dev) {}

static inline bool pm_runtime_suspended(struct device *dev) { return dev->pmsuspended; }

static inline void pm_runtime_get_noresume(struct device *dev) { dev->pmcnt++; }
int pm_runtime_get_sync(struct device *dev);
int pm_runtime_put_autosuspend(struct device *dev);
int pm_runtime_force_suspend(struct device *dev);
int pm_runtime_force_resume(struct device *dev);
void pm_runtime_allow(struct device *dev);
void pm_runtime_forbid(struct device *dev);

#endif /* __LINUX_PM_RUNTIME_H */
