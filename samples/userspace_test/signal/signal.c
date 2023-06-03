// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2021-2031, Jinping Wu (wunekky@gmail.com). All rights reserved.
 *
 */

#include <signal.h>
#include <stdio.h>

void sig_handler(int sig)
{
	printf("access signal handler, signal is %d\n", sig);
}

/* enter ctrl-c, it will call sig_handler, ctrl-z exit */
int main(void)
{
	//signal(SIGINT, sig_handler);
	struct sigaction act;

	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGINT);
	sigaction(SIGINT, &act, NULL);
	while (1)
		;
	return 0;
}
