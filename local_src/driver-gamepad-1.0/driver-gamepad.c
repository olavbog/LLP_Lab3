/*
 * This is a demo Linux kernel module.
 */

#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/version.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

#include "efm32gg.h"

#define GPIO_SIZE 0x24 // number of bytes in a GPIO bank

static void __iomem *vgpio_pc;
static dev_t dev_num;
static struct cdev c_dev;
static struct class *cl;

static int status = 1, dignity  = 3, ego = 5;


/*
User program opens the driver
When the file is opened, initialize registers.
*/
static int gpio_open(struct inode *inode, struct file *filp){
	printk(KERN_INFO "GPIO_OPEN Woooo");
	return 0;
}

// User program closes the driver
static int gpio_release(struct inode *inode, struct file *filp){
	printk(KERN_INFO "GPIO_Close Woo? wait yes, maybe no");
	return 0;
}

/* 
User program reads from the driver
ssize_t is 16 bit signed. 
len is lenght of buffer
use static variable as buffer
e.g static char c;
*/
static ssize_t gpio_read(struct file *filp, char __user *buff, size_t len, loff_t *offp){
	printk(KERN_INFO "Read me baby");
	uint32_t data = ioread32(vgpio_pc+0x1c);
	copy_to_user(vuff,&data,1);
	return count; // return number of bytes read
}

// User program writes to the driver
static ssize_t gpio_write(struct file *filp, cont char __user *buff, size_t len, loff_t *offp){
	printk(KERN_INFO "Write me a book, or go away. Also these are buttons, wtf do you want?");
	return count; //return the number of bytes written. Standard write response
} 
static int gpio_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	query_arg_t q;

	switch(cmd)
	{
		case QUERY_GET_VARIABLES:
			q.status = status;
			q.dignity = dignity;
			q.ego = ego;

			if(copy_to_user((query_arg_t *)arg, &q, sizeof(query_arg_t)))
			{
				return -EACCES;
			}
			break;
		case QUERY_CLR_VARIABLES:
			status = 0;
			dignity = 0;
			ego = 0;
			break;
		default:
			return -EINVAL;
	}
	return 0;
}



struct file_operations gpio_fops = {
	.owner   = THIS_MODULE,
	.open    = gpio_open,
	.release = gpio_release, 
	.read    = gpio_read,
	.write   = gpio_write,
	
};



/*
 * template_init - function to insert this module into kernel space
 *
 * This is the first of two exported functions to handle inserting this
 * code into a running kernel
 *
 * Returns 0 if successfull, otherwise -1
 */

static int __init template_init(void)
{

	/* Use kmalloc(size_t size, int flags);
		Allocates size bytes of memory/ Returns a pointer to that memory or null if the allocation fails
	*/
	int ret;
	struct device *dev_ret;


	printk(KERN_INFO "Initializing driver");
	if((ret = alloc_chrdev_region(&dev_num, 0, 1, "GPIO_driver")<0))
	{
		printk(KERN_ALERT "Failed to allocate deivce")
		return ret;
	}
	printk(KERN_INFO "<Major, Minor>: <%d, %d> \n", MAJOR(dev_num),MINOR(dev_num));

	// Requesting the memory regions we will write and read from
	if(request_mem_region(GPIO_PC_MODEL, 1, "GPIO_PC") == NULL)
	{
		printk(KERN_ALERT "ERROR requesting MODEL\n")
		return -1;
	}
	if(request_mem_region(GPIO_PC_DOUT, 1, "GPIO_PC") == NULL)
	{
		printk(KERN_ALERT "ERROR requesting MODEL\n")
		return -1;
	}
	if(request_mem_region(GPIO_PC_DIN, 1, "GPIO_PC") == NULL)
	{
		printk(KERN_ALERT "ERROR requesting MODEL\n")
		return -1;
	}


	if((vgpio_pc = ioremap_nocache(GPIO_PC_BASE, GPIO_SIZE))== NULL){
			printk(KERN_ERR "Mapping of GPIO failed\n");
			return -ENOMEM;
	}

	iowrite32(0x33333333, vgpio_pc+0x04); //MODEL
	iowrite32(0xFF, vgpio_pc+0x0c); //DOUT


	// Add device/ create device files for user mode
	if(IS_ERR(cl = class_create(THIS_MODULE, "chardrv"))){
		unregister_chrdev_region(dev_num, 1);
		return PTR_ERR(cl);
	}

	if(IS_ERR(dev_ret = device_create(cl, NULL, dev_num, NULL, "buttons"))){
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
		return PTR_ERR(dev_ret);
	}

	cdev_init(&c_dev, &gpio_fops);
	if((ret = cdev_add(&c_dev, dev_num, 1))<0){
		device_destroy(cl, dev_num);
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
		return ret;
	}
	printk(KERN_INFO "Driver initiated successfully")


	return 0;
}

/*
 * template_cleanup - function to cleanup this module from kernel space
 */

static void __exit template_cleanup(void)
{
	printk(KERN_INFO "Killing driver");

	release_mem_region(GPIO_PC_MODEL,1);
	release_mem_region(GPIO_PC_DOUT,1);
	release_mem_region(GPIO_PC_DIN,1);

	//unregister device
	cdev_del(&c_dev);
	device_destroy(cl, dev_num);
	class_destroy(cl);

	iounmap(vgpio_pc);
	unregister_chrdev_region(dev_num, 1);
	

}

module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Amazingness, I am god");
MODULE_LICENSE("GPL");




