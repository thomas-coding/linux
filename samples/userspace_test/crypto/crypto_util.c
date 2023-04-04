/*
 * Copyright (c) 2021-2031, Jinping Wu (wunekky@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <assert.h>
#include "crypto_data.h"
#include "crypto_util.h"
#include "crypto_hash_test.h"

/* 0-ok , -1-fail */
int hex_compare(uint8_t *src, uint8_t *dst, uint32_t len)
{
	for (int i = 0; i < len; i++) {
		if (*src == *dst) {
			src++;
			dst++;
		} else
			return -1;
	}
	return 0;
}

void memory_hex_dump(char* start, uint8_t *buffer, uint32_t len)
{
#if CONFIG_HEX_DUMP == 1
	printf("\n%s:\n", start);
	for (int i = 0; i < len; i++) {
		if (buffer[i] < 0x10)
			printf("0%x ", buffer[i]);
		else
			printf("%2x ", buffer[i]);
		if ((i + 1) % 16 == 0)
			printf("\n");
	}
	printf("\n");
#endif
}

int basic_check(void) {
	int sockfd = socket(AF_ALG, SOCK_SEQPACKET, 0);
	int ret;

	if(sockfd < 0)
	{
		if(errno == EAFNOSUPPORT)
		{
			ret = -EAFNOSUPPORT;
			printf("#define AF_ALG_UNAVAILABLE\n");
		} else {
			ret = -errno;
			printf("socket api occurs other errors %d\n", errno);
		}
	} else {
		ret = 0;
		//printf("#define AF_ALG_AVAILABLE\n");
	}
 
	close(sockfd);
	return ret;
}
