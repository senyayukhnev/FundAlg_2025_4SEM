#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_PHILOSOPHERS 5
#define EATING_TIME 2
#define THINKING_TIME 1
#define SIMULATION_DURATION 1
#define DEADLOCK_DELAY 1

typedef struct {
    int id;
    sem_t *left_fork;
    sem_t *right_fork;
    sem_t *waiter;
    pthread_t thread;
    bool *running;
    bool *sync_mode;
} Philosopher;

void* safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void safe_sem_init(sem_t *sem, int pshared, unsigned int value) {
    if (sem_init(sem, pshared, value) != 0) {
        perror("Ошибка инициализации семафора");
        exit(EXIT_FAILURE);
    }
}

void philosopher_think(int id) {
    printf("Философ %d размышляет...\n", id);
    sleep(THINKING_TIME);
}

void philosopher_eat(int id) {
    printf("Философ %d ест...\n", id);
    sleep(EATING_TIME);
}

void* philosopher(void *arg) {
    Philosopher *phil = (Philosopher*)arg;

    while (*phil->running) {
        philosopher_think(phil->id);

        printf("Философ %d хочет взять вилки...\n", phil->id);

        if (*phil->sync_mode) {
            // Синхронизированная версия с официантом
            sem_wait(phil->waiter);
        }


        sem_wait(phil->left_fork);
        printf("Философ %d взял левую вилку (%d)\n", phil->id, phil->id);

        if (!*phil->sync_mode) {
            sleep(DEADLOCK_DELAY);
        }


        sem_wait(phil->right_fork);
        printf("Философ %d взял правую вилку (%d)\n", phil->id, (phil->id + 1) % NUM_PHILOSOPHERS);

        philosopher_eat(phil->id);

        sem_post(phil->left_fork);
        sem_post(phil->right_fork);

        if (*phil->sync_mode) {
            sem_post(phil->waiter);
        }

        printf("Философ %d положил вилки\n", phil->id);
    }

    return NULL;
}

void run_simulation(bool sync_mode) {
    sem_t forks[NUM_PHILOSOPHERS];
    sem_t waiter;
    Philosopher phils[NUM_PHILOSOPHERS];
    bool running = true;

    // Инициализация
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        safe_sem_init(&forks[i], 0, 1);
    }

    if (sync_mode) {
        safe_sem_init(&waiter, 0, NUM_PHILOSOPHERS - 1);
        printf("\n=== Режим с синхронизацией (deadlock невозможен) ===\n");
    } else {
        printf("\n=== Режим без синхронизации (deadlock гарантирован) ===\n");
        printf("Ожидайте deadlock через %d секунд...\n", DEADLOCK_DELAY + 1);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        phils[i].id = i;
        phils[i].left_fork = &forks[i];
        phils[i].right_fork = &forks[(i + 1) % NUM_PHILOSOPHERS];
        phils[i].waiter = sync_mode ? &waiter : NULL;
        phils[i].running = &running;
        phils[i].sync_mode = &sync_mode;
        pthread_create(&phils[i].thread, NULL, philosopher, &phils[i]);
    }


    sleep(SIMULATION_DURATION);
    running = false;

    // Очистка
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(phils[i].thread, NULL);
        sem_destroy(&forks[i]);
    }

    if (sync_mode) {
        sem_destroy(&waiter);
    }
}

int main() {
    //(гарантированный deadlock)
    // run_simulation(false);

    run_simulation(true);

    return 0;
}