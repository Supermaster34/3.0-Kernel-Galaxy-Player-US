/*
 * serial_acm.c -- USB modem serial driver
 *
 * Copyright 2008 (C) Samsung Electronics
 *
 */
 
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/usb/cdc.h>

extern void acm_notify(void * dev, u16 state);


static wait_queue_head_t modem_wait_q;

static unsigned int read_state = 0;
static unsigned int control_line_state = 0;

static void * acm_data = NULL;

static int modem_open(struct inode *inode, struct file *file)
{
	read_state = 0;
	
	return 0;
}

static int modem_close(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t modem_read(struct file *file, char __user *buf,
		size_t count, loff_t *ppos)
{
	int ret = 0;

	if (file->f_flags & O_NONBLOCK) 
		return -EAGAIN;

	ret = wait_event_interruptible(modem_wait_q, read_state);
	if (ret)
		return ret;
	
	if (copy_to_user(buf, &control_line_state, sizeof(u32)))
		return -EFAULT;

	read_state = 0;
	
	return sizeof(u32);
}

static unsigned int modem_poll(struct file *file, poll_table *wait)
{
	poll_wait(file, &modem_wait_q, wait);

	return (read_state ? (POLLIN | POLLRDNORM) : 0);
}

void notify_control_line_state(u32 value)
{
	control_line_state = value;

	read_state = 1;

	wake_up_interruptible(&modem_wait_q);
}
EXPORT_SYMBOL(notify_control_line_state);


#define GS_CDC_NOTIFY_SERIAL_STATE	_IOW('S', 1, int)
#define GS_IOC_NOTIFY_DTR_TEST		_IOW('S', 3, int)

static int 
modem_ioctl (struct inode *inode, struct file *file, unsigned int  cmd,unsigned long arg)
{
	printk("modem_ioctl: cmd=0x%x, arg=%lu\n", cmd, arg);

	/* handle ioctls */
	switch (cmd) 
	{
		case GS_CDC_NOTIFY_SERIAL_STATE:
			acm_notify(acm_data, __constant_cpu_to_le16(arg));
			break;

		case GS_IOC_NOTIFY_DTR_TEST:
			{
			printk(KERN_ALERT"DUN : DTR %d\n",(int)arg);
			notify_control_line_state((int)arg);
			break;
			}
		
		default:
			printk("modem_ioctl: Unknown ioctl cmd(0x%x).\n", cmd);
			return -ENOIOCTLCMD;
	}
	return 0;
}


static struct file_operations modem_fops = {
	.owner 		= THIS_MODULE,
	.open 		= modem_open,
	.release 	= modem_close,
	.read 		= modem_read,
	.poll 		= modem_poll,
	.llseek 	= no_llseek,
	//.ioctl		= modem_ioctl,
};

static struct miscdevice modem_device = {
	.minor	= 123,
	.name	= "dun",
	.fops	= &modem_fops,
};

int modem_register(void * data)
{
	int ret;

	if(data == NULL)
	{
		printk("DUN register failed. data is null.\n");
		return -1;
	}

	acm_data = data;

	ret = misc_register(&modem_device);
	if (ret) {
		printk(KERN_ERR "DUN register is failed, ret = %d\n", ret);
		return ret;
	}

	init_waitqueue_head(&modem_wait_q);

	printk(KERN_INFO "DUN is registerd\n");
	
	return 0;
}
EXPORT_SYMBOL(modem_register);

void modem_unregister(void)
{
	acm_data = NULL;

	misc_deregister(&modem_device);
	
	printk(KERN_INFO "DUN is unregisterd\n");
}
EXPORT_SYMBOL(modem_unregister);
