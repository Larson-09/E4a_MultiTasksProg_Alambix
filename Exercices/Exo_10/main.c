#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>        /* pour les constantes O_* */
#include <sys/stat.h>     /* pour les modes d'ouverture */
#include <mqueue.h>

#include <signal.h>
#include "errno.h"

int ask_for_action(pid_t pid);

int main()
{

    pid_t pid_fils;
    int status;

    pid_fils = fork();
    if (pid_fils == -1) {
        fprintf(stderr, "fork() impossible, errno = %d\n", errno);
        exit(EXIT_FAILURE);
    }

    if (pid_fils == 0) {
        long long int n = 1000000000;
        long long int sum = 0;
        for (long long int i = 1; i <= n; i++) {
            sum += i;
        }
    } else {
        ask_for_action(pid_fils);
    }
}


int ask_for_action(pid_t pid_fils) {
    char choix;
    int status;

    printf("s - sleep \nr - restart \nq - quit \n");
    scanf("%c", &choix);
    printf("Choix %c\n", choix);

    switch (choix) {
        case 's':
            printf("s\n");
            sleep(1);
            kill(pid_fils, SIGSTOP); // Endormir le fils
            break;
        case 'r':
            printf("r\n");
            kill(pid_fils, SIGCONT); // Réveiller le fils
            wait(&status);
            break;
        case 'q':
            printf("q\n");
            sleep(5);
            kill(pid_fils, SIGINT);
            wait(&status);
            break;
        default:
            printf("Vous n'avez pas choisi une lettre valide.\n");
            break;
    }

    return 0;
}
