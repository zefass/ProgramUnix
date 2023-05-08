CONFIG_MODULE_SIG=n

obj-m += pci_thing.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod pci_thing.ko
	sudo dmesg
clean:
	sudo rmmod pci_thing.ko
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

