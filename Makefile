# You can set this KDIR environment variable to target different Linux sources than the ones this system is currently running.
KDIR ?= /lib/modules/$(shell uname -r)/build

# The names of the modules to build (.o -> .ko)
obj-m += echo-chrdev.o echo-cdev.o echo-miscdev.o

# The sources used in each module (.c -> .o)
echo-chrdev-objs := chrdev.o common.o
echo-cdev-objs := cdev.o common.o
echo-miscdev-objs := miscdev.o common.o

# Compile and link all kernel modules
all:
	make -C $(KDIR) M=$(CURDIR) modules

# Clean up build objects and kernel modules
clean:
	make -C $(KDIR) M=$(CURDIR) clean
