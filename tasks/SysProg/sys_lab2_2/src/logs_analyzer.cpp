#include "../include/logs_analyzer.h"

LogsAnalyzer::LogsAnalyzer(SafeQueue<std::string> &queue) : queue(queue) {
    logger = Logger::Builder()
            .setName("analyzer")
            .setLevel(Logger::LOG_WARNING)
            .build();
}

in_addr_t LogsAnalyzer::string_to_api(const std::string &ip_str) {
    struct in_addr addr {};
    int result = inet_pton(AF_INET, ip_str.c_str(), &addr);
    if (result <= 0) {
        return 0;
    }
    return addr.s_addr;
}

std::vector<std::string> LogsAnalyzer::split(const std::string &s, char delimiter) {
    std::vector<std::string> res;
    std::string token;
    std::stringstream in(s);

    while (std::getline(in, token, delimiter)) {
        if (!token.empty()) {
            res.push_back(token);
        }
    }
    return res;
}

void *LogsAnalyzer::analyzing_traffic(void *arg) {
    auto analyzer = static_cast<LogsAnalyzer *>(arg);
    if (analyzer == nullptr) {
        return nullptr;
    }
    while (!analyzer->stop) {
        std::string src = analyzer->queue.pop();
        std::vector<std::string> input = split(src, ' ');
        if (input.size() != 7 and input.size() != 8) {
            analyzer->logger->LogWarning("incorrect format log: " + src + " => skip");
            return nullptr;
        }
        std::vector<std::string> first_info = split(input[4], ':');
        std::vector<std::string> second_info = split(input[6], ':');
        if (first_info.size() != 2 or second_info.size() != 2) {
            analyzer->logger->LogWarning("incorrect format ip: " + input[4] + " or " + input[6] + " => skip");
            return nullptr;
        }

        std::string type_log = input[3];
        in_addr_t first_addr = string_to_api(first_info[0]);
        in_port_t first_port = std::stoul(first_info[1]);

        in_addr_t second_addr = string_to_api(second_info[0]);
        in_port_t second_port = std::stoul(second_info[1]);

        analyzer->logger->LogDebug(std::to_string(first_addr) + " " + std::to_string(first_port) + +" " +
                                   std::to_string(second_addr) + " " + std::to_string(second_port));

        if (type_log == "CONNECT") {
            connect(analyzer, second_addr, second_port, first_addr, first_port);
            connect(analyzer, first_addr, first_port, second_addr, second_port);
        } else if (type_log == "GET") {
            std::string tmp(input[7].begin() + 1, input[7].end() - 1);
            size_t size_information = std::stoul(tmp);
            get(analyzer, first_addr, first_port, second_addr, second_port, size_information);
        } else if (type_log == "POST") {
            std::string tmp(input[7].begin() + 1, input[7].end() - 1);
            size_t size_information = std::stoul(tmp);
            post(analyzer, second_addr, second_port, first_addr, first_port, size_information);
        } else if (type_log == "DISCONNECT") {
            disconnect(analyzer, second_addr, second_port, first_addr, first_port);
        }
    }
    for(auto & el : analyzer->info) {
        for(auto & c : el.second.connected) {
            delete[]c.second.pkgs;
        }
    }
    return nullptr;
}

void LogsAnalyzer::connect(LogsAnalyzer *analyzer, in_addr_t dst_addr, in_port_t dst_port, in_addr_t src_addr,
                           in_port_t src_port) {
    auto &dst_info = analyzer->info[dst_addr];
    auto it = dst_info.disconnected.begin();
    for (; it != dst_info.disconnected.end(); ++it) {
        if (*it == src_addr) {
            dst_info.disconnected.erase(it);
            break;
        }
    }
    if (dst_info.connected.find(src_addr) == dst_info.connected.end()) {
        dst_info.connected[src_addr] = {src_addr, new tcp_traffic_pkg[1]{{src_port, dst_addr, dst_port, 0}}, 1};
        return;
    }
}

void LogsAnalyzer::get(LogsAnalyzer *analyzer, in_addr_t dst_addr, in_port_t dst_port, in_addr_t src_addr,
                       in_port_t src_port, size_t size_information) {
    connect(analyzer, dst_addr, dst_port, src_addr, src_port);
    connect(analyzer, src_addr, src_port, dst_addr, dst_port);
    analyzer->info[dst_addr].total_received += size_information;

    auto &conn = analyzer->info[dst_addr].connected[src_addr];

    auto *new_pkgs = new tcp_traffic_pkg[conn.pkgs_sz + 1];
    std::copy(conn.pkgs, conn.pkgs + conn.pkgs_sz, new_pkgs);

    new_pkgs[conn.pkgs_sz] = {src_port, dst_addr, dst_port, size_information};

    delete[] conn.pkgs;

    conn.pkgs = new_pkgs;
    conn.pkgs_sz += 1;

    analyzer->info[src_addr].total_sent += size_information;
}

void LogsAnalyzer::post(LogsAnalyzer *analyzer, in_addr_t dst_addr, in_port_t dst_port, in_addr_t src_addr,
                        in_port_t src_port, size_t size_information) {
    get(analyzer, dst_addr, dst_port, src_addr, src_port, size_information);
}

void LogsAnalyzer::disconnect(LogsAnalyzer *analyzer, in_addr_t dst_addr, [[maybe_unused]] in_port_t dst_port,
                              in_addr_t src_addr, [[maybe_unused]] in_port_t src_port) {
    analyzer->info[dst_addr].disconnected.push_back(src_addr);
    analyzer->info[src_addr].disconnected.push_back(dst_addr);
    if (analyzer->info.find(dst_addr) == analyzer->info.end() or
        analyzer->info[dst_addr].connected.find(src_addr) == analyzer->info[dst_addr].connected.end()) {
        return;
    }
    delete[] analyzer->info[dst_addr].connected[src_addr].pkgs;
    delete[] analyzer->info[src_addr].connected[dst_addr].pkgs;
    analyzer->info[dst_addr].connected.erase(src_addr);
    analyzer->info[src_addr].connected.erase(dst_addr);
}

std::map<in_addr_t, Stats> LogsAnalyzer::get_info() {
    logger->LogWarning("get_info is not safe thread functions. Use with coverage.");
    return info;
}

std::map<in_addr_t, Stats> LogsAnalyzer::get_info(const in_addr_t ip) {
    logger->LogWarning("get_info is not safe thread functions. Use with coverage.");
    return {{ip, info[ip]}};
}

LogsAnalyzer::~LogsAnalyzer() { delete logger; }

LogsAnalyzer::LogsAnalyzer(LogsAnalyzer &&other) noexcept
        : queue{other.queue}, logger{std::exchange(other.logger, nullptr)}, stop(std::exchange(other.stop, false)) {}

LogsAnalyzer &LogsAnalyzer::operator=(LogsAnalyzer &&other) noexcept {
    if (this != &other) {
        delete logger;
        logger = std::exchange(other.logger, nullptr);
        stop = std::exchange(other.stop, false);
    }
    return *this;
}