#include <linux/kfifo.h>
#include <linux/log2.h>
#include <linux/mutex.h>
#include <linux/poll.h>

#define DEVICE_NAME "echo"           /* Device name */
#define CLASS_NAME "echo_class"      /* Class name */
#define CHRDEV_NAME "echo_dev"       /* Character device register name */
#define FIFO_SIZE_DEFAULT 1024       /* Default and minimum FIFO size in bytes */
#define FIFO_SIZE_MAX 1048576        /* Maximum possible FIFO size in bytes */

extern struct kfifo fifo;            /* The FIFO to use as a temporary buffer */
extern struct mutex fifo_lock;       /* The FIFO mutex */
extern unsigned int fifo_size;       /* The fifo_size parameter */
extern wait_queue_head_t fifo_queue; /* The queue to wake up poll listeners with */

extern const struct file_operations fops;

extern int allocate_buffer(uint size);
