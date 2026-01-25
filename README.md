
A small Linux kernel module project that builds **two example “echo” character device drivers**. Both expose a `/dev/echo` device node that behaves like a simple byte buffer:

- data written to the device can be read back later (FIFO semantics)
- supports blocking and non-blocking I/O
- supports `poll()` / `select()` to wait for readability/writability
- buffer size is configurable via a module parameter

This is intended for learning/demonstration purposes.

## What gets built

The Makefile builds two separate kernel modules:

- `echo-chrdev.ko` — registers a character device via the “classic” `register_chrdev()` approach
- `echo-cdev.ko` — registers via `alloc_chrdev_region()` + `cdev_add()`

They both create the same device node name (`/dev/echo`). Load **only one at a time**.

## Prerequisites

- Linux system with a kernel that supports out-of-tree module builds
- Kernel headers / build tools installed for your running kernel (e.g. `linux-headers-$(uname -r)` on Debian/Ubuntu)
- `make`, a C compiler toolchain

Secure Boot note: on many systems, Secure Boot requires signing kernel modules before they can be loaded.

## Build

From the project directory:
```
bash
make
```
To clean build artifacts:
```
bash
make clean
```
## Load / unload

Load **one** module:
```
bash
sudo insmod echo-chrdev.ko
# or:
sudo insmod echo-cdev.ko
```
Unload:
```
bash
sudo rmmod echo_chrdev
# or:
sudo rmmod echo_cdev
```
Check kernel logs:
```
bash
dmesg | tail -n 50
```
## Using the device

After loading, you should have:

- `/dev/echo`

### Basic echo test

Write some data:
```
bash
echo "hello" | sudo tee /dev/echo >/dev/null
```
Read it back:
```
bash
sudo cat /dev/echo
```
### Non-blocking behavior

If you open the device in non-blocking mode and try to read while empty (or write while full), it will return immediately with `EAGAIN`.

### poll/select

The device supports `poll()`/`select()`:
- readable when the internal buffer has data
- writable when the internal buffer has free space

This makes it suitable for simple event-driven experiments.

## Module parameter: `fifo_size`

The internal buffer size can be set at module load time:
```
bash
sudo insmod echo-cdev.ko fifo_size=4096
```
The effective size is clamped to a reasonable range and rounded to a power of two.

You can read the active value via sysfs:
```
bash
cat /sys/module/echo_cdev/parameters/fifo_size
# or if using the other module:
cat /sys/module/echo_chrdev/parameters/fifo_size
```
(Only readable at runtime in this project.)

## Notes / troubleshooting

- If `/dev/echo` doesn’t appear, check `dmesg` for errors.
- If you see “File exists” / device-node conflicts, ensure you didn’t load both modules at once and that the previous one is unloaded.
- If `insmod` fails with “Operation not permitted” on a Secure Boot system, you may need to sign the module or disable Secure Boot for development.

## License

GPL-2.0 (kernel module).
