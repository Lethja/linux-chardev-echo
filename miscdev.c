#include <linux/miscdevice.h>

/* File I/O and buffer allocation logic are shared between both examples in this header and source file */
#include "common.h"

#define MISC_MAJOR 10 /* Misc devices are always hardcoded to major device 10 */

static struct miscdevice misc_dev = {
	.minor = MISC_DYNAMIC_MINOR, /* Defined in linux/miscdevice.h */
	.name  = DEVICE_NAME,        /* Defined in common.h */
	.fops  = &fops,              /* Defined in common.h */
};

static int __init echo_char_init(void)
{
	int ret;

	if ((ret = allocate_buffer(fifo_size)))
		goto err_print;

	if ((ret = misc_register(&misc_dev)))
		goto err_fifo;

	pr_info("Echo miscdev module loaded as %d:%d\n", MISC_MAJOR, misc_dev.minor);
	return 0;

err_fifo:
	free_buffer(&fifo);

err_print:
	pr_err("Loaded example echo character device module failed with %d", ret);

	return ret;
}

static void __exit echo_char_exit(void)
{
	misc_deregister(&misc_dev);
	free_buffer(&fifo);
	printk(KERN_INFO "Unloaded example echo character device %d:%d\n", MISC_MAJOR, misc_dev.minor);
}

/* Module init, license, author, etc... boilerplate are shared between all examples in this header */
#include "module.h"
