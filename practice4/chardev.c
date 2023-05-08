#include <linux/module.h>	
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/stat.h>

 
MODULE_LICENSE("GPL");

#define TRUE 1
#define FALSE 0
#define BUF_LEN 1024

#define DEVICE_NAME "chardev"

int init_module(void);
void cleanup_module(void);
static int open(struct inode *, struct file *);
static int release(struct inode *, struct file *);
static ssize_t read(struct file *, char __user *, size_t, loff_t *);

static int counter = 0;
static int Major;
static bool thr_lock = FALSE;
static DECLARE_WAIT_QUEUE_HEAD(wq);
static char msg[BUF_LEN]; 
static char *msg_Ptr;

struct class *cls;

struct task_struct *ts;
int thread(void *data) {

  while(1) {
      msleep(100);
      sprintf(msg, "%d\n", counter);
      msg_Ptr = msg;
      if (thr_lock == FALSE) {
      	    counter++;
            printk("Hello. I am kernel thread! counter=%d\n", counter);
            msleep(1000);
            thr_lock = TRUE;
            wake_up_interruptible(&wq);
      } 
  }
  return 0;
}
ssize_t read(struct file *filp, char *buffer, size_t length, loff_t *offset) 
{
    printk(KERN_INFO "Inside read\n");
    printk(KERN_INFO "Scheduling Out\n");
    wait_event_interruptible(wq, thr_lock == TRUE);
    thr_lock = FALSE;
    printk(KERN_INFO "Woken Up\n");
    copy_to_user(buffer, msg, BUF_LEN);
    return length;
}
static ssize_t write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset) {
	int bytes_read = 0;
	char buf[32];
	int len = sprintf(buf, "%d\n", counter);

	if (len > length) {
		return -EINVAL;
	}
	if (copy_to_user(buffer, buf, len)) {
		return -EINVAL;
	}
	bytes_read = len;
	printk("Counter: Read %d bytes, counter = %d", bytes_read, counter);
	return bytes_read;
}

int open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "Inside open\n");
    try_module_get(THIS_MODULE);
    return 0;
}

int release(struct inode *inode, struct file *filp) 
{
    printk (KERN_INFO "Inside close\n");
    module_put(THIS_MODULE);
    return 0;
}

struct file_operations pra_fops = {
    read:        read,
    write:       write,
    open:        open,
    release:     release
};

int init_module(void) {
  Major = register_chrdev(0, DEVICE_NAME, &pra_fops);
  if (Major < 0) {
	printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	return Major;
  }
  printk(KERN_INFO "I was assigned major number %d.", Major);
  cls = class_create(THIS_MODULE, DEVICE_NAME);
  device_create(cls, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);
  ts=kthread_run(thread,NULL,"foo kthread");
  return 0;
}
void cleanup_module(void) {
  unregister_chrdev(Major, DEVICE_NAME);
  device_destroy(cls, MKDEV(Major, 0));
  class_destroy(cls);
  printk(KERN_INFO "cleanup_module() called\n");
  kthread_stop(ts);
}
