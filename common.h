#include <linux/kfifo.h>
#include <linux/log2.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/version.h>

#define DEVICE_NAME        "echo"  /* Device name */
#define CLASS_NAME   "echo_class"  /* Class name */
#define FIFO_SIZE_DEFAULT    1024  /* Default FIFO size in bytes */
#define FIFO_SIZE_MIN          64  /* Minimum FIFO size in bytes */
#define FIFO_SIZE_MAX     1048576  /* Maximum FIFO size in bytes */

extern struct kfifo fifo;            /* The FIFO to use as a temporary buffer */
extern struct mutex fifo_lock;       /* The FIFO mutex */
extern unsigned int fifo_size;       /* The fifo_size module parameter */
extern wait_queue_head_t fifo_queue; /* The queue to wake up poll listeners with */

extern const struct file_operations fops;

/**
 * Wrapper around kfifo_alloc that updates fifo_size parameter at the same time
 * @param size The size to allocate on the buffer in bytes
 * @return The kfifo_alloc return
 */
extern int allocate_buffer(uint size);

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

