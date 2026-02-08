#include <linux/version.h>

/*
 * This header contains wrappers around different ways that the kfifo functions changes throughout the kernels history
 * Some helper functions aren't available in older kernels are reimplemented
 */

/**
 * Macro function that wraps `class_create()` for compatibility.
 * When building against Linux 6.3 or lower class_create must specify `THIS_MODULE`.
 * On later versions of the kernel only the class name is needed.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
#define compat_class_create(name) class_create(name)
#else
#define compat_class_create(name) class_create(THIS_MODULE, name)
#endif

/**
 * Macro function that wraps `sysfs_emit()` for compatibility.
 * When building against Linux 4.11 or lower sysfs_emit is not available.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0)
#define compat_sysfs_emit(buf, fmt, ...) sysfs_emit(buf, fmt, ##__VA_ARGS__)
#else
#define compat_sysfs_emit(buf, fmt, ...) sprintf(buf, fmt, ##__VA_ARGS__)
#endif

/**
 * Macro function that wraps `kfifo_alloc()` for compatibility.
 * When building against Linux 2.6.32 or lower kfifo_alloc has different parameters and return value.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)

#define compat_kfifo_alloc(fifo, size, mode) kfifo_alloc(fifo, size, mode)
#define compat_kfifo_free(fifo) kfifo_free(fifo)

#else

#include <linux/kfifo.h>

extern int compat_kfifo_alloc(struct kfifo *fifo, unsigned int size, gfp_t mask);

extern void compat_kfifo_free(struct kfifo *fifo);

extern int kfifo_is_empty(struct kfifo *fifo);

extern int kfifo_is_full(struct kfifo *fifo);

extern int kfifo_from_user(struct kfifo *fifo, const void *from, unsigned long len, unsigned int *copied);

extern int kfifo_to_user(struct kfifo *fifo, void *to, unsigned long len, unsigned int *copied);

#endif
