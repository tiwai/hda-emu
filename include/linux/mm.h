#ifndef __LINUX_MM_H
#define __LINUX_MM_H

typedef int mm_segment_t;
#define KERNEL_DS	0
#define USER_DS		1
static inline void set_fs(mm_segment_t fs)	{}
#define get_ds()	KERNEL_DS
#define get_fs()	KERNEL_DS

#endif
