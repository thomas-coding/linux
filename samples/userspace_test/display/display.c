// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2021-2031, Jinping Wu (wunekky@gmail.com). All rights reserved.
 *
 */

/* Ref: linux/samples/auxdisplay/cfag12864b-example.c */
/* Ref: https://www.rapidtables.org/zh-CN/web/color/RGB_Color.html */

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

/* Get from get_info function*/
#define FB_WIDTH		(1024)
#define FB_HEIGHT		(768)
#define FB_BPP			(32)
#define FB_SIZE			(FB_WIDTH * FB_HEIGHT * FB_BPP / 8)


int fb_fd;
unsigned char *fb_mem;
unsigned char fb_buffer[FB_SIZE];

/*
 * init framebuffer device
 *
 * No error:       return = 0
 * Unable to open: return = -1
 * Unable to mmap: return = -2
 */
static int fb_device_init(char *path)
{
	fb_fd = open(path, O_RDWR);
	if (fb_fd == -1)
		return -1;
	fb_mem = mmap(0, FB_SIZE, PROT_READ | PROT_WRITE,
		MAP_SHARED, fb_fd, 0);
	if (fb_mem == MAP_FAILED) {
		close(fb_fd);
		return -2;
	}

	return 0;
}

/*
 * exit a fb framebuffer device
 */
static void fb_device_exit(void)
{
	munmap(fb_mem, FB_SIZE);
	close(fb_fd);
}

/*
 * clear (unset all pixels)
 */
static void fb_device_clear(void)
{
	int i;

	for (i = 0; i < FB_SIZE; i++)
		fb_buffer[i] = 0;
}

/*
 * white (all pixels 0xff)
 */
static void fb_device_make_white(void)
{
	int i;

	for (i = 0; i < FB_SIZE; i++)
		fb_buffer[i] = 0xff;
}

/*
 * draw a point
 */
static void fb_device_draw_point(int x, int y, int r, int g, int b)
{
	int offset = (y * FB_WIDTH + x) * FB_BPP / 8;

	fb_buffer[offset] = b;
	fb_buffer[offset+1] = g;
	fb_buffer[offset+2] = r;
}

/*
 * draw a rectangle
 */
static void fb_device_draw_rectangle(int x_from, int y_from, int x_to, int y_to, int r, int g, int b)
{
	int i, j;

	for (j = y_from; j <= y_to; j++)
		for (i = x_from; i <= x_to; i++)
			fb_device_draw_point(i, j, r, g, b);

}

/*
 * blit buffer to lcd
 */
static void fb_device_blit(void)
{
	memcpy(fb_mem, fb_buffer, FB_SIZE);
}

#define EXAMPLES	4
static void example(unsigned char n)
{
	int i, j;

	if (n > EXAMPLES)
		return;

	printf("Example %i/%i -\n", n, EXAMPLES);

	switch (n) {
	case 1:
		printf("Clear the LCD\n");
		fb_device_clear();
		break;

	case 2:
		printf("Make it white\n");
		fb_device_make_white();
		break;

	case 3:
		printf("Draw a point\n");
		fb_device_draw_point(100, 100, 255, 0, 255);
		break;

	case 4:
		printf("Draw a rectangle\n");
	fb_device_make_white();
		fb_device_draw_rectangle(100, 100, 500, 500, 255, 215, 0);
		break;
	}

	puts(" - [Press Enter]");
}

int get_info(void)
{
	struct fb_var_screeninfo fb_varinfo;
	int ret;

	fb_fd = open("/dev/fb0", O_RDWR);
	if (fb_fd == -1)
		return -1;

	ret = ioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_varinfo);
	if (fb_fd == -1) {
		close(fb_fd);
		return -1;
	}

	printf("x: %d\n", fb_varinfo.xres);
	printf("y: %d\n", fb_varinfo.yres);
	printf("bits_per_pixel: %d\n", fb_varinfo.bits_per_pixel);
	close(fb_fd);
	return 0;
}

int main(int argc, char *argv[])
{
	unsigned char n;

	//get_info();

	if (argc != 2) {
		printf(
			"Syntax:  %s fbdev\n"
			"Usually: /dev/fb0, /dev/fb1...\n", argv[0]);
		return -1;
	}

	if (fb_device_init(argv[1])) {
		printf("Can't init %s fbdev\n", argv[1]);
		return -2;
	}

	for (n = 1; n <= EXAMPLES; n++) {
		example(n);
		fb_device_blit();
		while (getchar() != '\n')
			;
	}

	fb_device_exit();

	return 0;
}

