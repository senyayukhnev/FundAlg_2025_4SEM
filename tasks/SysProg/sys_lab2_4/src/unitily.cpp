#include "../include/unility.h"

std::pair<size_t, size_t> get_args(int argc, char **argv) {
    if (argc != 3) {
        throw std::invalid_argument("must be 2 parameters: numbers_elevators numbers_floors");
    }
    size_t numbers_elevators = std::strtoul(argv[1], nullptr, 10);
    size_t numbers_floors = std::strtoul(argv[2], nullptr, 10);
    if (numbers_elevators == 0 or numbers_floors < 2) {
        throw std::invalid_argument("incorrect input data: numbers_elevators >= 1, numbers_floors >= 2");
    }
    return {numbers_elevators, numbers_floors};
}

Elevator::Elevator(Logger *logger, size_t serial_number, size_t max_number_people_in_elevator)
        : logger{logger}, serial_number{serial_number}, max_number_people_in_elevator{max_number_people_in_elevator} {}

std::ostream &operator<<(std::ostream &ostream, const Elevator &elevator) {
    std::cout << "---------------------------------------\nLift " << elevator.serial_number << ":\n"
              << "max_number_people_in_elevator: " << elevator.max_number_people_in_elevator << std::endl
              << "location: " << elevator.location << std::endl
              << "real_number_people: " << elevator.people_in_elevator.size() << std::endl
              << "waiting people: " << elevator.waiting_people.size() << std::endl
              << "total covered floors: " << elevator.elevator_statistics.total_number_covered_floors << std::endl
              << "total transported people: " << elevator.elevator_statistics.total_number_transported_people
              << std::endl
              << "---------------------------------------" << std::endl;

    return ostream;
}

