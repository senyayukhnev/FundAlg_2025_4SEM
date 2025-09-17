#pragma once

#include <sys/sem.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <random>

#include "logger.h"

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

struct Person {
    size_t waiting_time;
    size_t location;
    size_t destination;
};

class Elevator {
public:
    bool stop = false;
    Logger *logger;
    size_t serial_number{};
    size_t max_number_people_in_elevator{};

    size_t location = 1;
    std::vector<Person> waiting_people;

    struct ElevatorStatistics {
        size_t total_number_transported_people = 0;
        size_t total_number_covered_floors = 0;
        double average_waiting_time = 0;
    } elevator_statistics;

    enum Status {
        WAITING,
        OPENING_THE_DOORS,
        CLOSING_THE_DOORS,
        DISEMBARKATION,
        DRIVING_1,
        DRIVING_2
    } status = WAITING;

    std::vector<size_t> people_in_elevator;

    explicit Elevator(Logger *logger, size_t serial_number, size_t max_number_people_in_elevator = 8);

    friend std::ostream &operator<<(std::ostream &ostream, const Elevator &lift);
};

class House {
private:
    std::mutex mutex;
    Logger *logger;
    size_t numbers_elevators;
    size_t numbers_floors;
    size_t tick = 1;
    int sem_id;
    bool stop_house = false;

    std::vector<pthread_t> threads;

    struct ThreadData {
        House *house;
        size_t index_elevator;
    };

    std::vector<Person> waiting_people;

    void generate_random_people(size_t a = 0, size_t b = 2);

    static void elevator_update(House *house, Elevator &elevator);

    static void handler_driving_2(ThreadData * data, Elevator & elevator);
    static void handler_disembarkation(ThreadData * data, Elevator & elevator);

public:
    std::vector<Elevator> elevators;

    explicit House(size_t numbers_elevators, size_t numbers_floors);
    ~House();

    void work();
    static void *processing_house(void *arg);
    static void *processing(void *arg);

    friend std::ostream &operator<<(std::ostream &stream, House &house);
};

std::pair<size_t, size_t> get_args(int argc, char *argv[]);