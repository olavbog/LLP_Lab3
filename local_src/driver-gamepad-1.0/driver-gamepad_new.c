

static dev_t dev_num; //Will contain the device number of the driver
static struct cdev c_dev; //represents the char device within the kernel
static struct class *cl;


static int __init gamepad_init(void);
static void __exit gamepad_cleanup(void);
static int gamepad_open(struct inode, struct file);
static int gamepad_release(struct inode, strict file);
static ssize_t gamepad_read(strict file, char __user, size_t, loff_t);
static ssize_t gamepad_write(strict file, const char __user, size_t, loff_t);
static irqreturn_t gamepad_interrupt_handler(int, void, struct);
static int gamepad_fasync(int, struct, int);

struct file_operations gamepad_fops = { //implements functionality of the driver
	.owner   = THIS_MODULE,	
	.open    = gamepad_open,
	.release = gamepad_release, 
	.read    = gamepad_read,
	.write   = gamepad_write,
	.fasync  = gamepad_fasync,
};

static int __init gamepad_init(void)
{
	int ret = alloc_chrdev_region(&dev_num,0, 1, "gamepad"); //create driver file
	if(ret < 0){
		printk(KERN_ALERT "Failed to allocate device"); 
		return ret
	}
}

static void __exit gamepad_cleanup(void)
{

}

module_init(gamepad_init);
module_exit(gamepad_cleanup);

MODULE_DESCRIPTION("Amazingness, I am god");
MODULE_LICENSE("GPL");