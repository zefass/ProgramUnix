
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>

#define PCI_VENDOR_ID 0x8086
#define PCI_DEVICE_ID 0x100e

#define DRIVER_NAME "pci_driver"
#define DEVICE_NAME "pci_device" 
#define IOCTL_MAC_ADDR _IOR('mac', 1, char*)

MODULE_LICENSE("GPL");

static int open(struct inode *, struct file *);
static int release(struct inode *, struct file *); 
static long ioctl(struct file *filep, unsigned int cmd, unsigned long arg);
static int rtl8139_probe(struct pci_dev *dev, const struct pci_device_id *id);
static void rtl8139_remove_one(struct pci_dev *dev);

unsigned char port_addr[6];
static char *port_Ptr; 
static int major;
void __iomem *mmio_addr;
u8 *port_ptr;

static struct pci_device_id rtl8139_pci_tbl[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID, PCI_DEVICE_ID) },
	{ 0, }
};

static struct file_operations pci_fops = {
	.unlocked_ioctl = ioctl,
	.open = open,
	.release = release
};


static struct pci_driver rtl8139_pci_driver = {
	.name = DRIVER_NAME,
	.id_table = rtl8139_pci_tbl,
	.probe = rtl8139_probe,
	.remove = rtl8139_remove_one
};


static int open(struct inode *inode, struct file *file) { 
	port_Ptr = port_addr;
	printk(KERN_INFO "Open port address: %pM\n", port_addr); 
	try_module_get(THIS_MODULE);

	return 0;
}

static int release(struct inode *inode, struct file *file) { 
	module_put(THIS_MODULE);
    	iounmap(mmio_addr);
	return 0;
}

static long ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    	switch (cmd) {
        	case IOCTL_MAC_ADDR:
            		if (copy_to_user((char *)arg, port_addr, 6)) {
                		return -EFAULT;
            		}
			break;
        	default:
            		return -ENOTTY;
	}
	return 0;
}

int get_port_address(struct pci_dev *dev, unsigned char *port_addr) {
    mmio_addr = ioremap(pci_resource_start(dev, 0), pci_resource_len(dev, 0));
    if (!mmio_addr) {
        return -ENOMEM;
    }
    port_ptr = (u8 *)(mmio_addr + 0x0000);
    memcpy(port_addr, port_ptr, 6);

    return 0;
}

int rtl8139_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	
	printk(KERN_INFO "Probe\n");

	if (get_port_address(dev, port_addr)) {
		printk(KERN_ERR "Unable to get port address\n");
		return -EINVAL;
	}

	printk(KERN_INFO "port address: %pM\n", port_addr);

	major = register_chrdev(0, DEVICE_NAME, &pci_fops);
	if (major < 0) { 
		printk(KERN_ALERT "Registering char device failed with %d, do better!\n", major); 	
		return major; 
	} 
	printk(KERN_INFO "try 'sudo mknod /dev/%s c %d 0'.\n", DEVICE_NAME, major); 

	return 0;
}

void rtl8139_remove_one(struct pci_dev *dev)
{
	unregister_chrdev(major, DEVICE_NAME);	
}

int init_module(void)
{	
	printk(KERN_INFO "Init\n");
	int pcireg = pci_register_driver(&rtl8139_pci_driver);
	if (pcireg < 0) {
		printk("Couldn't register pci driver. Do better!");
		return pcireg;
	}
	return pcireg;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Exit\n");
	pci_unregister_driver(&rtl8139_pci_driver);
}
