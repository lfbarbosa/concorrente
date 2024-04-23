#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define NUM_INGRESSOS 10
#define NUM_PESSOAS 15

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int ingressos_disponiveis = NUM_INGRESSOS;

void *compra_ingresso(void *thread_id) {
    int tid = *((int *)thread_id);
    int status_compra=0;
    while(status_compra==0)
    {
        pthread_mutex_lock(&mutex);

        int ingressos_desejados = 1+rand()%3;

        if (ingressos_disponiveis >= ingressos_desejados) {
            printf("Pessoa %d comprou %d ingressos.\n", tid,ingressos_desejados);
            ingressos_disponiveis-=ingressos_desejados;
            status_compra=1;
        } else {
            printf("Pessoa %d: Desculpe, não há mais ingressos disponíveis.\n", tid);
            pthread_cond_wait(&cond,&mutex);
        }

        pthread_mutex_unlock(&mutex);
        sleep(rand()%3);

    }
    pthread_exit(NULL);
}

void *reabastece_ingresso(void *arg){
    for(int i = 0; i<5; i++)
    {
        pthread_mutex_lock(&mutex);
        printf("foram disponibilizados mais 5 ingressos!\n");
        ingressos_disponiveis+=5;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
        sleep(5);
    }
}

int main() {
    pthread_t threads[NUM_PESSOAS];
    pthread_t reabastecimento;
    int thread_ids[NUM_PESSOAS];
    srand(time(NULL));

    for (int i = 0; i < NUM_PESSOAS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, compra_ingresso, (void *)&thread_ids[i]);
    }

    pthread_create(&reabastecimento,NULL,reabastece_ingresso,NULL);

    for (int i = 0; i < NUM_PESSOAS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}