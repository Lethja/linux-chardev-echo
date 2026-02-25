While learning how to write a character device driver in the Linux kernel, 
I came across several contradictory examples on how to make character devices across the Internet.
After figuring it out, I created a small example project demonstrating each method
in an example more interesting than `printk()` but still less overwhelming than a real driver
so that others have an easier time learning in the future.

This repository contains three Linux kernel modules ([chrdev.c](chrdev.c), [cdev.c](cdev.c) and [miscdev.c](miscdev.c)), 
all of which create the same simple “echo” character device differently.
All modules are intended for learning purposes rather than doing something useful. 

You can see the differences in the code between the two modules by comparing them with `diff`. 
For example: 
```bash
diff -u chrdev.c cdev.c
```

All modules are valid ways of creating a character device,
the real question is which one fits a use case best.

Additionally, [common.c](common.c) and [common.h](common.h) implement buffer and file I/O logic including:
- A FIFO memory buffer so that multiple writes can concatenate
- Blocking and non-blocking I/O support including `epoll()` / `poll()` / `select()` support for programs to sleep and wait for readability/writability
- Configurable buffer size via a module parameter

The [module.h](module.h) header contains common but required footer boilerplate code between each module
to keep the character device code itself decluttered.

Compatibility macros and functions in [compat.h](compat.h) and [compat.c](compat.c) 
allow these modules to be built for modern and older kernel versions alike
by wrapping API differences behind a small shim.

The oldest kernel tested is 2.6.32 (that shipped with Debian 6 Squeeze)  
while the latest kernel tested is 6.19.3 (latest stable release at the time of writing).

All modules will create the same device node at `/dev/echo` when loaded (assuming udev is set up to do so).
Loading a module while another is loaded will result in an error
as the character device will already exist.

## Prerequisites

### Building
To build and load these kernel modules, some programs
from the systems distribution will need to be installed. 
The following command can be run to determine if the build environment is ready:
```bash
if which make gcc > /dev/null; then echo "Program OK"; fi; if [[ -f "${KDIR}/include/linux/kfifo.h" || -f "/lib/modules/$(uname -r)/build/include/linux/kfifo.h" ]]; then echo "Sources OK"; else echo "Sources NOT OK"; fi
```

#### GNU Make
On a GNU/Linux system, this is typically just called `make`.
On many distributions it is usually installed as part of a "build-essentials"
type meta-package; that is, if it isn't already installed. 
Refer to your distributions documentation for further details. 

#### GNU Compiler Collection (GCC)
This is the default C compiler on many GNU/Linux systems.
While GCC itself isn't a hard requirement for these modules specifically,
The kernel targets GNUC99 at the time of writing, 
so generally having a C compiler capable of these extensions is a good idea
when building kernel modules. 

#### Linux kernel sources

The kernel sources for the currently running system can often be found at 
`/lib/modules/$(shell uname -r)/build` 
or usally can be installed by distribution manager if it isn't already there.

It's also possible to build against another version of the kernel completely
by downloading and extracting the source tarball for of the desired version
from https://www.kernel.org/pub/linux/kernel/ 
and specifying its extraction path in the `KDIR` enviroment variable 
when calling `make`.

### Loading
The kernel on the intended system must be configured to support module loading
and to allow out-of-tree modules to be loaded.
```bash
cfg="$(zgrep -h -E '^(CONFIG_MODULES|CONFIG_MODULE_SIG_FORCE)=' /proc/config.gz 2>/dev/null || grep -h -E '^(CONFIG_MODULES|CONFIG_MODULE_SIG_FORCE)=' "/boot/config-$(uname -r)" 2>/dev/null)"; modok="$(printf '%s\n' "$cfg" | awk -F= '$1=="CONFIG_MODULES"{print $2}')"; sigforce="$(printf '%s\n' "$cfg" | awk -F= '$1=="CONFIG_MODULE_SIG_FORCE"{print $2}')"; disabled="$(cat /proc/sys/kernel/modules_disabled 2>/dev/null)"; lockdown="$(cat /sys/kernel/security/lockdown 2>/dev/null | tr -d '\n')"; if [ "$modok" = "y" ] && [ "$disabled" = "0" ] && [ "$sigforce" != "y" ] && ! printf '%s' "$lockdown" | grep -q '\[integrity\]\|\[confidentiality\]'; then echo insmod OK; fi
```
Additionally, if Secure Boot is being enforced, 
then the modules will require signing with an acceptable key 
before they can be loaded.

## Build

From the project directory:
```bash
make
```
To clean build artifacts:
```bash
make clean
```

## Load Module

Load a module with:
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

### Unload Module
Check which module is loaded (if any) with:
```bash
lsmod | grep echo
```
Unload the loaded module with:
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

### Logs
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
