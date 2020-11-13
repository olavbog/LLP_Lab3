/*
 * This is a demo Linux kernel module.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/moduleparam.h>
#include <linux/kdev_t.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/signal.h>
#include <asm/siginfo.h>
#include <linux/interrupt.h>

#include "efm32gg.h"

#define GPIO_SIZE 0x24 // number of bytes in a GPIO bank
#define REG_SIZE  4

// static void __iomem *vgamepad_pc;
static dev_t dev_num;
static struct cdev c_dev;
static struct class *cl;

uint8_t button_action = 8;

/*
User program opens the driver
When the file is opened, initialize registers.
*/
static int gamepad_open(struct inode *inode, struct file *filp){
	printk(KERN_INFO "GPIO_OPEN Woooo\n");
	return 0;
}

// User program closes the driver
static int gamepad_release(struct inode *inode, struct file *filp){
	printk(KERN_INFO "GPIO_Close Woo? wait yes, maybe no\n");
	return 0;
}

/* 
User program reads from the driver
ssize_t is 16 bit signed. 
len is lenght of buffer
use static variable as buffer
e.g static char c;
*/
static ssize_t gamepad_read(struct file *filp, char __user *buff, size_t len, loff_t *offp){
	printk(KERN_INFO "I was read\n");
	// uint8_t data = ioread8(GPIO_PC_DIN);
	uint8_t data = button_action;
	printk(KERN_INFO "Has something happened (8 for no) - %d", data);
	copy_to_user(buff,&data,1);
	button_action = 8;
	return len; // return number of bytes read
}

// User program writes to the driver
static ssize_t gamepad_write(struct file *filp, const char __user *buff, size_t len, loff_t *offp){
	printk(KERN_INFO "I was written to");
	return len; //return the number of bytes written. Standard write response
} 

/* TODO: Figure out how to do complete interrupt solution.
		Currently using the half-solution where the interrupt
		triggers a save of the event in a driver variable
*/


static irqreturn_t gamepad_interrupt_handler(int irq_no, void *dev_id, struct pt_regs)
{
	printk(KERN_ALERT "Interrupt detected");
	printk(KERN_ALERT "I am working, and I am awesome");

	// Save the event that occurred during the interrupt
	button_action = ioread8(GPIO_PC_DIN);
	// Clear current interrupt flags
	iowrite32(ioread32(GPIO_IF),GPIO_IFC);
	
}


struct file_operations gamepad_fops = {
	.owner   = THIS_MODULE,
	.open    = gamepad_open,
	.release = gamepad_release, 
	.read    = gamepad_read,
	.write   = gamepad_write,	
};



/*
 * template_init - function to insert this module into kernel space
 *
 * This is the first of two exported functions to handle inserting this
 * code into a running kernel
 *
 * Returns 0 if successfull, otherwise -1
 */

