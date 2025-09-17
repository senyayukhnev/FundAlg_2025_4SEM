#include "../include/logs_generator.h"

LogsGenerator::LogsGenerator(SafeQueue<std::string> &q, const std::string &logger_name,
                             Logger::LevelLogger logger_level)
        : queue{q} {
    logger = Logger::Builder()
            .setName(logger_name)
            .setLevel(logger_level)
            .addHandler(std::make_unique<QueueLoggerHandler>(q))
            .addHandler(std::make_unique<FileLoggerHandler>("generate.log"))
            .build();
}

LogsGenerator::LogsGenerator(SafeQueue<std::string> &q, Logger *logger) : queue{q} {}

std::string LogsGenerator::ip_to_string(in_addr_t ip) {
    struct in_addr addr {};
    addr.s_addr = ip;
    return inet_ntoa(addr);
}

void *LogsGenerator::generate_sample_traffic(void *arg) {
    auto *instance = static_cast<LogsGenerator *>(arg);
    if (instance == nullptr) {
        return nullptr;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<in_addr_t> ip_dist(0x0A000001, 0x0A0000FF);
    std::uniform_int_distribution<in_port_t> port_dist(1024, 65535);
    std::uniform_int_distribution<size_t> size_dist(64, 4096);
    std::uniform_int_distribution<size_t> what_pkgs(0, 1);
    std::uniform_int_distribution<size_t> is_fatal(0, 100);  // Допустим вероятность выпадения ошибки 1%
    while (!instance->stop) {
        tcp_traffic traffic{};

        traffic.src_addr = ip_dist(gen);
        traffic.pkgs_sz = 2 + gen() % 10;

        traffic.pkgs = new tcp_traffic_pkg[traffic.pkgs_sz];

        tcp_traffic_pkg tmp_pkg = {
                .src_port = port_dist(gen),
                .dst_addr = ip_dist(gen),
                .dst_port = port_dist(gen),
        };
        while (tmp_pkg.dst_addr == traffic.src_addr) {
            tmp_pkg.dst_addr = ip_dist(gen);
        }


        std::string tmp1 = ip_to_string(traffic.src_addr) + ":" + std::to_string(ntohs(tmp_pkg.src_port)) + " → " +
                           ip_to_string(tmp_pkg.dst_addr) + ":" + std::to_string(ntohs(tmp_pkg.dst_port));
        std::string tmp2 = ip_to_string(tmp_pkg.dst_addr) + ":" + std::to_string(ntohs(tmp_pkg.dst_port)) + " ← " +
                           ip_to_string(traffic.src_addr) + ":" + std::to_string(ntohs(tmp_pkg.src_port));

        for (size_t i = 0; i < traffic.pkgs_sz; ++i) {
            traffic.pkgs[i] = tmp_pkg;
            traffic.pkgs[i].sz = size_dist(gen);
            if(i != 0 and is_fatal(gen) == 0) {
                instance->logger->LogError("DISCONNECT " + tmp1);
                break;
            }
            if (i == 0) {
                instance->logger->LogInfo("CONNECT " + tmp1);
            } else if (i != traffic.pkgs_sz - 1) {
                if (what_pkgs(gen) == 0) {
                    instance->logger->LogInfo("POST " + tmp1 + " (" + std::to_string(size_dist(gen)) + ")");
                } else {
                    instance->logger->LogInfo("GET " + tmp2 + " (" + std::to_string(size_dist(gen)) + ")");
                }
            } else {
                instance->logger->LogInfo("DISCONNECT " + tmp1);
            }
            usleep(10000);
        }
        delete[] traffic.pkgs;
    }
    return nullptr;
}

LogsGenerator::~LogsGenerator() {
    delete logger;
}

LogsGenerator &LogsGenerator::operator=(LogsGenerator &&other) noexcept {
    if (this != &other) {
        delete logger;
        logger = std::exchange(other.logger, nullptr);
        stop = std::exchange(other.stop, false);
    }
    return *this;
}

LogsGenerator::LogsGenerator(LogsGenerator &&other) noexcept
        : logger(std::exchange(other.logger, nullptr)),
          queue(other.queue),
          stop(std::exchange(other.stop, false)) {}

void QueueLoggerHandler::write(const std::string &str) { queue.push(str); }