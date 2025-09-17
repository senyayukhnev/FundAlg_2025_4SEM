#pragma once

#include "logs_analyzer.h"
#include "logs_generator.h"

class Traffic {
    SafeQueue<std::string> q;
    size_t count_generate_threads;
    size_t count_analyze_threads;
    std::vector<LogsGenerator> generators;
    std::vector<LogsAnalyzer> analyzers;
    std::mutex mutex;

public:
    explicit Traffic(size_t count_generate_threads = 3, size_t count_analyze_threads = 3);

    static void *start_traffic(void *arg);
    static void *analyze_traffic(void *arg);

    static void *dialog(void *arg);

    [[nodiscard]] std::map<in_addr_t, Stats> get_info();
    [[nodiscard]] std::map<in_addr_t, Stats> get_info(in_addr_t addr);
    static void print_and_delete_info(const std::map<in_addr_t, Stats> &info);
    static void print_and_delete_ip(const std::pair<in_addr_t, Stats> &el);
};