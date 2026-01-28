KDIR ?= /lib/modules/$(shell uname -r)/build
obj-m += echo-chrdev.o echo-cdev.o echo-miscdev.o
echo-chrdev-objs := chrdev.o common.o
echo-cdev-objs := cdev.o common.o
echo-miscdev-objs := miscdev.o common.o

all:
	make -C $(KDIR) M=$(CURDIR) modules

clean:
	make -C $(KDIR) M=$(CURDIR) clean
