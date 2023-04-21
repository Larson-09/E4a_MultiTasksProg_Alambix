//
// Created by jordan on 21/04/23.
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

#ifndef E4A_MULTITHREADS_ALAMBIX_ALAMBIX_BIX_H
#define E4A_MULTITHREADS_ALAMBIX_ALAMBIX_BIX_H

/** CONSTANTS **/

#define ALAMBIX_BARTENDER_MQ_NAME "/alambix_bartender_mq"
#define MQ_MSG_MAX      10
#define MQ_MSG_SIZE     48
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0

/** GLOBAL VARIABLES **/

pthread_t alambix_client0_thread;
pthread_t alambix_client1_thread;
pthread_t alambix_waiter_thread;
pthread_t alambix_bartender_thread;

pthread_mutex_t order_mutex = PTHREAD_MUTEX_INITIALIZER;

sem_t sem_order_drink;
sem_t sem_provide_drink;

mqd_t mq;
struct mq_attr mq_bartender_attr;

struct sigevent event;
timer_t timer_drink_prepare;


/** FUNCTIONS **/

/**
 * @brief   Initializes alambix elements
 */
void alambix_init();

/**
 * @brief   Starts alambix threads
 */
void alambix_start();

/**
 * @brief   Opens the help page of the project in the configured browser
 */
void alambix_help();

/**
 * @brief   Function linked with the clients threads
 * @param   arg
 */

/**
 * @param arg
 * @return
 */
void * alambix_client_thread_fct(void * arg);

/**
 * @brief   Function linked with the waiter thread
 * @param   arg
 */
void * alambix_waiter_thread_fct(void * arg);

/**
 * @brief   Function linked with the bartender thread
 * @param   arg
 */
void * alambix_bartender_thread_fct(void * arg);

/**
 * @brief   Free memory of the used elements
 */
void alambix_destroy();

#endif //E4A_MULTITHREADS_ALAMBIX_ALAMBIX_BIX_H
