// SPDX-License-Identifier: GPL-2.0-only
/*
 * a simple kernel module: hello block
 *
 * Copyright (C) 2022 Jinping Wu  (jinping.wu@gmail.com)
 *
 * Licensed under GPLv2 or later.
 */

/*
 * Sample disk driver, refer: https://github.com/CodeImp/sblkdev
 */

// SPDX-License-Identifier: GPL-2.0

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/hdreg.h> /* for HDIO_GETGEO */
#include <linux/cdrom.h> /* for CDROM_GET_CAPABILITY */
#include "hello_block.h"

#ifdef CONFIG_SBLKDEV_REQUESTS_BASED

static inline int process_request(struct request *rq, unsigned int *nr_bytes)
{
	int ret = 0;
	struct bio_vec bvec;
	struct req_iterator iter;
	struct sblkdev_device *dev = rq->q->queuedata;
	loff_t pos = blk_rq_pos(rq) << SECTOR_SHIFT;
	loff_t dev_size = (dev->capacity << SECTOR_SHIFT);

	rq_for_each_segment(bvec, rq, iter) {
		unsigned long len = bvec.bv_len;
		void *buf = page_address(bvec.bv_page) + bvec.bv_offset;

		if ((pos + len) > dev_size)
			len = (unsigned long)(dev_size - pos);

		if (rq_data_dir(rq))
			memcpy(dev->data + pos, buf, len); /* WRITE */
		else
			memcpy(buf, dev->data + pos, len); /* READ */

		pos += len;
		*nr_bytes += len;
	}

	return ret;
}

static blk_status_t _queue_rq(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd)
{
	unsigned int nr_bytes = 0;
	blk_status_t status = BLK_STS_OK;
	struct request *rq = bd->rq;

	//might_sleep();
	cant_sleep(); /* cannot use any locks that make the thread sleep */

	blk_mq_start_request(rq);

	if (process_request(rq, &nr_bytes))
		status = BLK_STS_IOERR;

	pr_debug("request %llu:%d processed\n", blk_rq_pos(rq), nr_bytes);

	blk_mq_end_request(rq, status);

	return status;
}

static struct blk_mq_ops mq_ops = {
	.queue_rq = _queue_rq,
};

#else  /* CONFIG_SBLKDEV_REQUESTS_BASED */

#pragma GCC push_options
#pragma GCC optimize ("O0")
static inline void process_bio(struct sblkdev_device *dev, struct bio *bio)
{
	struct bio_vec bvec;
	struct bvec_iter iter;
	loff_t pos = bio->bi_iter.bi_sector << SECTOR_SHIFT;
	loff_t dev_size = (dev->capacity << SECTOR_SHIFT);
	unsigned long start_time;

	start_time = bio_start_io_acct(bio);
	bio_for_each_segment(bvec, bio, iter) {
		unsigned int len = bvec.bv_len;
		void *buf = page_address(bvec.bv_page) + bvec.bv_offset;

		if ((pos + len) > dev_size) {
			/* len = (unsigned long)(dev_size - pos);*/
			bio->bi_status = BLK_STS_IOERR;
			break;
		}

		if (bio_data_dir(bio))
			memcpy(dev->data + pos, buf, len); /* WRITE */
		else
			memcpy(buf, dev->data + pos, len); /* READ */

		pos += len;
	}
	bio_end_io_acct(bio, start_time);
	bio_endio(bio);
}
#pragma GCC pop_options

#ifdef HAVE_QC_SUBMIT_BIO
blk_qc_t _submit_bio(struct bio *bio)
{
	blk_qc_t ret = BLK_QC_T_NONE;
#else
void _submit_bio(struct bio *bio)
{
#endif
#ifdef HAVE_BI_BDEV
	struct sblkdev_device *dev = bio->bi_bdev->bd_disk->private_data;
#endif
#ifdef HAVE_BI_BDISK
	struct sblkdev_device *dev = bio->bi_disk->private_data;
#endif

	might_sleep();
	//cant_sleep(); /* cannot use any locks that make the thread sleep */

	process_bio(dev, bio);

#ifdef HAVE_QC_SUBMIT_BIO
	return ret;
}
#else
}
#endif

#endif /* CONFIG_SBLKDEV_REQUESTS_BASED */


static int _open(struct block_device *bdev, fmode_t mode)
{
	struct sblkdev_device *dev = bdev->bd_disk->private_data;

	if (!dev) {
		pr_err("Invalid disk private_data\n");
		return -ENXIO;
	}

	pr_debug("Device was opened\n");

	return 0;
}

