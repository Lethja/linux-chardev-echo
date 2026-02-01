/*
 * This file is added at the end of the the the source file
 * to tell Linux where to enter the program and some module information.
 */

#include <linux/module.h>

/** Macro function that wraps `sysfs_emit()` for compatibility.
 * When building against Linux 4.11 or lower sysfs_emit is not available.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0)
#define compat_sysfs_emit(buf, fmt, ...) sysfs_emit(buf, fmt, ##__VA_ARGS__)
#else
#define compat_sysfs_emit(buf, fmt, ...) sprintf(buf, fmt, ##__VA_ARGS__)
#endif

module_init(echo_char_init);
module_exit(echo_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jason Lethbridge");
MODULE_DESCRIPTION("Character Device Echo Example");
MODULE_VERSION("1.0");

MODULE_PARM_DESC(fifo_size, "Echo buffer size in bytes");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)

static int fifo_size_get(char *buf, const struct kernel_param *kp)
{
	return compat_sysfs_emit(buf, "%u\n", fifo_size);
}

static const struct kernel_param_ops fifo_size_ops = {
	.get = fifo_size_get,
};

module_param_cb(fifo_size, &fifo_size_ops, &fifo_size, 0444);

#else

module_param(fifo_size, uint, 0444);

#endif