static int __init gamepad_init(void)
{

	/* Use kmalloc(size_t size, int flags);
		Allocates size bytes of memory/ Returns a pointer to that memory or null if the allocation fails
	*/
	int ret;
	struct device *dev_ret;

	

	printk(KERN_INFO "Initializing driver");
	if((ret = alloc_chrdev_region(&dev_num, 0, 1, "gamepad")<0))
	{
		printk(KERN_ALERT "Failed to allocate device");
		return ret;
	}
	printk(KERN_INFO "<Major, Minor>: <%d, %d> \n", MAJOR(dev_num),MINOR(dev_num));



	// Requesting the memory regions we will write and read from
	/* 
		This tells the kernel that the driver will use these regions
		and then others can not reserve them. 
		Length is number of bytes, so for our case, 4 bytes
		This doesn't do any mapping, only reserves the are so that
		if another driver calls the request_mem_region for these
		it will receive an error. This is just for good coding and
		keeping track
		
		There is a check_mem_region function, but this should not be used.
		Just do as shown below using if 

	*/
	if(request_mem_region(GPIO_PC_MODEL, reg_size, "GPIO_PC_MODEL") == NULL)
	{
		printk(KERN_ALERT "ERROR requesting MODEL\n");
		return -1;
	}
	if(request_mem_region(GPIO_PC_DOUT, reg_size, "GPIO_PC_DOUT") == NULL)
	{
		printk(KERN_ALERT "ERROR requesting DOUT\n");
		return -1;
	}
	if(request_mem_region(GPIO_PC_DIN, reg_size, "GPIO_PC_DIN") == NULL)
	{
		printk(KERN_ALERT "ERROR requesting DIN\n");
		return -1;
	}
	
	// Connects interrupts to interrupt handlers
	if(request_irq(GPIO_EVEN_IRQ_LINE, (irq_handler_t)gamepad_interrupt_handler,IRQF_SHARED,"gamepad",&c_dev))
	{
		printk(KERN_ALERT "ERROR requesting IRQ ODD\n");
		return -1;
	}
	if(request_irq(GPIO_EVEN_IRQ_LINE, (irq_handler_t)gamepad_interrupt_handler,IRQF_SHARED,"gamepad",&c_dev))
	{
		printk(KERN_ALERT "ERROR requesting IRQ EVEN\n");
		return -1;
	}

	// if((vgamepad_pc = ioremap_nocache(GPIO_PC_BASE, GPIO_SIZE))== NULL){
	// 		printk(KERN_ERR "Mapping of GPIO failed\n");
	// 		return -ENOMEM;
	// }

	/* TODO
		Use ioremap function to remap the adresses and
		use offsets for iowrite
	*/

	iowrite32(0x33333333, GPIO_PC_MODEL); //Set drive mode to high(?? check this)
	iowrite32(0xFF, 	  GPIO_PC_DOUT);  //Enable pull-up

	// Actually enable interrupts in HW
	// WE HAVE NOT MEMORY REQUESTED THESE
	// it will still work, but other devices could technically write
	// to these registers now
	iowrite32(0x22222222, GPIO_EXTIPSELL);//Connect interrupt to pin on GPIO bank PC
	 
	/*Technically we would only need to trigger on rising edge for the game*/
	// iowrite32(0xFF, 	  GPIO_EXTIFALL); //Interrupt trigger on falling edge
	iowrite32(0xFF, 	  GPIO_EXTIRISE); //Interrupt trigger on rising edge
	iowrite32(0xFF, 	  GPIO_IEN);      //Enable interrupts


	// Add device/ create device files for user mode
	if(IS_ERR(cl = class_create(THIS_MODULE, "chardrv"))){
		unregister_chrdev_region(dev_num, 1);
		return PTR_ERR(cl);
	}

	if(IS_ERR(dev_ret = device_create(cl, NULL, dev_num, NULL, "button_driver"))){
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
		return PTR_ERR(dev_ret);
	}

	cdev_init(&c_dev, &gamepad_fops);
	if((ret = cdev_add(&c_dev, dev_num, 1))<0){
		device_destroy(cl, dev_num);
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
		return ret;
	}
	printk(KERN_INFO "Driver initiated successfully");


	return 0;
}

/*
 * template_cleanup - function to cleanup this module from kernel space
 */

static void __exit gamepad_cleanup(void)
{
	printk(KERN_INFO "Killing driver");

	release_mem_region(GPIO_PC_MODEL,1);
	release_mem_region(GPIO_PC_DOUT,1);
	release_mem_region(GPIO_PC_DIN,1);

	// release interrupts

	/* TODO
		Move these to the close function
	*/
	free_irq (GPIO_EVEN_IRQ_LINE, &c_dev);
	free_irq (GPIO_ODD_IRQ_LINE, &c_dev);

	//unregister device
	cdev_del(&c_dev);
	device_destroy(cl, dev_num);
	class_destroy(cl);

	// iounmap(vgamepad_pc);
	unregister_chrdev_region(dev_num, 1);
}

module_init(gamepad_init);
module_exit(gamepad_cleanup);

MODULE_DESCRIPTION("Amazingness, I am god");
MODULE_LICENSE("GPL");