static void _release(struct gendisk *disk, fmode_t mode)
{
	struct sblkdev_device *dev = disk->private_data;

	if (!dev) {
		pr_err("Invalid disk private_data\n");
		return;
	}

	pr_debug("Device was closed\n");
}

static inline int ioctl_hdio_getgeo(struct sblkdev_device *dev, unsigned long arg)
{
	struct hd_geometry geo = {0};

	pr_err("ioctl_hdio_getgeo \n");
	geo.start = 0;
	if (dev->capacity > 63) {
		sector_t quotient;

		geo.sectors = 63;
		quotient = (dev->capacity + (63 - 1)) / 63;

		if (quotient > 255) {
			geo.heads = 255;
			geo.cylinders = (unsigned short)
				((quotient + (255 - 1)) / 255);
		} else {
			geo.heads = (unsigned char)quotient;
			geo.cylinders = 1;
		}
	} else {
		geo.sectors = (unsigned char)dev->capacity;
		geo.cylinders = 1;
		geo.heads = 1;
	}

	if (copy_to_user((void *)arg, &geo, sizeof(geo)))
		return -EINVAL;

	return 0;
}

static int _ioctl(struct block_device *bdev, fmode_t mode, unsigned int cmd, unsigned long arg)
{
	struct sblkdev_device *dev = bdev->bd_disk->private_data;

	pr_debug("contol command [0x%x] received\n", cmd);

	switch (cmd) {
	case HDIO_GETGEO:
		return ioctl_hdio_getgeo(dev, arg);
	case CDROM_GET_CAPABILITY:
		return -EINVAL;
	default:
		return -ENOTTY;
	}
}

static int _getgeo(struct block_device *bd, struct hd_geometry *geo)
{
	struct sblkdev_device *dev = bd->bd_disk->private_data;
	int ret = 0;
	//pr_err("_getgeo \n");
	geo->start = 0;
	if (dev->capacity > 63) {
		sector_t quotient;

		geo->sectors = 63;
		quotient = (dev->capacity + (63 - 1)) / 63;

		if (quotient > 255) {
			geo->heads = 255;
			geo->cylinders = (unsigned short)
				((quotient + (255 - 1)) / 255);
		} else {
			geo->heads = (unsigned char)quotient;
			geo->cylinders = 1;
		}
	} else {
		geo->sectors = (unsigned char)dev->capacity;
		geo->cylinders = 1;
		geo->heads = 1;
	}
	return ret;
}

#ifdef CONFIG_COMPAT
static int _compat_ioctl(struct block_device *bdev, fmode_t mode, unsigned int cmd, unsigned long arg)
{
	// CONFIG_COMPAT is to allow running 32-bit userspace code on a 64-bit kernel
	return -ENOTTY; // not supported
}
#endif

static const struct block_device_operations fops = {
	.owner = THIS_MODULE,
	.open = _open,
	.release = _release,
	.ioctl = _ioctl,
	.getgeo		= _getgeo,
#ifdef CONFIG_COMPAT
	.compat_ioctl = _compat_ioctl,
#endif
#ifndef CONFIG_SBLKDEV_REQUESTS_BASED
	.submit_bio = _submit_bio,
#endif
};

/*
 * sblkdev_remove() - Remove simple block device
 */
void sblkdev_remove(struct sblkdev_device *dev)
{
	del_gendisk(dev->disk);

#ifdef HAVE_BLK_MQ_ALLOC_DISK
#ifdef HAVE_BLK_CLEANUP_DISK
	blk_cleanup_disk(dev->disk);
#else
	put_disk(dev->disk);
#endif
#else
	blk_cleanup_queue(dev->disk->queue);
	put_disk(dev->disk);
#endif

#ifdef CONFIG_SBLKDEV_REQUESTS_BASED
	blk_mq_free_tag_set(&dev->tag_set);
#endif
	vfree(dev->data);

	kfree(dev);

	pr_info("simple block device was removed\n");
}

#ifdef CONFIG_SBLKDEV_REQUESTS_BASED
static inline int init_tag_set(struct blk_mq_tag_set *set, void *data)
{
	set->ops = &mq_ops;
	set->nr_hw_queues = 1;
	set->nr_maps = 1;
	set->queue_depth = 128;
	set->numa_node = NUMA_NO_NODE;
	set->flags = BLK_MQ_F_SHOULD_MERGE | BLK_MQ_F_STACKING;

	set->cmd_size = 0;
	set->driver_data = data;

	return blk_mq_alloc_tag_set(set);
}
#endif

