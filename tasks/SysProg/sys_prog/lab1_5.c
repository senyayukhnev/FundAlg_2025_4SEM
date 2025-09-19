#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    BATHROOM_EMPTY,
    BATHROOM_WOMEN_ONLY,
    BATHROOM_MEN_ONLY
} BathroomState;

typedef struct {
    BathroomState state;
    int women_count;
    int men_count;
    int max_people;
    pthread_mutex_t mutex;
    sem_t women_sem;
    sem_t men_sem;
} Bathroom;

void bathroom_init(Bathroom *br, int max_people) {
    br->state = BATHROOM_EMPTY;
    br->women_count = 0;
    br->men_count = 0;
    br->max_people = max_people;
    pthread_mutex_init(&br->mutex, NULL);
    sem_init(&br->women_sem, 0, max_people);
    sem_init(&br->men_sem, 0, max_people);
}

void bathroom_destroy(Bathroom *br) {
    pthread_mutex_destroy(&br->mutex);
    sem_destroy(&br->women_sem);
    sem_destroy(&br->men_sem);
}

void print_bathroom_state(Bathroom *br) {
    const char *states[] = {
            "Пустая",
            "Только женщины",
            "Только мужчины"
    };
    printf("Состояние: %s, Женщин: %d, Мужчин: %d\n",
           states[br->state], br->women_count, br->men_count);
}

bool woman_wants_to_enter(Bathroom *br) {
    pthread_mutex_lock(&br->mutex);

    if (br->state == BATHROOM_MEN_ONLY) {
        pthread_mutex_unlock(&br->mutex);
        return false;
    }

    if (sem_trywait(&br->women_sem)) {
        pthread_mutex_unlock(&br->mutex);
        return false;
    }

    if (br->state == BATHROOM_EMPTY) {
        br->state = BATHROOM_WOMEN_ONLY;
    }

    br->women_count++;
    print_bathroom_state(br);

    pthread_mutex_unlock(&br->mutex);
    return true;
}

bool man_wants_to_enter(Bathroom *br) {
    pthread_mutex_lock(&br->mutex);

    if (br->state == BATHROOM_WOMEN_ONLY) {
        pthread_mutex_unlock(&br->mutex);
        return false;
    }

    if (sem_trywait(&br->men_sem)) {
        pthread_mutex_unlock(&br->mutex);
        return false;
    }

    if (br->state == BATHROOM_EMPTY) {
        br->state = BATHROOM_MEN_ONLY;
    }

    br->men_count++;
    print_bathroom_state(br);

    pthread_mutex_unlock(&br->mutex);
    return true;
}

void woman_leaves(Bathroom *br) {
    pthread_mutex_lock(&br->mutex);

    br->women_count--;
    sem_post(&br->women_sem);

    if (br->women_count == 0) {
        br->state = BATHROOM_EMPTY;
    }

    print_bathroom_state(br);
    pthread_mutex_unlock(&br->mutex);
}

void man_leaves(Bathroom *br) {
    pthread_mutex_lock(&br->mutex);

    br->men_count--;
    sem_post(&br->men_sem);

    if (br->men_count == 0) {
        br->state = BATHROOM_EMPTY;
    }

    print_bathroom_state(br);
    pthread_mutex_unlock(&br->mutex);
}

typedef struct {
    int id;
    bool is_woman;
    Bathroom *bathroom;
    int iterations;
} PersonData;

void* person_thread(void *arg) {
    PersonData *data = (PersonData*)arg;

    for (int i = 0; i < data->iterations; i++) {
        sleep(rand() % 3 + 1); // Имитация времени до следующего похода

        bool entered = false;
        if (data->is_woman) {
            printf("Женщина %d хочет войти в ванную (попытка %d)\n", data->id, i+1);
            entered = woman_wants_to_enter(data->bathroom);
        } else {
            printf("Мужчина %d хочет войти в ванную (попытка %d)\n", data->id, i+1);
            entered = man_wants_to_enter(data->bathroom);
        }

        if (entered) {
            sleep(rand() % 5 + 1); // Время в ванной

            if (data->is_woman) {
                woman_leaves(data->bathroom);
            } else {
                man_leaves(data->bathroom);
            }
        } else {
            printf("%s %d не смог(ла) войти, попробует позже\n",
                   data->is_woman ? "Женщина" : "Мужчина", data->id);
        }
    }

    free(data);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Использование: %s <макс_людей> <кол-во_потоков>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int max_people = atoi(argv[1]);
    int num_threads = atoi(argv[2]);

    if (max_people <= 0 || num_threads <= 0) {
        printf("Ошибка: аргументы должны быть положительными числами\n");
        return EXIT_FAILURE;
    }

    Bathroom bathroom;
    bathroom_init(&bathroom, max_people);

    pthread_t threads[num_threads];

    for (int i = 0; i < num_threads; i++) {
        PersonData *data = malloc(sizeof(PersonData));
        data->id = i + 1;
        data->is_woman = rand() % 2;
        data->bathroom = &bathroom;
        data->iterations = rand() % 3 + 1;

        pthread_create(&threads[i], NULL, person_thread, data);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    bathroom_destroy(&bathroom);
    return EXIT_SUCCESS;
}