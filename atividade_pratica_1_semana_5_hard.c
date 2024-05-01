#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define TAMANHO 60

typedef struct Argumentos {
    int identificador;
    pthread_cond_t *cond;
} Argumento;

pthread_cond_t *condicionais[TAMANHO];
pthread_cond_t **fila;
pthread_cond_t caixa_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t fila_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t fila_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t identificador_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t caixa_mutex = PTHREAD_MUTEX_INITIALIZER;

int proximo = 0;
int ultimo = -1;
int identificador = 0;

void usa_caixa(int usuario_id) {
    pthread_mutex_lock(&caixa_mutex);
    printf("Pessoa%d comecou a usar o caixa eletronico!\n", usuario_id);
    sleep(1 + rand() % 5);
    printf("Pessoa%d terminou o uso do caixa eletronico!\n", usuario_id);
    pthread_cond_signal(&caixa_cond);
    pthread_mutex_unlock(&caixa_mutex);
}

void espera_fila(int id, pthread_cond_t *cond) {
    pthread_mutex_lock(&fila_mutex);
    if (proximo > ultimo) {
        printf("A pessoa %d não encontrou fila!\n", id);
        ultimo++;
        fila[ultimo] = cond;
        pthread_cond_signal(&fila_cond);
    } else {
        printf("A pessoa %d está na posição %d da fila!\n", id, (ultimo - proximo));
        ultimo++;
        fila[ultimo] = cond;
    }
    pthread_mutex_unlock(&fila_mutex);
}

void *pessoas(void *argumentos) {
    sleep(rand()%10);
    struct Argumentos *argumento = (struct Argumentos *)argumentos;
    int pessoa_id = argumento->identificador;
    espera_fila(pessoa_id, argumento->cond);
    pthread_cond_wait(argumento->cond, &caixa_mutex);
    usa_caixa(pessoa_id);
    return NULL;
}

void *gerador_pessoas(void *arg) {
    pthread_t *pessoa[TAMANHO / 2];
    for (int i = 0; i < TAMANHO / 2; i++) {
        sleep(rand() % 3);
        pthread_mutex_lock(&identificador_mutex);
        Argumento *argumento = malloc(sizeof(Argumento));
        argumento->identificador = identificador;
        argumento->cond = condicionais[i];
        pthread_create(&pessoa[i], NULL, pessoas, argumento);
        identificador++;
        pthread_mutex_unlock(&identificador_mutex);
    }
    pthread_exit(NULL);
}

void *fila_gerenciador(void *args) {
    while (1) {
        pthread_mutex_lock(&fila_mutex);
        if (proximo <= ultimo) {
            pthread_cond_signal(fila[proximo]);
            pthread_cond_wait(&caixa_cond, &fila_mutex);
            proximo++;
        } else {
            pthread_cond_wait(&fila_cond, &fila_mutex);
        }
        pthread_mutex_unlock(&fila_mutex);
    }
    return NULL;
}

int main() {
    fila = malloc(TAMANHO * sizeof(pthread_cond_t *));
    pthread_t chegada_pessoas[2];
    srand(time(NULL));

    for (int i = 0; i < TAMANHO; i++) {
        condicionais[i] = malloc(sizeof(pthread_cond_t));
        pthread_cond_init(condicionais[i], NULL);
    }

    for (int i = 0; i < 2; i++) {
        pthread_create(&chegada_pessoas[i], NULL, gerador_pessoas, NULL);
    }

    pthread_t fila_manager;
    pthread_create(&fila_manager, NULL, fila_gerenciador, NULL);

    sleep(60);

    return 0;
}