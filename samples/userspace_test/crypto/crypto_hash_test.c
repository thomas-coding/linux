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

void sha1_test(void) {
	struct sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type = "hash",
		.salg_name = "sha1"
	};
	char out[HASH_LENGTH_SHA1];
	int fd_base, fd_alg, ret;
	ssize_t size;
 
	fd_base = socket(AF_ALG, SOCK_SEQPACKET, 0);
	assert(fd_base > 0);
 
	ret = bind(fd_base, (struct sockaddr *)&sa, sizeof(sa));
	assert(!ret);
 
	fd_alg = accept(fd_base, NULL, 0);
	assert(fd_alg > 0);
 
	size = write(fd_alg, sha1_msg, sizeof(sha1_msg));
	assert(size == sizeof(sha1_msg));

	size = read(fd_alg, out, HASH_LENGTH_SHA1);
	assert(size == HASH_LENGTH_SHA1);
 
 	memory_hex_dump("crypto hash1 digest", out, HASH_LENGTH_SHA1);
	ret =  hex_compare(out, sha1_digest, HASH_LENGTH_SHA1);
	if (ret < 0)
		printf(RED"[%s] FAIL\n"COLOR_NONE, __func__);
	else
		printf(GREEN"[%s] SUCCESS\n"COLOR_NONE, __func__);

	close(fd_alg);
	close(fd_base);

}

void crypto_hash_test(void) {
#if CONFIG_HASH_SHA1 == 1
    sha1_test();
#endif
}