//
// Created by jordan on 21/04/23.
//

#include "alambix_bix.h"





void signal_sigchld_handler(int signal)
{
    fprintf(stdout, "SIGCHLD catched (PID %d)\n", getpid());
    wait(NULL);
}

void alambix_init()
{
    // Init signal treatment
    struct sigaction action;
    action.sa_handler = signal_sigchld_handler;
    sigemptyset(&(action.sa_mask));
    action.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &action, NULL);

    // Init
    sem_init(&sem_order_drink, 0, 0);
    sem_init(&sem_provide_drink, 0, 0);

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

    // Init timer for drink preparation by the bartender
    event.sigev_notify = SIGEV_THREAD;
    event.sigev_notify_function = alambix_still_stop;
    event.sigev_notify_attributes = NULL;

    if (timer_create(CLOCK_REALTIME, &event, &timer_drink_prepare) != 0) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }
}

void alambix_destroy(){
    sem_destroy(&sem_order_drink);
    sem_destroy(&sem_provide_drink);

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
    sem_post(&sem_order_drink);
}

void * alambix_waiter_thread_fct(void * arg)
{
    // Take clients order
    sem_wait(&sem_order_drink);
    sem_wait(&sem_order_drink);
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

    // Wait for the order from the bartender
    sem_wait(&sem_provide_drink);
    alambix_serve_order();
}

void * alambix_bartender_thread_fct(void * arg){
    // receive the order from the waiter
    char buffer[MQ_MSG_SIZE];
    do
    {
        // Read the order
        if (mq_receive(mq, buffer, mq_bartender_attr.mq_msgsize, NULL) == -1) {
            perror("mq_receive()");
            mq_close(mq);
            mq_unlink(ALAMBIX_BARTENDER_MQ_NAME);
            exit(EXIT_FAILURE);
        }

        // Prepare the order
        char * drink_to_prepare = buffer;
        alambix_still_start(drink_to_prepare);

        event.sigev_value.sival_ptr = (void *)drink_to_prepare;

        struct itimerspec itimer;
        itimer.it_interval.tv_sec = 0;
        itimer.it_interval.tv_nsec = 0;
        itimer.it_value.tv_sec = 2;
        itimer.it_value.tv_nsec = 0;

        if (timer_settime(timer_drink_prepare, 0, &itimer, NULL) != 0) {
            perror("timer_settime");
            exit(EXIT_FAILURE);
        }
    }
    while (alambix_has_ordered_drink());

    // Provide the order to the waiter
    alambix_provide_order();
    sem_post(&sem_provide_drink);
}