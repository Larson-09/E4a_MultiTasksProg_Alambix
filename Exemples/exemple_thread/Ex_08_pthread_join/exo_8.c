#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "stdbool.h"

void * thread_read_fct(void * arg);
void * thread_write_fct(void * arg);

pthread_t thread_read;
pthread_t thread_write;

char content;
char stop = 's';
int is_new_content = 0;


int main(void)
{
    if (pthread_create(&thread_read, NULL, thread_read_fct, &content) != 0) {
        fprintf(stderr, "erreur pthread_create\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread_write, NULL, thread_write_fct, &content) != 0) {
        fprintf(stderr, "erreur pthread_create\n");
        exit(EXIT_FAILURE);
    }

    pthread_join(thread_read, NULL);
    pthread_join(thread_write, NULL);

    fprintf(stdout, "Thread main sortie\n");
    return EXIT_SUCCESS;
}

void * thread_read_fct(void * arg)
{
    while(content != stop){
        if(is_new_content == 0){
            printf("Entrer un caractere : ");
            content = getchar();
            printf("Lettre lue : %c\n", content);
            is_new_content = 1;
        }

    }
    return NULL;
}

void * thread_write_fct(void * arg)
{
    while (content != stop){
        if(is_new_content == 1){
            printf("Lettre affichee : %c\n", content);
            is_new_content = 0;
        }
        else{
            sleep(1);
        }
    }
    return NULL;
}


