#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void inc();

int main(void)
{
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, inc, NULL);
    pthread_create(&thread2, NULL, inc, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    printf("Counter value : %d\n", counter);
    return 0;
}

void inc() {
    int i;
    for (i = 0; i < 100000; i++) {
        pthread_mutex_lock(&mutex); // verrouille le mutex
        counter++;
        pthread_mutex_unlock(&mutex); // déverrouille le mutex
    }
}


