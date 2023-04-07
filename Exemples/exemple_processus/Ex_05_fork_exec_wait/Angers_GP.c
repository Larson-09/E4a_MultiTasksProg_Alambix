//
// Created by jordan on 31/03/23.
//

#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>

#include <errno.h>

#include <stdlib.h>

#include <sys/wait.h>

void race(){
    srand((long)getpid());
    int nb_laps = rand() % 5001 + 5000;
    for (int j = 0; j <nb_laps; ++j) {
        int x = 0;
    }
}

int main(void)
{
    int nb_racers = 5;
    pid_t pid_fils;

    for (int i = 0; i < nb_racers; ++i) {

        pid_fils = fork();

        if (pid_fils == 0) {
            race();
            exit(0);
        }
    }

    // Afficher le classement
    int leaderboard[nb_racers];
    for (int i = 0; i < nb_racers; ++i) {
        int status;
        leaderboard[i] = (long) wait(&status);
    }

    for (int i = 0; i < nb_racers; i++) {
        printf("%i : %ld \n", i + 1, leaderboard[i]);
    }
}
