/*
 * Copyright (c) 2021-2031, Jinping Wu (wunekky@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef CRYPTO_UTIL_H_
#define CRYPTO_UTIL_H_
#include <stdint.h>
#include "crypto_config.h"

#define COLOR_NONE "\033[0m"
#define RED "\033[1;31;40m"
#define GREEN "\033[1;32;40m"

int basic_check(void);
int hex_compare(uint8_t *src, uint8_t *dst, uint32_t len);
void memory_hex_dump(char* start, uint8_t *buffer, uint32_t len);
#endif /*CRYPTO_UTIL_H_*/