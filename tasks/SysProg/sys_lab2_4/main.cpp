#include "src/elevator.cpp"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <num_elevators> <num_floors>\n";
        return 1;
    }
    size_t num_elevators = 0;
    size_t num_floors = 0;
    try {
        num_elevators = std::stoul(argv[1]);
        num_floors = std::stoul(argv[2]);
    } catch (const std::exception &e) {
        std::cerr << "Arg parsing error: " << e.what() << std::endl;
        return 1;
    }

    if (num_elevators == 0 || num_floors < 2) {
        std::cerr << "Bad args: elevators > 0, floors >= 2.\n";
        return 1;
    }

    std::shared_ptr<Logger> logger =
            Logger::Builder().add_file_handler("elevator_sim.log").set_log_level(LogLevel::INFO).build();

    if (!logger) {
        std::cerr << "Logger init failed. Exiting." << std::endl;
        return 1;
    }

    House house(num_elevators, num_floors, logger.get());

    std::thread sim_thread([&house]() { house.run_simulation(); });

    std::string cmd;
    while (true) {
        std::cout << "\nCmd ('stats', 'exit'): ";
        std::cin >> cmd;

        if (std::cin.fail() || std::cin.eof()) {
            logger->info("Input error/EOF, stopping.");
            house.signal_stop();
            break;
        }

        if (cmd == "stats") {
            house.print_statistics();
        } else if (cmd == "exit") {
            logger->info("User 'exit' cmd.");
            house.signal_stop();
            break;
        } else {
            std::cout << "Unknown cmd: " << cmd << std::endl;
        }
    }

    if (sim_thread.joinable()) {
        sim_thread.join();
    }

    house.print_statistics();
    return 0;
}