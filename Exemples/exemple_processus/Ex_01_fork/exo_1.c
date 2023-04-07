//
// Created by jordan on 27/03/23.
//
#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>

#include <errno.h>

int main(void)
{
    pid_t pid_fils;

    for (int i = 0; i < 5; ++i) {

        pid_fils = fork();

        if(pid_fils == 0) {
            for (int j = 0; j < 2 ; ++j) {
                printf("Fils : PPID : %ld, PID : %ld, counter: %i, born : %i ", (long) getppid(), (long) getpid(), j, i);
                printf("I was born at iteration %i \n", i);
            }
            break;
        }
        else{
            printf("Pere : PPID : %ld, PID : %ld \n", (long) getppid(), (long) getpid());
        }
    }


}