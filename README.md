While learning how to write a character device driver in the Linux kernel, 
I came across several contradictory examples on how to make character devices across the Internet.
After figuring it out, I created a small example project demonstrating each method
in an example more interesting than `printk()` but still less overwhelming than a real driver
so that others have an easier time learning in the future.

This repository contains three Linux kernel modules (`chrdev.c`, `cdev.c` and `miscdev.c`), 
all of which create the same simple “echo” character device differently.
All modules are intended for learning purposes rather than doing something useful. 

You can see the differences in the code between the two modules by comparing them with `diff`. 
For example: 
```bash
diff -u chrdev.c cdev.c
```

All modules are valid ways of creating a character device,
the real question is which one fits a use case best.

Additionally, `common.c` implements buffer and file I/O logic including:
- A FIFO buffer in the kernel
- Blocking and non-blocking I/O support
- `epoll()` / `poll()` / `select()` support so user programs can wait for readability/writability
- Configurable buffer size via a module parameter

All modules will create the same device node at `/dev/echo` when loaded (assuming udev is set up to do so).
Loading a module while another is loaded will result in an error
as the character device will already exist.

## Prerequisites

To build and load these kernel modules, you will need:
- Linux system with a kernel that supports out-of-tree module builds
- Kernel headers
- `make` & `gcc`

Note: Secure Boot requires signing kernel modules before they can be loaded.

## Build

From the project directory:
```bash
make
```
To clean build artifacts:
```bash
make clean
```

## Load / unload

Load a module:
```bash
insmod echo-chrdev.ko
```
or
```bash
insmod echo-cdev.ko
```
or
```bash
insmod echo-miscdev.ko
```

Unload:
```bash
rmmod echo_chrdev
```
or
```bash
rmmod echo_cdev
```
or
```bash
rmmod echo_miscdev
```

Check kernel logs with:
```bash
dmesg
```

### Basic echo test

Write some data:
```bash
echo "Hello Character Device!" > /dev/echo
```

Read it back:
```bash
cat /dev/echo
```

`cat` will keep waiting for more data after it drains the buffer.
Press `Ctrl + C` to stop it or use the timeout command to stop it automatically:
```bash
timeout 5 cat /dev/echo
```

## Module parameters

The internal buffer size can be set at module load time:
```bash
insmod echo-chrdev.ko fifo_size=4096
```
or
```bash
insmod echo-cdev.ko fifo_size=4096
```
or
```bash
insmod echo-miscdev.ko fifo_size=4096
```

The buffer size can be anywhere between 64 bytes to 1MB 
and will automatically be rounded up to the nearest power of two.

Once a module has been loaded `fifo_size` cannot be set again 
without unloading the module first, but the current value can be read via sysfs:
```bash
cat /sys/module/echo_chrdev/parameters/fifo_size
```
or
```bash
cat /sys/module/echo_cdev/parameters/fifo_size
```
or
```bash
cat /sys/module/echo_miscdev/parameters/fifo_size
```

## Notes / troubleshooting

- Commands like `insmod`, `rmmod` and other programs reading from/writing to `/dev` files usually need to be run as root (or with `sudo`) on many Linux distributions. 
- If `/dev/echo` doesn’t appear, check `dmesg` for errors.
- If you see “File exists” / device-node conflicts, ensure you didn’t already load another module and unload it if you did.
- If `insmod` fails with “Operation not permitted” on a Secure Boot system, you may need to sign the module or disable Secure Boot for development.

## License

GPL-2.0 (kernel module).
