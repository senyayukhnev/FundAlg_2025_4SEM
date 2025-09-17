#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "logs_generator.h"

struct Stats {
    size_t total_received = 0;
    size_t total_sent = 0;
    std::map<in_addr_t, tcp_traffic> connected;
    std::vector<in_addr_t> disconnected;
};

class LogsAnalyzer {
private:
    SafeQueue<std::string> &queue;
    std::map<in_addr_t, Stats> info;
    Logger *logger;

    static void connect(LogsAnalyzer *analyzer, in_addr_t dst_addr, in_port_t dst_port, in_addr_t src_addr,
                        in_port_t src_port);

    static void disconnect(LogsAnalyzer *analyzer, in_addr_t dst_addr, in_port_t dst_port, in_addr_t src_addr,
                           in_port_t src_port);

    static void get(LogsAnalyzer *analyzer, in_addr_t dst_addr, in_port_t dst_port, in_addr_t src_addr,
                    in_port_t src_port, size_t size_information);

    static void post(LogsAnalyzer *analyzer, in_addr_t dst_addr, in_port_t dst_port, in_addr_t src_addr,
                     in_port_t src_port, size_t size_information);

public:
    bool stop = false;
    [[nodiscard]] std::map<in_addr_t, Stats> get_info();
    [[nodiscard]] std::map<in_addr_t, Stats> get_info(in_addr_t ip);
    explicit LogsAnalyzer(SafeQueue<std::string> &queue);
    ~LogsAnalyzer();
    LogsAnalyzer(LogsAnalyzer && other) noexcept;
    LogsAnalyzer& operator=(LogsAnalyzer &&other) noexcept;
    static void *analyzing_traffic(void *arg);

    static in_addr_t string_to_api(const std::string &ip_str);
    static std::vector<std::string> split(const std::string &s, char delimiter);
};