House::House(size_t numbers_elevators, size_t numbers_floors)
        : numbers_elevators{numbers_elevators}, numbers_floors{numbers_floors} {
    if (numbers_elevators == 0 or numbers_floors < 2) {
        throw std::invalid_argument("incorrect input data: numbers_elevators >= 1, numbers_floors >= 2");
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> max_number_people_in_elevator(2, 8);

    logger = Logger::Builder()
            .setLevel(Logger::LOG_INFO)
            .setName("House")
            .addHandler(std::make_unique<FileLoggerHandler>("house.log"))
            .build();

    for (size_t i = 0; i < numbers_elevators; ++i) {
        elevators.emplace_back(logger, i + 1, max_number_people_in_elevator(gen));
    }
    logger->LogInfo("House init");

    key_t key = ftok("/tmp", 13);
    if (key == -1) {
        throw std::runtime_error("ftok");
    }

    sem_id = semget(key, (unsigned short)numbers_elevators, IPC_CREAT | IPC_EXCL | 0666);
    if (sem_id == -1) {
        throw std::runtime_error("semget");
    }

    {
        std::lock_guard<std::mutex> lock(mutex);
        for (size_t i = 0; i < numbers_elevators; ++i) {
            threads.push_back(pthread_t{});
            ThreadData *data = new ThreadData{this, i};
            pthread_create(&threads[i], nullptr, processing, (void *)data);
        }
        threads.push_back(pthread_t{});
        pthread_create(&threads[threads.size() - 1], nullptr, processing_house, this);
    }
}

House::~House() {
    std::lock_guard<std::mutex> lock(mutex);
    stop_house = true;
    for (size_t i = 0; i < numbers_elevators; ++i) {
        elevators[i].stop = true;
    }

    std::vector<unsigned short> values(numbers_elevators, 3);
    union semun arg = {.array = values.data()};
    semctl(sem_id, 0, SETALL, arg);

    for (unsigned long thread : threads) {
        pthread_join(thread, nullptr);
    }
    semctl(sem_id, 0, IPC_RMID);
    logger->close_logger();
    delete logger;
}

void *House::processing(void *arg) {
    ThreadData *data = static_cast<ThreadData *>(arg);
    if (data == nullptr) {
        return nullptr;
    }

    Elevator &elevator = data->house->elevators[data->index_elevator];

    struct sembuf buf = {.sem_num = (unsigned short)data->index_elevator, .sem_op = -1, .sem_flg = 0};

    int i = 0;
    while (true) {
        if (elevator.stop) {
            break;
        }

        buf.sem_op = -2;
        semop(data->house->sem_id, &buf, 1);

        if (elevator.stop) {
            break;
        }


        std::lock_guard<std::mutex> lock(data->house->mutex);

        switch (elevator.status) {
            case Elevator::WAITING:
                if (elevator.waiting_people.empty() and elevator.people_in_elevator.empty() and
                    !data->house->waiting_people.empty()) {
                    Person p = data->house->waiting_people.back();
                    data->house->waiting_people.pop_back();
                    elevator.waiting_people.push_back(p);
                    if (p.location == elevator.location) {
                        elevator.status = Elevator::OPENING_THE_DOORS;
                    } else {
                        elevator.status = Elevator::DRIVING_1;
                    }
                }
                break;
            case Elevator::OPENING_THE_DOORS:
                elevator.status = Elevator::DISEMBARKATION;
                elevator_update(data->house, elevator);
                elevator.logger->LogInfo("Tick " + std::to_string(data->house->tick) + " elevator " +
                                         std::to_string(elevator.serial_number) + " Floor " +
                                         std::to_string(elevator.location) + " open door");
                break;
            case Elevator::CLOSING_THE_DOORS:
                if (elevator.people_in_elevator.empty() and elevator.waiting_people.empty()) {
                    elevator.status = Elevator::WAITING;
                } else {
                    elevator.status = Elevator::DRIVING_1;
                }
                elevator_update(data->house, elevator);
                elevator.logger->LogInfo("Tick " + std::to_string(data->house->tick) + " elevator " +
                                         std::to_string(elevator.serial_number) + " Floor " +
                                         std::to_string(elevator.location) + " close door");
                break;
            case Elevator::DISEMBARKATION:
                handler_disembarkation(data, elevator);
                break;
            case Elevator::DRIVING_1:
                elevator.status = Elevator::DRIVING_2;
                elevator_update(data->house, elevator);
                break;
            case Elevator::DRIVING_2:
                handler_driving_2(data, elevator);
                break;
            default:
                std::cout << "Incorrect elevator status" << std::endl;
                break;
        }

        buf.sem_op = 1;
        semop(data->house->sem_id, &buf, 1);

        i++;
    }

    delete data;
    return nullptr;
}

void House::work() {
    std::string input;
    while (true) {
        std::cout << "Write:\n1.print\n2.print_all\n3.stop" << std::endl;
        std::cin >> input;

        if (input == "print" or input == "1") {
            std::cout << *this;
        } else if (input == "print_all" or input == "2") {
            {
                std::lock_guard<std::mutex> lock(mutex);
                for (size_t i = 0; i < numbers_elevators; ++i) {
                    std::cout << elevators[i];
                }
            }
        } else if (input == "stop" or input == "3") {
            break;
        }
    }
    {
        std::lock_guard<std::mutex> lock(mutex);
        stop_house = true;
        for (auto &el : elevators) {
            el.stop = true;
        }
    }
    // Принудительно разблокируем семафоры
    std::vector<unsigned short> values(numbers_elevators, 3);
    union semun arg_s = {.array = values.data()};
    semctl(sem_id, 0, SETALL, arg_s);
}

std::ostream &operator<<(std::ostream &stream, House &house) {
    std::lock_guard<std::mutex> lock(house.mutex);
    std::cout << "Waiting: " << house.waiting_people.size() << std::endl;
    for (long i = (long)house.numbers_floors; i > 0; --i) {
        std::cout << std::setfill(' ') << std::setw(2) << i << " ";
        for (auto &el : house.elevators) {
            if (i == el.location) {
                std::cout << "u ";
            } else {
                std::cout << "  ";
            }
        }
        std::cout << std::endl;
    }
    return stream;
}

void House::generate_random_people(size_t a, size_t b) {
    std::lock_guard<std::mutex> lock(mutex);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> floor(1, numbers_floors);
    std::uniform_int_distribution<size_t> number_people(a, b);

    for (size_t i = 0; i < number_people(gen); ++i) {
        size_t e1 = floor(gen);
        size_t e2 = floor(gen);
        while (e1 == e2) {
            e2 = floor(gen);
        }
        waiting_people.push_back({0, e1, e2});
    }
}

void House::elevator_update(House *house, Elevator &elevator) {
    for (auto it = house->waiting_people.begin(); it != house->waiting_people.end();) {
        if (elevator.people_in_elevator.size() + elevator.waiting_people.size() ==
            elevator.max_number_people_in_elevator) {
            return;
        }
        Person p = *it;
        if (elevator.people_in_elevator.empty() and elevator.waiting_people.empty()) {
            elevator.waiting_people.push_back(p);
            it = house->waiting_people.erase(it);
        } else if (!elevator.people_in_elevator.empty()) {
            // Вариант, что в лифте есть люди -> ориентируемся по ним
            size_t dr = elevator.people_in_elevator[0];
            // Проверяем что человек нам подходит
            if ((dr - elevator.location) * (p.location - elevator.location) > 0 and
                (dr - elevator.location) * (p.destination - p.location) > 0) {
                elevator.waiting_people.push_back(p);
                it = house->waiting_people.erase(it);
            } else {
                ++it;
            }
        } else if (!house->waiting_people.empty()) {
            // Вариант, что в лифте никого нет -> ориентируемся по ожидающим людям
            Person dr = elevator.waiting_people[0];
            if ((dr.destination - dr.location) * (p.destination - p.location) > 0 and
                ((dr.location - elevator.location) * (p.location - elevator.location) > 0)) {
                elevator.waiting_people.push_back(p);
                it = house->waiting_people.erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
}

void *House::processing_house(void *arg) {
    House *house = static_cast<House *>(arg);
    if (house == nullptr) {
        return nullptr;
    }
    while (!house->stop_house) {
        house->generate_random_people();
        {
            std::lock_guard<std::mutex> lock(house->mutex);
            house->tick += 1;
            for (auto &p : house->waiting_people) {
                p.waiting_time += 1;
            }
        }
        usleep(200000);
        std::vector<unsigned short> values(house->numbers_elevators, 2);
        union semun arg_s = {.array = values.data()};
        semctl(house->sem_id, 0, SETALL, arg_s);

        for (unsigned short i = 0; i < house->numbers_elevators; ++i) {
            struct sembuf buf = {.sem_num = i, .sem_op = -1, .sem_flg = 0};
            semop(house->sem_id, &buf, 1);
        }
    }
    return nullptr;
}

void House::handler_driving_2(House::ThreadData *data, Elevator &elevator) {
    elevator.elevator_statistics.total_number_covered_floors += 1;
    size_t p;
    if (!elevator.people_in_elevator.empty()) {
        p = elevator.people_in_elevator[0];
    } else {
        p = elevator.waiting_people[0].location;
    }

    if (elevator.location != p) {
        if (elevator.location > p) {
            elevator.logger->LogInfo("Tick " + std::to_string(data->house->tick) + " elevator " +
                                     std::to_string(elevator.serial_number) + " Floor " +
                                     std::to_string(elevator.location) + " down");
            elevator.location -= 1;

        } else {
            elevator.logger->LogInfo("Tick " + std::to_string(data->house->tick) + " elevator " +
                                     std::to_string(elevator.serial_number) + " Floor " +
                                     std::to_string(elevator.location) + " up");
            elevator.location += 1;
        }
    }
    if (elevator.location == 0 or elevator.location > data->house->numbers_floors) {
        throw std::runtime_error("elevator isn't good: " + std::to_string(elevator.location));
    }

    {
        bool check = std::find(elevator.people_in_elevator.begin(), elevator.people_in_elevator.end(),
                               elevator.location) != elevator.people_in_elevator.end();
        for (Person &person : elevator.waiting_people) {
            if(check or person.location == elevator.location) {
                check = true;
                break;
            }
        }
        if (check) {
            elevator.status = Elevator::OPENING_THE_DOORS;
        } else {
            elevator.status = Elevator::DRIVING_1;
        }
    }

    elevator_update(data->house, elevator);
}

void House::handler_disembarkation(House::ThreadData *data, Elevator &elevator) {
    // сажаем людей и высаживаем людей + статистика -> закрытие дверей
    for (auto it = elevator.people_in_elevator.begin(); it != elevator.people_in_elevator.end();) {
        size_t p = *it;
        if (p == elevator.location) {
            elevator.elevator_statistics.total_number_transported_people += 1;
            it = elevator.people_in_elevator.erase(it);
            elevator.logger->LogInfo("Tick " + std::to_string(data->house->tick) + " elevator " +
                                     std::to_string(elevator.serial_number) + " Floor " +
                                     std::to_string(elevator.location) + " disembarkation out");
        } else {
            ++it;
        }
    }
    //сажаем
    for (auto it = elevator.waiting_people.begin(); it != elevator.waiting_people.end();) {
        Person p = *it;
        if (p.location == elevator.location) {
            elevator.people_in_elevator.push_back(p.destination);
            // Обновляем среднее время ожидания в лифте
            elevator.elevator_statistics.average_waiting_time =
                    (elevator.elevator_statistics.average_waiting_time *
                     elevator.elevator_statistics.total_number_transported_people +
                     p.waiting_time) /
                    (double)(elevator.elevator_statistics.total_number_transported_people + 1);
            it = elevator.waiting_people.erase(it);
            elevator.logger->LogInfo("Tick " + std::to_string(data->house->tick) + " elevator " +
                                     std::to_string(elevator.serial_number) + " Floor " +
                                     std::to_string(elevator.location) + " disembarkation in");
        } else {
            ++it;
        }
    }
    elevator.status = Elevator::CLOSING_THE_DOORS;
    elevator_update(data->house, elevator);

}