#include <linux/cdev.h>
#include <linux/device.h>

/* File I/O and buffer allocation logic are shared between both examples in this header and source file */
#include "common.h"

static dev_t devt;              /* Encodes major/minor */
static struct cdev echo_cdev;   /* Character device structure */
static struct class *cls;       /* Device class */

static int __init echo_char_init(void)
{
	int ret;
	struct device *dev;

	if ((ret = allocate_buffer(fifo_size)))
		goto err_print;

	if ((ret = alloc_chrdev_region(&devt, 0, 1, DEVICE_NAME)) < 0)
		goto err_fifo;

	cdev_init(&echo_cdev, &fops);
	echo_cdev.owner = THIS_MODULE;

	if ((ret = cdev_add(&echo_cdev, devt, 1)) < 0)
		goto err_chrdev;

	if (IS_ERR((cls = compat_class_create(CLASS_NAME)))) {
		ret = PTR_ERR(cls);
		goto err_cdev;
	}

	if (IS_ERR((dev = device_create(cls, NULL, devt, NULL, DEVICE_NAME)))) {
		ret = PTR_ERR(dev);
		goto err_class;
	}

	pr_info("Echo cdev module loaded as %u:%u\n", MAJOR(devt), MINOR(devt));
	return 0;

err_class:
	class_destroy(cls);

err_cdev:
	cdev_del(&echo_cdev);

err_chrdev:
	unregister_chrdev_region(devt, 1);

err_fifo:
	kfifo_free(&fifo);

err_print:
	pr_err("Loaded example echo character device module failed with %d", ret);

	return ret;
}

static void __exit echo_char_exit(void)
{
	device_destroy(cls, devt);
	class_destroy(cls);
	cdev_del(&echo_cdev);
	unregister_chrdev_region(devt, 1);
	kfifo_free(&fifo);
	printk(KERN_INFO "Unloaded example echo character device %u:%u\n", MAJOR(devt), MINOR(devt));
}

/* Module init, license, author, etc... are shared between both examples in this header */
#include "module.h"
