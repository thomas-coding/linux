// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2021-2031, Jinping Wu (wunekky@gmail.com). All rights reserved.
 *
 */
/* Also refer: linux/tools/testing/selftests/arm64/abi/ptrace.c */
#include <stdio.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <linux/elf.h>

void signal_handler(int sig)
{
	printf("Process %ld received signal %d\n", (long)getpid(), sig);
}

/* Stop at enternce and dump register */
void do_debugger(void)
{
	int status = 0;
	pid_t child;
	/* Refer: linux/arch/arm64/include/uapi/asm/ptrace.h */
	struct user_regs_struct regs;

	printf("In debugger process %ld\n", (long)getpid());

	if (signal(SIGCHLD, signal_handler) == SIG_ERR) {
		perror("signal");
		exit(-1);
	}

	do {
		child = wait(&status);
		printf("Debugger exited wait() for child:%ld\n",  child);
		if (WIFSTOPPED(status)) {
			printf("Child has stopped due to signal %d\n", WSTOPSIG(status));
			struct iovec iov;

			iov.iov_base = &regs;
			iov.iov_len = sizeof(struct user_regs_struct);
			ptrace(PTRACE_GETREGSET, child, NT_PRSTATUS, &iov);

			for (int i = 0; i < 31; i++)
				printf("X[%d]: 0x%x\n", i, regs.regs[i]);
			printf("sp: 0x%x\n", regs.sp);
			printf("pc: 0x%x\n", regs.pc);
			printf("pstate: 0x%x\n", regs.pstate);

			ptrace(PTRACE_CONT, child, NULL, NULL);
		}

		if (WIFSIGNALED(status))
			printf("Child %ld received signal %d\n", (long)child, WTERMSIG(status));
	} while (!WIFEXITED(status));
}

/* Step to run debuggee and dump all instruction */
void run_debugger(pid_t child_pid)
{
	int wait_status;
	unsigned int icounter = 0;

	printf("debugger started\n");

	/* Wait for child to stop on its first instruction */
	wait(&wait_status);

	while (WIFSTOPPED(wait_status)) {
		icounter++;

		struct user_regs_struct regs;
		struct iovec iov;

		iov.iov_base = &regs;
		iov.iov_len = sizeof(struct user_regs_struct);
		ptrace(PTRACE_GETREGSET, child_pid, NT_PRSTATUS, &iov);
		unsigned int instr = ptrace(PTRACE_PEEKTEXT, child_pid, regs.pc, 0);

		printf("icounter = %u.  PC = 0x%08x.  instr = 0x%08x\n", icounter, regs.pc, instr);

		/* Make the child execute another instruction */
		if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
			perror("ptrace");
			return;
		}

		/* Wait for child to stop on its next instruction */
		wait(&wait_status);
	}

	printf("the child executed %u instructions\n", icounter);
}

void do_debuggee(void)
{
	printf("In debuggie process %ld\n", (long)getpid());

	if (ptrace(PTRACE_TRACEME, 0, NULL, NULL)) {
		perror("ptrace");
		return;
	}

	execve("/usr/bin/debuggee", NULL, NULL);
}

int main(void)
{
	pid_t child;

	// Creating child process. It will execute do_debuggee().
	// Parent process will continue to do_debugger() or run_debugger(child).
	child = fork();
	if (child == 0)
		do_debuggee();
	else if (child > 0)
		do_debugger();
		//run_debugger(child);
	else {
		perror("fork");
		return -1;
	}

	return 0;
}
