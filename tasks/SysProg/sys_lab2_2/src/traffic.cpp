
#include "../include/traffic.h"

Traffic::Traffic(size_t count_generate_threads, size_t count_analyze_threads)
        : count_analyze_threads(count_analyze_threads), count_generate_threads(count_generate_threads) {
    for (int i = 0; i < count_generate_threads; ++i) {
        generators.emplace_back(q, "Generator " + std::to_string(i + 1), Logger::LOG_DEBUG);
    }
    for (int i = 0; i < count_analyze_threads; ++i) {
        analyzers.emplace_back(q);
    }
}

void *Traffic::start_traffic(void *arg) {
    auto *traffic = static_cast<Traffic *>(arg);
    if (arg == nullptr) {
        return nullptr;
    }
    pthread_t threads[traffic->count_generate_threads];
    for (int i = 0; i < traffic->count_generate_threads; ++i) {
        pthread_create(&threads[i], nullptr, &LogsGenerator::generate_sample_traffic, &traffic->generators[i]);
    }

    for (int i = 0; i < traffic->count_generate_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    return nullptr;
}

void *Traffic::analyze_traffic(void *arg) {
    auto *traffic = static_cast<Traffic *>(arg);
    if (arg == nullptr) {
        return nullptr;
    }
    pthread_t threads[traffic->count_analyze_threads + 1];
    pthread_create(&threads[traffic->count_analyze_threads], nullptr, &dialog, traffic);
    for (int i = 0; i < traffic->count_analyze_threads; ++i) {
        pthread_create(&threads[i], nullptr, &LogsAnalyzer::analyzing_traffic, &traffic->analyzers[i]);
    }

    for (int i = 0; i < traffic->count_analyze_threads + 1; ++i) {
        pthread_join(threads[i], nullptr);
    }
    return nullptr;
}

std::map<in_addr_t, Stats> Traffic::get_info() {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<in_addr_t, Stats> result;

    for (auto &analyzer : analyzers) {
        for (auto &pair : analyzer.get_info()) {
            in_addr_t ip = pair.first;
            Stats &src_stats = pair.second;
            Stats &dst_stats = result[ip];

            dst_stats.total_sent += src_stats.total_sent;
            dst_stats.total_received += src_stats.total_received;
            dst_stats.disconnected.insert(dst_stats.disconnected.end(), src_stats.disconnected.begin(),
                                          src_stats.disconnected.end());

            for (auto &src_conn : src_stats.connected) {
                in_addr_t src_ip = src_conn.first;
                auto &src_info = src_conn.second;
                auto &dst_info = dst_stats.connected[src_ip];

                auto *new_pkgs = new tcp_traffic_pkg[src_info.pkgs_sz + dst_info.pkgs_sz];
                std::copy(dst_info.pkgs, dst_info.pkgs + dst_info.pkgs_sz, new_pkgs);
                std::copy(src_info.pkgs, src_info.pkgs + src_info.pkgs_sz, new_pkgs + dst_info.pkgs_sz);

                if (dst_info.pkgs_sz > 0) {
                    delete[] dst_info.pkgs;
                }

                dst_info.src_addr = src_info.src_addr;
                dst_info.pkgs = new_pkgs;
                dst_info.pkgs_sz = src_info.pkgs_sz + dst_info.pkgs_sz;
            }
        }
    }

    for (auto &conn : result) {
        for (auto dis : conn.second.disconnected) {
            if (conn.second.connected.find(dis) != conn.second.connected.end()) {
                delete[] conn.second.connected[dis].pkgs;
                conn.second.connected.erase(dis);
            }
        }
    }
    return result;
}

void Traffic::print_and_delete_info(const std::map<in_addr_t, Stats> &info) {
    for (auto &el : info) {
        print_and_delete_ip(el);
    }
}

void Traffic::print_and_delete_ip(const std::pair<in_addr_t, Stats> &el) {
    std::cout << "------------------\n";
    std::cout << "IP: " << LogsGenerator::ip_to_string(el.first) << std::endl;
    std::cout << "Total sent: " << el.second.total_sent << std::endl;
    std::cout << "Total received: " << el.second.total_received << std::endl;
    std::cout << "Connected ip:\n";
    for (auto &addr : el.second.connected) {
        std::cout << "\t" << LogsGenerator::ip_to_string(addr.second.src_addr) << std::endl;
        std::cout << "\tPackets dst:src\n";
        for (int i = 0; i < addr.second.pkgs_sz; ++i) {
            std::cout << "\t\t" << std::to_string(addr.second.pkgs[i].dst_port) << ":"
                      << std::to_string(addr.second.pkgs[i].src_port) << " " << std::to_string(addr.second.pkgs[i].sz)
                      << " B" << std::endl;
        }
        if (addr.second.pkgs_sz > 0) {
            delete[] addr.second.pkgs;
        }
    }
}

void *Traffic::dialog(void *arg) {
    auto *a = static_cast<Traffic *>(arg);
    if (a == nullptr) {
        return nullptr;
    }
    std::string line;
    std::cout << "Write: \n1. get all\n2. get <addr>\n3. stop\n";
    while (getline(std::cin, line)) {
        if (line == "stop") {
            for (int i = 0; i < a->count_generate_threads; ++i) {
                a->generators[i].stop = true;
            }
            for (int i = 0; i < a->count_analyze_threads; ++i) {
                a->analyzers[i].stop = true;
            }
            break;
        } else {
            if (line == "get all") {
                Traffic::print_and_delete_info(a->get_info());
            } else {
                auto vec = LogsAnalyzer::split(line, ' ');
                if (vec.size() == 2 and vec[0] == "get") {
                    in_addr_t t = LogsAnalyzer::string_to_api(vec[1]);
                    Traffic::print_and_delete_info(a->get_info(t));

                } else {
                    std::cout << "Incorrect command\n";
                }
            }
        }
        std::cout << "Write: \n1. get all\n2. get <addr>\n3. stop\n";
    }
    return nullptr;
}

std::map<in_addr_t, Stats> Traffic::get_info(in_addr_t addr) {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<in_addr_t, Stats> result;

    for (auto &analyzer : analyzers) {
        for (auto &pair : analyzer.get_info(addr)) {
            in_addr_t ip = pair.first;
            Stats &src_stats = pair.second;
            Stats &dst_stats = result[ip];

            dst_stats.total_sent += src_stats.total_sent;
            dst_stats.total_received += src_stats.total_received;
            dst_stats.disconnected.insert(dst_stats.disconnected.end(), src_stats.disconnected.begin(),
                                          src_stats.disconnected.end());

            for (auto &src_conn : src_stats.connected) {
                in_addr_t src_ip = src_conn.first;
                auto &src_info = src_conn.second;
                auto &dst_info = dst_stats.connected[src_ip];

                auto *new_pkgs = new tcp_traffic_pkg[src_info.pkgs_sz + dst_info.pkgs_sz];
                std::copy(dst_info.pkgs, dst_info.pkgs + dst_info.pkgs_sz, new_pkgs);
                std::copy(src_info.pkgs, src_info.pkgs + src_info.pkgs_sz, new_pkgs + dst_info.pkgs_sz);

                if (dst_info.pkgs_sz > 0) {
                    delete[] dst_info.pkgs;
                }

                dst_info.src_addr = src_info.src_addr;
                dst_info.pkgs = new_pkgs;
                dst_info.pkgs_sz = src_info.pkgs_sz + dst_info.pkgs_sz;
            }
        }
    }

    for (auto &conn : result) {
        for (auto dis : conn.second.disconnected) {
            if (conn.second.connected.find(dis) != conn.second.connected.end()) {
                delete[] conn.second.connected[dis].pkgs;
                conn.second.connected.erase(dis);
            }
        }
    }
    return result;
}
