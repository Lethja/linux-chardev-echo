#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)

#define compat_kfifo_alloc(fifo, size, mode) kfifo_alloc(fifo, size, mode)

#else

#include <linux/kfifo.h>

int compat_kfifo_alloc(struct kfifo *fifo, unsigned int size, gfp_t mode)
{
        return -1;
}

int kfifo_is_empty(struct kfifo *fifo)
{
        return -1;
}

int kfifo_is_full(struct kfifo *fifo)
{
        return -1;
}

int kfifo_from_user(struct kfifo *fifo, const void *from, unsigned long len, unsigned int *copied)
{
        return -1;
}

int kfifo_to_user(struct kfifo *fifo, void *to, unsigned long len, unsigned int *copied)
{
        return -1;
}


#endif
