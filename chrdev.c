#include <linux/device.h>

#include "common.h"

#define CHR_DEV_NAME    "echo_dev"  /* Character device register name */

static int major = 0;      /* Major number for the echo device */
static struct class *cls;  /* Device class */

static int __init echo_char_init(void)
{
	int ret;
	struct device *dev;

	if ((ret = allocate_buffer(fifo_size)))
		goto err_print;

	if ((major = register_chrdev(0, CHR_DEV_NAME, &fops)) < 0)
		goto err_fifo;

	if (IS_ERR((cls = class_create(CLASS_NAME)))) {
		ret = PTR_ERR(cls);
		goto err_chrdev;
	}

	if (IS_ERR((dev = device_create(cls, NULL, MKDEV(major,0), NULL, DEVICE_NAME)))) {
		ret = PTR_ERR(dev);
		goto err_class;
	}

	pr_info("Loaded example echo character device module and registered as %d:0\n", major);
	return 0;

err_class:
	class_destroy(cls);

err_chrdev:
	unregister_chrdev(major, CHR_DEV_NAME);

err_fifo:
	kfifo_free(&fifo);

err_print:
	pr_err("Loaded example echo character device module failed with %d", ret);

	return ret;
}

static void __exit echo_char_exit(void)
{
	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, CHR_DEV_NAME);
	kfifo_free(&fifo);
	printk(KERN_INFO "Unloaded example echo character device %d:0\n", major);
}

#include "module.h"
