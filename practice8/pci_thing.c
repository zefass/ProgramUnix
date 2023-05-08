
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>

#define PCI_VENDOR_ID 0x8086
#define PCI_DEVICE_ID 0x100e

#define DRIVER_NAME "pci_driver"
#define IOCTL_MAC_ADDR _IOR('mac', 1, char*)

MODULE_LICENSE("GPL");

static int rtl8139_probe(struct pci_dev *dev, const struct pci_device_id *id);
static void rtl8139_remove_one(struct pci_dev *dev);


static struct pci_device_id rtl8139_pci_tbl[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID, PCI_DEVICE_ID) },
	{0,}
};

MODULE_DEVICE_TABLE(pci, rtl8139_pci_tbl);

static struct pci_driver rtl8139_pci_driver = {
	.name = DRIVER_NAME,
	.id_table = rtl8139_pci_tbl,
	.probe = rtl8139_probe,
	.remove = rtl8139_remove_one
};

static int open(struct inode *inode, struct file *file);
static int release(struct inode *inode, struct file *file);
static long ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations pci_fops = {
	.open		=	open,
	.release	=	release,
	.unlocked_ioctl	=	ioctl
};

static int major;
unsigned char port_addr[6];
static struct class *cls;


static int open(struct inode *inode, struct file *file) {
    try_module_get(THIS_MODULE);
    printk(KERN_INFO "Network_card: Character device opened\n");
    return 0;
}

static int release(struct inode *inode, struct file *file) {
    module_put(THIS_MODULE);
    printk(KERN_INFO "Network_card: Character device closed\n");
    return 0;
}

static long ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case IOCTL_MAC_ADDR:
            if (copy_to_user((unsigned long __user *)arg, port_addr, 6) != 0) {
                return -EFAULT;
            }
            break;
        default:
            return -ENOTTY;
    }

    return 0;
}



static int rtl8139_init_module (void) {
	int pcireg = pci_register_driver(&rtl8139_pci_driver);
	if (pcireg < 0) {
		printk("Couldn't register pci driver. Do better!");
		return pcireg;
	}
	major = register_chrdev(0, DRIVER_NAME, &pci_fops);
	if (major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d, do better!\n", major);
		return major;
  	}
  	printk(KERN_INFO "I was assigned major number %d.", major); 	
  	cls = class_create(THIS_MODULE, DRIVER_NAME);
    	device_create(cls, NULL, MKDEV(major, 0), NULL, DRIVER_NAME);
    	
  	return pcireg;
}
int rtl8139_probe(struct pci_dev *dev, const struct pci_device_id *id) {
	int i;
	u8 __iomem *mmio_addr;

	mmio_addr = ioremap(pci_resource_start(dev, 0), pci_resource_len(dev, 0));
    	if (!mmio_addr) {
        	return -ENOMEM;
    	}
    	for (i = 0; i < 6; i++) {
    		port_addr[i] = ioread8(mmio_addr + i);
    		printk(KERN_INFO "Address number %d: %02x\n", i, mmio_addr[i]);
    	}
    	iounmap(mmio_addr);
    	return 0;
}

void rtl8139_remove_one(struct pci_dev *dev) {
	unregister_chrdev(major,"MyPCI");
}

void cleanup_module(void) {
	pci_unregister_driver(&rtl8139_pci_driver);
	device_destroy(cls, MKDEV(major, 0));
   	class_destroy(cls);
}
