/*
 * This header contains wrappers around different ways that the kfifo functions changes throughout the kernels history
 * Some helper functions aren't avalible in older kernels are reimplemented
 */

/**
 * Macro function that wraps `class_create()` for compatiibilty.
 * When building against Linux 6.3 or lower class_create must specify `THIS_MODULE`.
 * On later versions of the kernel only the class name is needed.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
#define compat_class_create(name) class_create(name)
#else
#define compat_class_create(name) class_create(THIS_MODULE, name)
#endif

/** Macro function that wraps `sysfs_emit()` for compatibility.
 * When building against Linux 4.11 or lower sysfs_emit is not available.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0)
#define compat_sysfs_emit(buf, fmt, ...) sysfs_emit(buf, fmt, ##__VA_ARGS__)
#else
#define compat_sysfs_emit(buf, fmt, ...) sprintf(buf, fmt, ##__VA_ARGS__)
#endif

