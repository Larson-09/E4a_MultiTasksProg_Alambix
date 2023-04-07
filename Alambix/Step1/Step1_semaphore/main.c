//
// Created by jordan on 27/03/23.
//

//
// \file main.c
//
// \brief Source code example for Alambix programming.
//
#include "alambix.h"
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <mqueue.h>
#include <semaphore.h>
#include <signal.h>
#include "errno.h"

#define ALAMBIX_BARTENDER_MQ_NAME "/alambix_bartender_mq"
#define MQ_MSG_MAX      10
#define MQ_MSG_SIZE     48
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0

void alambix_init();
void alambix_start();
void alambix_help();
void * alambix_client_thread_fct(void * arg);
void * alambix_waiter_thread_fct(void * arg);
void * alambix_bartender_thread_fct(void * arg);


pthread_t alambix_client0_thread;
pthread_t alambix_client1_thread;
pthread_t alambix_waiter_thread;
pthread_t alambix_bartender_thread;

pthread_mutex_t order_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t sem;

mqd_t mq;
struct mq_attr mq_bartender_attr;

void signal_sigchld_handler(int signal)
{
    fprintf(stdout, "SIGCHLD intercepté (PID %d)\n", getpid());
    wait(NULL);
}

void alambix_init()
{
    // Init
    sem_init(&sem, 0, 0);

    // Init bartender message queue
    mq_bartender_attr.mq_maxmsg = MQ_MSG_MAX;
    mq_bartender_attr.mq_msgsize = MQ_MSG_SIZE;

    if ((mq = mq_open(ALAMBIX_BARTENDER_MQ_NAME, O_RDWR | O_CREAT | O_EXCL, 0644, &mq_bartender_attr)) == -1) {
        // ERROR management
        if(errno == EEXIST)
        {
            // Unlink and opean the mq again to clear it
            mq_unlink(ALAMBIX_BARTENDER_MQ_NAME);
            mq = mq_open(ALAMBIX_BARTENDER_MQ_NAME, O_RDWR | O_CREAT, 0644, NULL);
        }
        else{
            exit(-1);
        }
    }
}

void alambix_destroy(){
    sem_destroy(&sem);

    mq_unlink(ALAMBIX_BARTENDER_MQ_NAME);
    mq_close(mq);
}

void alambix_start(){

    // Start clients thread
    if (pthread_create(&alambix_client0_thread, NULL, alambix_client_thread_fct, NULL) != 0) {
        fprintf(stderr, "erreur pthread_create\n");
        exit(-1);
    }
    pthread_detach(alambix_client0_thread);

    if (pthread_create(&alambix_client1_thread, NULL, alambix_client_thread_fct, NULL) != 0) {
        fprintf(stderr, "erreur pthread_create\n");
        exit(-1);
    }
    pthread_detach(alambix_client1_thread);

    // Start waiter thread
    if (pthread_create(&alambix_waiter_thread, NULL, alambix_waiter_thread_fct, NULL) != 0) {
        fprintf(stderr, "erreur pthread_create\n");
        exit(-1);
    }
    pthread_detach(alambix_waiter_thread);

    // Start bartender thread
    if (pthread_create(&alambix_bartender_thread, NULL, alambix_bartender_thread_fct, NULL) != 0) {
        fprintf(stderr, "erreur pthread_create\n");
        exit(-1);
    }
    pthread_detach(alambix_waiter_thread);
}

void alambix_help()
{
    // TODO: Insert here the code to launch the Alambix help documentation in a browser.

    // Intercept signals to avoid zombies process
    struct sigaction action;
    action.sa_handler = signal_sigchld_handler;
    sigemptyset(&(action.sa_mask));
    action.sa_flags = SA_RESTART | SA_NOCLDSTOP; // SA_NOCLDSTOP : seulement quand un fils se termine (pas quand il est juste arrêté)

    sigaction(SIGCHLD, &action, NULL);

    // Launch help process
    char* path = alambix_help_html();

    pid_t pid_fils;
    pid_fils = fork();

    if (pid_fils == -1) {
        fprintf(stderr, "fork() impossible, errno = %d\n", errno);
        return 1;
    }

    if (pid_fils == 0) {
        char *argv[3] = {"firefox", path, (char *) NULL};
        execvp("firefox", argv);
    }
}

void * alambix_client_thread_fct(void * arg)
{
    char * drink;
    while ((drink = alambix_choose_drink()) != NULL)
    {
        // Launch the drink order
        pthread_mutex_lock(&order_mutex);
        alambix_order_drink(drink);
        pthread_mutex_unlock(&order_mutex);
    }
    // Put a coin on the table
    sem_post(&sem);
}

void * alambix_waiter_thread_fct(void * arg)
{
    // Take clients order
    sem_wait(&sem);
    sem_wait(&sem);
    alambix_take_order();

    // transmit the order to the bartender
    char * ordered_drink;
    while ((ordered_drink = alambix_get_ordered_drink()) != NULL)
    {
       // memset(ordered_drink, 0, mq_bartender_attr.mq_msgsize);

        if (mq_send(mq, ordered_drink, mq_bartender_attr.mq_msgsize, 0) == -1) {
            perror("mq_send()");
            mq_close(mq);
            exit(EXIT_FAILURE);
        }
    }
}

void * alambix_bartender_thread_fct(void * arg){
    // receive the order from the waiter
    char buffer[MQ_MSG_SIZE];
    do
    {
        if (mq_receive(mq, buffer, mq_bartender_attr.mq_msgsize, NULL) == -1) {
            perror("mq_receive()");
            mq_close(mq);
            mq_unlink(ALAMBIX_BARTENDER_MQ_NAME);
            exit(EXIT_FAILURE);
        }
    }
    while (alambix_has_ordered_drink());

}

int main(int argc, char * argv[])
{
    alambix_open();
    alambix_destroy();
    return alambix_close();
}