#ifndef HAVE_BLK_MQ_ALLOC_DISK
static inline struct gendisk *blk_mq_alloc_disk(struct blk_mq_tag_set *set,
						void *queuedata)
{
	struct gendisk *disk;
	struct request_queue *queue;

	queue = blk_mq_init_queue(set);
	if (IS_ERR(queue)) {
		pr_err("Failed to allocate queue\n");
		return ERR_PTR(PTR_ERR(queue));
	}

	queue->queuedata = queuedata;
	blk_queue_bounce_limit(queue, BLK_BOUNCE_HIGH);

	disk = alloc_disk(1);
	if (!disk)
		pr_err("Failed to allocate disk\n");
	else
		disk->queue = queue;

	return disk;
}
#endif

#ifndef HAVE_BLK_ALLOC_DISK
static inline struct gendisk *blk_alloc_disk(int node)
{
	struct request_queue *q;
	struct gendisk *disk;

	q = blk_alloc_queue(node);
	if (!q)
		return NULL;

	disk = __alloc_disk_node(0, node);
	if (!disk) {
		blk_cleanup_queue(q);
		return NULL;
	}
	disk->queue = q;

	return disk;
}
#endif

/*
 * sblkdev_add() - Add simple block device
 */
struct sblkdev_device *sblkdev_add(int major, int minor, char *name,
				  sector_t capacity)
{
	struct sblkdev_device *dev = NULL;
	int ret = 0;
	struct gendisk *disk;

	pr_info("add device '%s' capacity %llu sectors\n", name, capacity);

	dev = kzalloc(sizeof(struct sblkdev_device), GFP_KERNEL);
	if (!dev) {
		ret = -ENOMEM;
		goto fail;
	}

	INIT_LIST_HEAD(&dev->link);
	dev->capacity = capacity;
	dev->data = __vmalloc(capacity << SECTOR_SHIFT, GFP_NOIO | __GFP_ZERO);
	if (!dev->data) {
		ret = -ENOMEM;
		goto fail_kfree;
	}

#ifdef CONFIG_SBLKDEV_REQUESTS_BASED
	ret = init_tag_set(&dev->tag_set, dev);
	if (ret) {
		pr_err("Failed to allocate tag set\n");
		goto fail_vfree;
	}

	disk = blk_mq_alloc_disk(&dev->tag_set, dev);
	if (unlikely(!disk)) {
		ret = -ENOMEM;
		pr_err("Failed to allocate disk\n");
		goto fail_free_tag_set;
	}
	if (IS_ERR(disk)) {
		ret = PTR_ERR(disk);
		pr_err("Failed to allocate disk\n");
		goto fail_free_tag_set;
	}

#else
	disk = blk_alloc_disk(NUMA_NO_NODE);
	if (!disk) {
		pr_err("Failed to allocate disk\n");
		ret = -ENOMEM;
		goto fail_vfree;
	}
#endif
	dev->disk = disk;

	/* only one partition */
#ifdef GENHD_FL_NO_PART_SCAN
	disk->flags |= GENHD_FL_NO_PART_SCAN;
#else
	disk->flags |= GENHD_FL_NO_PART;
#endif

	/* removable device */
	/* disk->flags |= GENHD_FL_REMOVABLE; */

	disk->major = major;
	disk->first_minor = minor;
	disk->minors = 1;

	disk->fops = &fops;

	disk->private_data = dev;

	sprintf(disk->disk_name, name);
	set_capacity(disk, dev->capacity);

#ifdef CONFIG_SBLKDEV_BLOCK_SIZE
	blk_queue_physical_block_size(disk->queue, CONFIG_SBLKDEV_BLOCK_SIZE);
	blk_queue_logical_block_size(disk->queue, CONFIG_SBLKDEV_BLOCK_SIZE);
	blk_queue_io_min(disk->queue, CONFIG_SBLKDEV_BLOCK_SIZE);
	blk_queue_io_opt(disk->queue, CONFIG_SBLKDEV_BLOCK_SIZE);
#else
	blk_queue_physical_block_size(disk->queue, SECTOR_SIZE);
	blk_queue_logical_block_size(disk->queue, SECTOR_SIZE);
#endif
	blk_queue_max_hw_sectors(disk->queue, BLK_DEF_MAX_SECTORS);
	blk_queue_flag_set(QUEUE_FLAG_NOMERGES, disk->queue);


#ifdef HAVE_ADD_DISK_RESULT
	ret = add_disk(disk);
	if (ret) {
		pr_err("Failed to add disk '%s'\n", disk->disk_name);
		goto fail_put_disk;
	}
#else
	add_disk(disk);
#endif

	pr_info("Simple block device [%d:%d] was added\n", major, minor);

	return dev;

#ifdef HAVE_ADD_DISK_RESULT
fail_put_disk:
#ifdef HAVE_BLK_MQ_ALLOC_DISK
#ifdef HAVE_BLK_CLEANUP_DISK
	blk_cleanup_disk(dev->disk);
#else
	put_disk(dev->disk);
#endif
#else
	blk_cleanup_queue(dev->queue);
	put_disk(dev->disk);
#endif
#endif /* HAVE_ADD_DISK_RESULT */

#ifdef CONFIG_SBLKDEV_REQUESTS_BASED
fail_free_tag_set:
	blk_mq_free_tag_set(&dev->tag_set);
#endif
fail_vfree:
	vfree(dev->data);
fail_kfree:
	kfree(dev);
fail:
	pr_err("Failed to add block device\n");

	return ERR_PTR(ret);
}

