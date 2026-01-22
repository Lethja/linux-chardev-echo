obj-m += echo-chrdev.o echo-cdev.o
echo-chrdev-objs := chrdev.o common.o
echo-cdev-objs := cdev.o common.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
