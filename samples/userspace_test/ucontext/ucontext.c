/* User context switch test */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ucontext.h>

int main()
{
	int i = 0;
	ucontext_t ctx;

    /* Save context */
	getcontext(&ctx);

	printf("i = %d\n", i++);
	sleep(1);

    /* Restore context */
	setcontext(&ctx);
	return 0;
}
