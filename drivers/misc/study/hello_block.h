// SPDX-License-Identifier: GPL-2.0-only
/*
 * a simple kernel module: hello block
 *
 * Copyright (C) 2022 Jinping Wu  (jinping.wu@gmail.com)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/blk_types.h>
#include <linux/blkdev.h>
#include <linux/device.h>
#include <linux/blk-mq.h>
#include <linux/list.h>

#define HAVE_BI_BDEV 1
#define HAVE_BLK_MQ_ALLOC_DISK 1
#define HAVE_BLK_ALLOC_DISK 1
//#define CONFIG_SBLKDEV_REQUESTS_BASED 1
#define HAVE_ADD_DISK_RESULT 1

struct sblkdev_device {
	struct list_head link;

	sector_t capacity;		/* Device size in sectors */
	u8 *data;			/* The data in virtual memory */
#ifdef CONFIG_SBLKDEV_REQUESTS_BASED
	struct blk_mq_tag_set tag_set;
#endif
	struct gendisk *disk;
};

struct sblkdev_device *sblkdev_add(int major, int minor, char *name,
				  sector_t capacity);
void sblkdev_remove(struct sblkdev_device *dev);
