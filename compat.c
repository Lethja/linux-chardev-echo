#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)

#include <linux/kfifo.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/log2.h>

static inline unsigned int compat_kfifo_len(struct kfifo *fifo)
{
	return fifo->in - fifo->out;
}

static inline unsigned int compat_kfifo_avail(struct kfifo *fifo)
{
	return fifo->size - compat_kfifo_len(fifo);
}

static inline unsigned int compat_kfifo_in_bytes(struct kfifo *fifo, const unsigned char *src, unsigned int len)
{
	unsigned int l;
	unsigned int off;

	if (!len)
		return 0;

	len = min(len, compat_kfifo_avail(fifo));

	off = fifo->in & (fifo->size - 1);
	l = min(len, fifo->size - off);

	memcpy(fifo->buffer + off, src, l);
	memcpy(fifo->buffer, src + l, len - l);

	/*
	 * Paired with reader updating fifo->out. If you need strict SMP safety
	 * beyond single-producer/single-consumer, add proper barriers/locking.
	 */
	fifo->in += len;

	return len;
}

static inline unsigned int compat_kfifo_out_bytes(struct kfifo *fifo, unsigned char *dst, unsigned int len)
{
	unsigned int l;
	unsigned int off;

	if (!len)
		return 0;

	len = min(len, compat_kfifo_len(fifo));

	off = fifo->out & (fifo->size - 1);
	l = min(len, fifo->size - off);

	memcpy(dst, fifo->buffer + off, l);
	memcpy(dst + l, fifo->buffer, len - l);

	fifo->out += len;

	return len;
}

int compat_kfifo_alloc(struct kfifo *fifo, unsigned int size, gfp_t mode)
{
	unsigned int sz;

	if (!fifo || !size)
		return -EINVAL;

	sz = roundup_pow_of_two(size);
	if (sz < 2)
		sz = 2;

	fifo->buffer = kmalloc(sz, mode);
	if (!fifo->buffer)
		return -ENOMEM;

	fifo->size = sz;
	fifo->in = 0;
	fifo->out = 0;
	fifo->lock = NULL;

	return 0;
}

void compat_kfifo_free(struct kfifo *fifo)
{
	if (!fifo)
		return;

	kfree(fifo->buffer);
	fifo->buffer = NULL;
	fifo->size = 0;
	fifo->in = 0;
	fifo->out = 0;
}

int kfifo_is_empty(struct kfifo *fifo)
{
	return !fifo || (fifo->in == fifo->out);
}

int kfifo_is_full(struct kfifo *fifo)
{
	if (!fifo)
		return 0;

	return compat_kfifo_len(fifo) >= fifo->size;
}

int kfifo_from_user(struct kfifo *fifo, const void __user *from, unsigned long len, unsigned int *copied)
{
	unsigned long done = 0;
	unsigned char *tmp;
	unsigned int chunk_max = PAGE_SIZE;

	if (copied)
		*copied = 0;

	if (!fifo || !from)
		return -EINVAL;

	if (!len)
		return 0;

	tmp = kmalloc(chunk_max, GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	while (done < len) {
		unsigned int avail = compat_kfifo_avail(fifo);
		unsigned int want = len - done;
		unsigned int chunk;

		if (!avail)
			break;

		if (want > avail)
			want = avail;

		chunk = want;
		if (chunk > chunk_max)
			chunk = chunk_max;

		if (copy_from_user(tmp, (const unsigned char __user *)from + done, chunk)) {
			kfree(tmp);
			return -EFAULT;
		}

		done += compat_kfifo_in_bytes(fifo, tmp, chunk);
	}

	kfree(tmp);
	if (copied)
		*copied = done;

	return 0;
}

int kfifo_to_user(struct kfifo *fifo, void __user *to, unsigned long len, unsigned int *copied)
{
	unsigned long done = 0;
	unsigned char *tmp;
	unsigned int chunk_max = PAGE_SIZE;

	if (copied)
		*copied = 0;

	if (!fifo || !to)
		return -EINVAL;

	if (!len)
		return 0;

	tmp = kmalloc(chunk_max, GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	while (done < len) {
		unsigned int have = compat_kfifo_len(fifo);
		unsigned int want = len - done;
		unsigned int chunk;
		unsigned int pulled;

		if (!have)
			break;

		if (want > have)
			want = have;

		chunk = want;
		if (chunk > chunk_max)
			chunk = chunk_max;

		pulled = compat_kfifo_out_bytes(fifo, tmp, chunk);
		if (!pulled)
			break;

		if (copy_to_user((unsigned char __user *)to + done, tmp, pulled)) {
			kfree(tmp);
			return -EFAULT;
		}

		done += pulled;
	}

	kfree(tmp);
	if (copied)
		*copied = done;

	return 0;
}

#endif
