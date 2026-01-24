#include "common.h"

unsigned int fifo_size = FIFO_SIZE_DEFAULT; /* The fifo_size parameter */

struct kfifo fifo;
DEFINE_MUTEX(fifo_lock);
DECLARE_WAIT_QUEUE_HEAD(fifo_queue);

static unsigned int echo_poll(struct file *file, poll_table *wait) 
{
	unsigned int mask = 0;

	/* Tell the kernel which wait queues this file should sleep on */
	poll_wait(file, &fifo_queue, wait);

	mutex_lock(&fifo_lock);

	if (!kfifo_is_empty(&fifo))
		mask |= POLLIN | POLLRDNORM;  /* There's something to read */
	if (!kfifo_is_full(&fifo))
		mask |= POLLOUT | POLLWRNORM; /* There's room to write */

	mutex_unlock(&fifo_lock);

	return mask;
}


static ssize_t echo_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) 
{
	int ret;
	unsigned int copied;

	if (mutex_lock_interruptible(&fifo_lock))
		return -ERESTARTSYS;

	while (kfifo_is_full(&fifo)) {
		mutex_unlock(&fifo_lock);

		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;

		if (wait_event_interruptible(fifo_queue, !kfifo_is_full(&fifo)))
			return -ERESTARTSYS;

		mutex_lock(&fifo_lock);
	}


	ret = kfifo_from_user(&fifo, buf, count, &copied);

	mutex_unlock(&fifo_lock);
	if (ret)
		return ret;

	if (copied)
		wake_up_interruptible(&fifo_queue);

	return copied;
}

static ssize_t echo_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int ret;
	unsigned int copied;

	if (mutex_lock_interruptible(&fifo_lock))
		return -ERESTARTSYS;

	if (kfifo_is_empty(&fifo)) {
		mutex_unlock(&fifo_lock);

		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;

		if (wait_event_interruptible(fifo_queue, !kfifo_is_empty(&fifo)))
			return -ERESTARTSYS;

		mutex_lock(&fifo_lock);
	}

	ret = kfifo_to_user(&fifo, buf, count, &copied);

	mutex_unlock(&fifo_lock);
	if (ret)
		return ret;

	if (copied)
		wake_up_interruptible(&fifo_queue);

	return copied;
}

static int echo_open(struct inode *inode, struct file *file)
{
	return 0; /* Nothing to initialize */
}

static int echo_release(struct inode *inode, struct file *file)
{
	return 0; /* Nothing to release */
}

const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = echo_open,
	.poll = echo_poll,
	.release = echo_release,
	.read = echo_read,
	.write = echo_write,
};

int allocate_buffer(uint size)
{
	/* Keep the buffer size aligned to a power of two as required by kfifo */
	size = roundup_pow_of_two(size);

	/* Avoid excessively big or small buffers */
	if (size < FIFO_SIZE_MIN)
		size = FIFO_SIZE_MIN;
	else if(size > FIFO_SIZE_MAX)
		size = FIFO_SIZE_MAX;

	/* Update sysfs value to the buffer length that will actually be used */
	fifo_size = size;

	/*
	 * Finally, allocate the kfifo buffer
	 *
	 * kfifo is used instead of kmalloc as it's a ring buffer that
	 * automatically allows writes to concatenate if it hasn't been
	 * read out yet.
	 */
	return kfifo_alloc(&fifo, size, GFP_KERNEL);
}
