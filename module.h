/*
 * This file is added at the end of the the the source file
 * to tell Linux where to enter the program and some module information.
 */

#include <linux/module.h>

module_init(echo_char_init);
module_exit(echo_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jason Lethbridge");
MODULE_DESCRIPTION("Character Device Echo Example");
MODULE_VERSION("1.0");

module_param(fifo_size, uint, 0444);
MODULE_PARM_DESC(fifo_size, "Echo buffer size in bytes");

