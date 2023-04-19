#include "watchdog.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

struct watchdog_t
{
	timer_t timer;
	unsigned int  delay;
	watchdog_callback callback;
};

/**
 * Function that is triggered at the end of the timer
 * @param val the signal value
 */
static void watchdog_function(union sigval val);

watchdog_t * watchdog_create(unsigned int delay, watchdog_callback callback)
{
    // Memory allocation
    watchdog_t *this = malloc(sizeof(watchdog_t));
    if (this == NULL) {
        perror("watchdog_create");
        exit(EXIT_FAILURE);
    }

    // Add parameters to watchdog structure
    this->delay = delay;
    this->callback = callback;

    // Create the signal targeted at the end of the timer
    struct sigevent event;
    event.sigev_notify = SIGEV_THREAD;
    event.sigev_notify_function = watchdog_function;
    event.sigev_value.sival_ptr = (void*) this;
    event.sigev_notify_attributes = NULL;

    // Create the timer
    if (timer_create(CLOCK_REALTIME, &event, &this->timer) != 0) {
        perror("watchdog_create");
        exit(EXIT_FAILURE);
    }

    return this;
}

void watchdog_start(watchdog_t * this)
{
    struct itimerspec itimer;
    itimer.it_value.tv_sec = this->delay;
    itimer.it_value.tv_nsec = 0;
    itimer.it_interval.tv_sec = 0;
    itimer.it_interval.tv_nsec = 0;

    if (timer_settime(this->timer, 0, &itimer, NULL) != 0) {
        perror("watchdog_start");
        exit(EXIT_FAILURE);
    }
}

void watchdog_cancel(watchdog_t * this)
{
    // Reset timer values
    struct itimerspec itimer;
    itimer.it_value.tv_sec = 0;
    itimer.it_value.tv_nsec = 0;
    itimer.it_interval.tv_sec = 0;
    itimer.it_interval.tv_nsec = 0;

    if (timer_settime(this->timer, 0, &itimer, NULL) != 0) {
        perror("watchdog_cancel");
        exit(EXIT_FAILURE);
    }

}

void watchdog_destroy(watchdog_t * this)
{
    timer_delete(this->timer);
    free(this);
}

static void watchdog_function(union sigval val)
{
    watchdog_t *this = (watchdog_t *) val.sival_ptr;
    this->callback(this);
}

