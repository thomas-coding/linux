// SPDX-License-Identifier: GPL-2.0-only
/*
 * a simple kernel module: hello char
 *
 * Copyright (C) 2022 Jinping Wu  (jinping.wu@gmail.com)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/platform_device.h>

#define DRIVER_NAME "hello_char"
#define HELLO_CHAR_BUFFER_SIZE 256
struct hello_char_device {
	struct cdev cdev;
	dev_t devno;
	char hello_char_buffer[HELLO_CHAR_BUFFER_SIZE];
};
static struct hello_char_device hello_dev;


static struct class *hello_class;

static int hello_char_open(struct inode *inode, struct file *file)
{
	struct hello_char_device *dev;

	/* From cdev to our dev */
	dev = container_of(inode->i_cdev, struct hello_char_device, cdev);

	/* Put it to file pointer, than other function can access it */
	file->private_data = dev;

	//memset(dev->hello_char_buffer, 0, HELLO_CHAR_BUFFER_SIZE);

	pr_err("hello char open ..\n");
	//pr_err("file short name:%s\n", file->f_path.dentry->d_iname);
	//pr_err("inode dev_t %d:%d\n", MAJOR(inode->i_rdev), MINOR(inode->i_rdev));
	return 0;
}

static int hello_char_release(struct inode *inode, struct file *file)
{
	//pr_err("hello char release ..\n");
	return 0;
}

/*
 * Read, input file, user buffer, size how much want to read, ppos where to read
 * Return, count put to userspace, if end return 0, if fail return negitive value
 */
//char hello_data[] = "\nhello, I am data from kernel...\n\n";
//char hello_char_buffer[256];
ssize_t hello_char_read(struct file *file, char __user *buf, size_t size,
			  loff_t *ppos)
{
	struct hello_char_device *dev = file->private_data;
	unsigned long p = *ppos;
	unsigned int count = size;
	int retval = 0;
	unsigned long hello_data_size = sizeof(dev->hello_char_buffer);

	pr_err("hello char read ..\n");
	if (p >= hello_data_size) {
		//pr_err("sorry, we do not have more data\n");
		return 0;
	}

	if (count > hello_data_size - p) {
		/* Only read which we have */
		count = hello_data_size - p;
	}

	if (copy_to_user(buf, dev->hello_char_buffer + p, count))
		return -EFAULT;

	/* Update position and return value, tell user how much we read */
	*ppos += count;
	retval = count;
	//pr_err("read %u bytes(s) from %lu\n", count, p);

	return retval;
}

/*
 * Read, input file, user buffer have data, size how much want to write, ppos where to write
 * Return, count put to userspace, if end return 0, if fail return negitive value
 */
//char write_buffer[256];
ssize_t hello_char_write(struct file *file, const char __user *buf, size_t size,
			  loff_t *ppos)
{
	struct hello_char_device *dev = file->private_data;
	unsigned long p = *ppos;
	unsigned int count = size;
	int retval = 0;
	unsigned long write_data_size = sizeof(dev->hello_char_buffer);

	if (p >= write_data_size) {
		//pr_err("sorry, we do not have more buffer to write to\n");
		return 0;
	}

	if (count > write_data_size - p) {
		/* Only write suit for our buffer */
		count = write_data_size - p;
	}

	if (copy_from_user(dev->hello_char_buffer + p, buf, count))
		return -EFAULT;

	/* Update position and return value, tell user how much we write */
	*ppos += count;
	retval = count;
	pr_err("written %u bytes(s) from %lu\n", count, p);

	return retval;
}

static const struct file_operations hello_char_fops = {
	.open       = hello_char_open,
	.release    = hello_char_release,
	.read       = hello_char_read,
	.write      = hello_char_write,
	.owner      = THIS_MODULE,
};

/* new char device register */
static int __init hello_char_init(void)
{
	int retval;
	struct device *dev;

	/* Init cdev , init kobject(cdev.kobj) and put ops to cdev */
	/* only do struct init, and link to system */
	cdev_init(&hello_dev.cdev, &hello_char_fops);

	/* Why need do this? TODO */
	hello_dev.cdev.owner = THIS_MODULE;

	/* Get device id , major is dynamically distribute, baseminor is 0, count is 1 */
	/* malloc char_device_struct and put to golbal chrdevs */
	retval = alloc_chrdev_region(&hello_dev.devno, 0, 1, DRIVER_NAME);
	if (retval < 0) {
		pr_err("register-chrdev failed: %d\n", retval);
		/* Fail, so char_device_struct 'NOT' malloc, no need to free */
		goto error;
	}

	/* Register device , input cdev, first device number and minor count */
	/* malloc probe struct and put to golbal cdev_map */
	retval = cdev_add(&hello_dev.cdev, hello_dev.devno, 1);
	if (retval < 0) {
		pr_err("cannot add cdev\n");
		/* Here we already done char_device_struct malloc, so we need to clean */
		/* Fail, so probe 'NOT' malloc, no need to free */
		goto cleanup_alloc_chrdev_region;
	}

	/* Init buffer */
	memset(hello_dev.hello_char_buffer, 0, HELLO_CHAR_BUFFER_SIZE);

	/* Create class */
	hello_class = class_create(THIS_MODULE, "hello_char_class");
	if (IS_ERR(hello_class)) {
		retval = PTR_ERR(hello_class);
		goto cleanup_alloc_chrdev_region;
	}

	/* Create dev */
	dev = device_create(hello_class, NULL, hello_dev.devno, NULL, "hello_char");
	if (IS_ERR(dev)) {
		pr_err("Could not create files\n");
		retval = PTR_ERR(hello_class);
		goto cleanup_all;
	}

	return 0;

cleanup_all:
	class_destroy(hello_class);
cleanup_alloc_chrdev_region:
	unregister_chrdev_region(hello_dev.devno, 1);
error:
	return retval;
}

static void __exit hello_char_exit(void)
{
	/* Free dev number. actually free char_device_struct */
	unregister_chrdev_region(hello_dev.devno, 1);

	/* Free cdev, actually free probe and delete cdev from system */
	cdev_del(&hello_dev.cdev);

	class_destroy(hello_class);
}

module_init(hello_char_init);
module_exit(hello_char_exit);
