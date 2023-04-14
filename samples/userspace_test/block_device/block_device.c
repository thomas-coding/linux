/*
 * Copyright (c) 2021-2031, Jinping Wu (wunekky@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>

#define BLOCK_DEVICE "/dev/mmcblk0p3"

/*
 * xxd -s 0 -l 100 /dev/mmcblk0p3
 * 00000000: 4865 6c6c 6f20 576f 726c 6421 0000 0000  Hello World!....
 */
int block_device_test(void) {
    int fd = -1;
    int res = 0;

    char write_dat[] = "Hello World!";
    char read_buf[128] = {0};

    fd = open(BLOCK_DEVICE, O_RDWR | O_CREAT, 0664);
    if(fd < 0)
    {
        printf("%s file open fail,errno = %d.\r\n", BLOCK_DEVICE, errno);
        exit(1);
    }

    res = write(fd, write_dat, sizeof(write_dat));
    if(res < 0)
    {
        printf("write dat fail,errno = %d.\r\n", errno);
        close(fd);
        exit(1);
    }
    else
    {
        printf("write %d bytes:%s\r\n", res, write_dat);
    }

    close(fd);

    fd = open(BLOCK_DEVICE, O_RDONLY);
    if(fd < 0)
    {
        printf("%s file open fail,errno = %d.\r\n", BLOCK_DEVICE, errno);
        exit(1);
    }

    res = read(fd, read_buf, sizeof(read_buf));
    if(res < 0)
    {
        printf("read dat fail,errno = %d.\r\n", errno);
        close(fd);
        exit(1);
    }
    else
    {
        printf("read %d bytes:%s\r\n", res, read_buf);
    }

    close(fd);

    return 0;
}

void main()
{
    printf("block device test\n");
    block_device_test();
}

