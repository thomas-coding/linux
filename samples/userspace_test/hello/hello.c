#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

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

void main()
{
    printf("Userspace Hello World\n");
    fflush(stdout);
    //two_task_test();
}

