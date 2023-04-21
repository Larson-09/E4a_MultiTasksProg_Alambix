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

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <mqueue.h>

#define MQ_MSG_MAX (20)
#define MQ_MSG_SIZE (48)

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
pthread_barrier_t take_order_barrier;

mqd_t mq_bartender;
struct mq_attr attr;

void alambix_init()
{
    // Init
    pthread_barrier_init(&take_order_barrier, NULL, 3);

    // Set bartender message queue
    if ((mq = mq_open(ALAMBIX_mq_bartender, O_WRONLY)) == -1) {
        perror("mq_open()");
        exit(EXIT_FAILURE);
    }

    if (mq_getattr(mq, &attr) == -1) {
        perror("mq_getattr()");
        mq_close(mq);
        exit(EXIT_FAILURE);
    }

    attr.mq_maxmsg = MQ_MSG_MAX;
    attr.mq_msgsize = MQ_MSG_SIZE;
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
        pthread_mutex_lock(&order_mutex); // verrouille le mutex
        alambix_order_drink(drink);
        pthread_mutex_unlock(&order_mutex); // déverrouille le mutex
    }
    pthread_barrier_wait(&take_order_barrier);
}

void * alambix_waiter_thread_fct(void * arg)
{
    // Take clients order
    pthread_barrier_wait(&take_order_barrier);
    alambix_take_order();

    // transmit the order to the bartender
    char * ordered_drink;
    while ((ordered_drink = alambix_get_ordered_drink()) != NULL)
    {
        memset(buffer, 0, attr.mq_msgsize);

        if (mq_send(mq_bartender, ordered_drink, attr.mq_msgsize, 0) == -1) {
            perror("mq_send()");
            mq_close(mq);
            exit(EXIT_FAILURE);
        }
    }
}

void * alambix_bartender_thread_fct(void * arg){
    // receive the order from the waiter
    do
    {
        // receive the drink order using the bartender message queue

    }
    while (alambix_has_ordered_drink());
}


int main(int argc, char * argv[])
{
    alambix_open();
    return alambix_close();
}