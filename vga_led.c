/*
 * Device driver for the VGA LED Emulator
 *
 * A Platform device implemented using the misc subsystem
 *
 * Stephen A. Edwards
 * Columbia University
 *
 * References:
 * Linux source: Documentation/driver-model/platform.txt
 *               drivers/misc/arm-charlcd.c
 * http://www.linuxforu.com/tag/linux-device-drivers/
 * http://free-electrons.com/docs/
 *
 * "make" to build
 * insmod vga_led.ko
 *
 * Check code style with
 * checkpatch.pl --file --no-tree vga_led.c
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "vga_led.h"

#define DRIVER_NAME "vga_led"

/*
 * Information about our device
 */
struct vga_led_dev 
{
    struct resource res;          /* Resource: our registers */
    void __iomem *virtbase;       /* Where registers can be accessed in memory */
    int coordinates[2];           /* 2 Element Array of x & y coordinates */
} dev;

/*
 * Write segments of a single digit
 * Assumes digit is in range and the device information has been set up
 */
static void write_coordinates(int x, int y)
{  
    iowrite16((u16)x, dev.virtbase);
    iowrite16((u16)y, dev.virtbase + 2);
    
    dev.coordinates[0] = x;
    dev.coordinates[1] = y;
}

/*
 * Handle ioctl() calls from userspace:
 * Read or write the segments on single digits.
 * Note extensive error checking of arguments
 */
static long 
vga_led_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    vga_led_arg_t vla;
    
    switch (cmd) 
    {
    case VGA_LED_DRAW_BALL:
	if (copy_from_user(&vla, (vga_led_arg_t *) arg, sizeof(vga_led_arg_t)))
	    return -EACCES;
	write_coordinates(vla.x, vla.y);
	break;
	
    default:
	return -EINVAL;
    }
    
    return 0;
}

/* The operations our device knows how to do */
static const struct file_operations vga_led_fops = {
    .owner		= THIS_MODULE,
    .unlocked_ioctl     = vga_led_ioctl,
};

/* Information about our device for the "misc" framework -- like a char dev */
static struct miscdevice vga_led_misc_device = {
    .minor		= MISC_DYNAMIC_MINOR,
    .name		= DRIVER_NAME,
    .fops		= &vga_led_fops,
};

/*
 * Initialization code: get resources (registers) and display
 * a welcome message
 */
static int __init vga_led_probe(struct platform_device *pdev)
{
    int ret;
    
    /* Register ourselves as a misc device: creates /dev/vga_led */
    ret = misc_register(&vga_led_misc_device);
    
    /* Get the address of our registers from the device tree */
    ret = of_address_to_resource(pdev->dev.of_node, 0, &dev.res);
    if (ret) 
    {
	ret = -ENOENT;
	goto out_deregister;
    }
    
    /* Make sure we can use these registers */
    if (request_mem_region(dev.res.start, resource_size(&dev.res), DRIVER_NAME) == NULL) 
    {
	ret = -EBUSY;
	goto out_deregister;
    }
    
    /* Arrange access to our registers */
    dev.virtbase = of_iomap(pdev->dev.of_node, 0);
    if (dev.virtbase == NULL) 
    {
	ret = -ENOMEM;
	goto out_release_mem_region;
    }
    
    /* Plot Static Ball*/
    write_coordinates(200,200);
    
    return 0;

out_release_mem_region:
    release_mem_region(dev.res.start, resource_size(&dev.res));
out_deregister:
    misc_deregister(&vga_led_misc_device);
    return ret;
}

/* Clean-up code: release resources */
static int vga_led_remove(struct platform_device *pdev)
{
    iounmap(dev.virtbase);
    release_mem_region(dev.res.start, resource_size(&dev.res));
    misc_deregister(&vga_led_misc_device);
    return 0;
}

/* Which "compatible" string(s) to search for in the Device Tree */
#ifdef CONFIG_OF
static const struct of_device_id vga_led_of_match[] = 
{
    { .compatible = "altr,vga_led" },
    {},
};
MODULE_DEVICE_TABLE(of, vga_led_of_match);
#endif

/* Information for registering ourselves as a "platform" driver */
static struct platform_driver vga_led_driver = 
{
    .driver = 
    {
	.name	        = DRIVER_NAME,
	.owner	        = THIS_MODULE,
	.of_match_table = of_match_ptr(vga_led_of_match),
    },
    .remove      	= __exit_p(vga_led_remove),
};

/* Called when the module is loaded: set things up */
static int __init vga_led_init(void)
{
    pr_info(DRIVER_NAME ": init\n");
    return platform_driver_probe(&vga_led_driver, vga_led_probe);
}

/* Called when the module is unloaded: release resources */
static void __exit vga_led_exit(void)
{
    platform_driver_unregister(&vga_led_driver);
    pr_info(DRIVER_NAME ": exit\n");
}

module_init(vga_led_init);
module_exit(vga_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stephen A. Edwards, Columbia University");
MODULE_DESCRIPTION("VGA 7-segment LED Emulator");
