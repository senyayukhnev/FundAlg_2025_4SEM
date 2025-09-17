#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include <csignal>
#include <iostream>
#include <random>

#include "logger.h"
#include "safe_queue.h"

struct tcp_traffic_pkg {
    in_port_t src_port;
    in_addr_t dst_addr;
    in_port_t dst_port;
    size_t sz;
};

struct tcp_traffic {
    in_addr_t src_addr;
    struct tcp_traffic_pkg *pkgs;
    size_t pkgs_sz;
};

class QueueLoggerHandler : public LogHandler {
protected:
    SafeQueue<std::string> &queue;

public:
    explicit QueueLoggerHandler(SafeQueue<std::string> &q) : queue(q){};
    void write(const std::string &str) override;
};

class LogsGenerator {
private:
    Logger *logger = nullptr;
    SafeQueue<std::string> &queue;

public:
    bool stop = false;
    static std::string ip_to_string(in_addr_t ip);

    explicit LogsGenerator(SafeQueue<std::string> &q, const std::string &logger_name, Logger::LevelLogger logger_level);
    explicit LogsGenerator(SafeQueue<std::string> &q, Logger * logger);
    LogsGenerator(LogsGenerator&& other) noexcept;
    LogsGenerator& operator=(LogsGenerator&& other) noexcept;
    ~LogsGenerator();
    static void *generate_sample_traffic(void *arg);
};