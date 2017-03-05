obj-m += chardev.o
all:
	sudo rmmod chardev.ko	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod chardev.ko
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
ins:
	sudo insmod chardev.ko
rm:
	sudo rmmod chardev.ko

