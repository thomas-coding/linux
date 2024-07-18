#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define UART_BASE 0x10000000
void *uart_userspace_addr;

void two_task_test(void) {
	pid_t pid = 0;

	pid = fork();
	if(pid == 0) {//clild
		while(1) {
			printf("I am child\n");
			//sleep(10);
		}
	} else {//parent
		while(1) {
			printf("I am parent\n");
			//sleep(10);
		}
	}
}

void uart_map(void) {
	int mem_fd;
	size_t pagesize = getpagesize();

	mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mem_fd < 0) {
		printf ("Couldn't open /dev/mem. Are you root?\n");
		return;
	}

	uart_userspace_addr = mmap(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0x10000000);
	if (uart_userspace_addr == MAP_FAILED) {
		perror("Unable to map uart register memory");
		return;
	}

	printf("uart mmap addr:0x%lx\n", uart_userspace_addr);

	close(mem_fd);
}

void uart_unmap(void) {
	size_t pagesize = getpagesize();
	munmap(uart_userspace_addr, pagesize);
}

void debug_output_char(char a)
{
	*(volatile uint32_t *)(uart_userspace_addr) = a;
	/* delay */
	for(int i = 0; i < 0x4888; i++)
		asm volatile("NOP");
}

void bm_printf(char *s)
{
	while (*s) {
		debug_output_char(*s);
		s++;
	}
}

void main()
{
	printf("Userspace Hello World\n");
	uart_map();
	bm_printf("this is bm printf\n");
	uart_unmap();
	fflush(stdout);
	//two_task_test();

	while(1);
}


