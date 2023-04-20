#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>

MODULE_LICENSE("GPL");

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);

#define SUCCESS 0
#define DEVICE_NAME "chardev"
#define BUF_LEN 1024
#define IOCTL_RESET _IOR(Major, 0, int)

static int Major;
static int Device_Open = 0;

static char msg[BUF_LEN];
static char *msg_rPtr;
static char *msg_wPtr;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release
};

int init_module(void)
{
        Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
	  printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	  return Major;
	}

	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return SUCCESS;
}

void cleanup_module(void)
{
	unregister_chrdev(Major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_rPtr = msg;
	msg_wPtr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;
	module_put(THIS_MODULE);

	return 0;
}

static ssize_t device_read(struct file *filp,
			   char *buffer,
			   size_t length,
			   loff_t * offset)
{
	int bytes_read = 0;
	if (*msg_rPtr == 0)
		return 0;
	while (length && *msg_rPtr) {
		put_user(*(msg_rPtr++), buffer++);

		length--;
		bytes_read++;
	}

	return bytes_read;
}

static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	int i;
    	for(i=0; i<len && i<BUF_LEN; i++)
    	get_user(msg[i], buff+i);
    	msg_wPtr = msg;
    	return i;
}
long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param){
    int i;
 
    printk(KERN_INFO "Device IOCTL");
    char buf[BUF_LEN];
    for (i = 0; i < BUF_LEN; i++)
    	buf[i] = ' ';
            
    device_write(file, buf, i, 0);
    
    return 0;
}
