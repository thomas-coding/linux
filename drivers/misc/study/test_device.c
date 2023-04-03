// SPDX-License-Identifier: GPL-2.0-only
/*
 * a simple kernel module: test device
 *
 * Copyright (C) 2022 Jinping Wu  (jinping.wu@gmail.com)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/mailbox_client.h>
#include <linux/of_irq.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/reset.h>

#define TEST_MONITOR_CMD_MAX_LENGTH        16

struct test_device {
	struct device *dev;
	void __iomem *regs;
	int num_rstcs;
	struct work_struct worker;
#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs_dir;
#endif /* CONFIG_DEBUG_FS */
};

struct test_regs {
	u32 reg0;               /* Offset: 0x000 (R/W) Value1 */
	u32 reg1;               /* Offset: 0x004 (R/W) Value2 */
	u32 reg2;               /* Offset: 0x008 (R/W) Value1 + Value2 */
	u32 reg3;               /* Offset: 0x00c (R/W) write 0xa5 trigger interrupt */
	u32 reg4;               /* Offset: 0x010 (R/W) write 0xa5 clear interrupt */
};

static irqreturn_t test_device_irq_handler(int irq, void *context)
{
	struct test_device *test = context;
	struct test_regs *regs = ((struct test_regs *)(test->regs));

	regs->reg4 = 0xa5;
	schedule_work(&test->worker);
	return IRQ_HANDLED;
}

static int test_device_remove(struct platform_device *pdev)
{
#ifdef CONFIG_DEBUG_FS
	struct test_device *test = platform_get_drvdata(pdev);

	debugfs_remove_recursive(test->debugfs_dir);
	test->debugfs_dir = NULL;
#endif /* CONFIG_DEBUG_FS */
	return 0;
}

#ifdef CONFIG_DEBUG_FS
void test_dump(struct test_device *test)
{
	struct test_regs *regs = ((struct test_regs *)(test->regs));
	regs->reg0 = 0x33;
	regs->reg1 = 0x44;
	dev_info(test->dev, "test_dump expect:0x%x actual:0x%x\n",
			regs->reg0 + regs->reg1, regs->reg2);
}

void test_trigger(struct test_device *test)
{
	struct test_regs *regs = ((struct test_regs *)(test->regs));

	dev_info(test->dev, "test_trigger\n");
	regs->reg3 = 0xa5;
}

static int test_device_open(struct inode *inode, struct file *filp)
{
	struct platform_device *pdev = inode->i_private;

	filp->private_data = pdev;
	return 0;
}

static ssize_t test_device_write(struct file *filp, const char __user *buf,
				 size_t count, loff_t *ppos)
{
	int ret;
	char cmd[TEST_MONITOR_CMD_MAX_LENGTH];
	struct platform_device *pdev = filp->private_data;
	struct test_device *test = platform_get_drvdata(pdev);

	if (count > TEST_MONITOR_CMD_MAX_LENGTH) {
		ret = -EINVAL;
		goto out;
	}

	if (copy_from_user(cmd, buf, count)) {
		ret = -EFAULT;
		goto out;
	}

	if (!strncmp(cmd, "trigger", 7))
		test_trigger(test);
	else if (!strncmp(cmd, "dump", 4))
		test_dump(test);
	else if (!strncmp(cmd, "bug_on", 6))
		BUG_ON("test device bug");
	else if (!strncmp(cmd, "panic", 5))
		panic("test device panic");
	else
		dev_info(&pdev->dev, "unknown cmd\n");

	ret = count;
out:
	return ret;
}

static const struct file_operations test_device_fops = {
	.write	= test_device_write,
	.open	= test_device_open,
};
#endif /* CONFIG_DEBUG_FS */

static void test_work(struct work_struct *work)
{

	struct test_device *test =
	    container_of(work, struct test_device, worker);
	struct device *dev = test->dev;

	dev_warn(dev, "Detect interrupt happen\n");

}

static int test_device_probe(struct platform_device *pdev)
{
	int	ret, irq;
	struct test_device *test;

	dev_warn(&pdev->dev, "test_device_probe\n");
	test = devm_kzalloc(&pdev->dev, sizeof(*test), GFP_KERNEL);
	if (!test)
		return -ENOMEM;

	test->dev = &pdev->dev;
	test->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(test->regs))
		return PTR_ERR(test->regs);

	platform_set_drvdata(pdev, test);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "Could not get irq: %d\n", irq);
		return irq;
	}

	ret = devm_request_irq(&pdev->dev, irq, test_device_irq_handler, 0,
			       dev_name(&pdev->dev), (void *)test);
	if (ret) {
		dev_err(&pdev->dev, "Request_irq failed\n");
		return ret;
	}

	INIT_WORK(&test->worker, test_work);

#ifdef CONFIG_DEBUG_FS
	/* mount -t debugfs none /sys/kernel/debug for use */
	test->debugfs_dir = debugfs_create_dir(dev_name(&pdev->dev), NULL);
	if (!test->debugfs_dir) {
		dev_err(&pdev->dev, "Creating debugfs dir failed\n");
		return -EIO;
	}

	debugfs_create_file("control", 0600,
			    test->debugfs_dir,
							 pdev,
							 &test_device_fops);
#endif /* CONFIG_DEBUG_FS */
	return 0;
}

static const struct of_device_id test_device_id_table[] = {
	{ .compatible = "virtual-platform,test-device", },
	{ }
};
MODULE_DEVICE_TABLE(of, test_device_id_table);

static struct platform_driver test_device_driver = {
	.driver		= {
		.name	= "test_device",
		.of_match_table = of_match_ptr(test_device_id_table),
	},
	.probe		= test_device_probe,
	.remove		= test_device_remove,
};

module_platform_driver(test_device_driver);

MODULE_DESCRIPTION("QEMU TEST DEVICE");
MODULE_LICENSE("GPL");