/*
 * A module can create more than one block device.
 * The configuration of block devices is implemented in the simplest way:
 * using the module parameter, which is passed when the module is loaded.
 * Example:
 *    modprobe sblkdev catalog="sblkdev1,2048;sblkdev2,4096"
 */

static int sblkdev_major;
static LIST_HEAD(sblkdev_device_list);
//static char *sblkdev_catalog = "sblkdev1,2048;sblkdev2,40960";
static char *sblkdev_catalog = "hello_block,40960";

/*
 * sblkdev_init() - Entry point 'init'.
 *
 * Executed when the module is loaded. Parses the catalog parameter and
 * creates block devices.
 */
static int __init sblkdev_init(void)
{
	int ret = 0;
	int inx = 0;
	char *catalog;
	char *next_token;
	char *token;
	size_t length;

	pr_err("sblkdev_init \n");
	sblkdev_major = register_blkdev(sblkdev_major, KBUILD_MODNAME);
	if (sblkdev_major <= 0) {
		pr_info("Unable to get major number\n");
		return -EBUSY;
	}

	length = strlen(sblkdev_catalog);
	if ((length < 1) || (length > PAGE_SIZE)) {
		pr_info("Invalid module parameter 'catalog'\n");
		ret = -EINVAL;
		goto fail_unregister;
	}

	catalog = kzalloc(length + 1, GFP_KERNEL);
	if (!catalog) {
		ret = -ENOMEM;
		goto fail_unregister;
	}
	strcpy(catalog, sblkdev_catalog);

	next_token = catalog;
	while ((token = strsep(&next_token, ";"))) {
		struct sblkdev_device *dev;
		char *name;
		char *capacity;
		sector_t capacity_value;

		name = strsep(&token, ",");
		if (!name)
			continue;
		capacity = strsep(&token, ",");
		if (!capacity)
			continue;

		ret = kstrtoull(capacity, 10, &capacity_value);
		if (ret)
			break;

		dev = sblkdev_add(sblkdev_major, inx, name, capacity_value);
		if (IS_ERR(dev)) {
			ret = PTR_ERR(dev);
			break;
		}

		list_add(&dev->link, &sblkdev_device_list);
		inx++;
	}
	kfree(catalog);

	if (ret == 0)
		return 0;

fail_unregister:
	unregister_blkdev(sblkdev_major, KBUILD_MODNAME);
	return ret;
}

/*
 * sblkdev_exit() - Entry point 'exit'.
 *
 * Executed when the module is unloaded. Remove all block devices and cleanup
 * all resources.
 */
static void __exit sblkdev_exit(void)
{
	struct sblkdev_device *dev;

	while ((dev = list_first_entry_or_null(&sblkdev_device_list,
					       struct sblkdev_device, link))) {
		list_del(&dev->link);
		sblkdev_remove(dev);
	}

	if (sblkdev_major > 0)
		unregister_blkdev(sblkdev_major, KBUILD_MODNAME);
}

module_init(sblkdev_init);
module_exit(sblkdev_exit);

module_param_named(catalog, sblkdev_catalog, charp, 0644);
MODULE_PARM_DESC(catalog, "New block devices catalog in format '<name>,<capacity sectors>;...'");
