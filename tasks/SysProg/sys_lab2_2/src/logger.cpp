#include "../include/logger.h"

#include <utility>

Logger::Builder &Logger::Builder::setName(const std::string &name) {
    name_ = name;
    return *this;
}

Logger::Builder &Logger::Builder::setLevel(Logger::LevelLogger level) {
    level_ = level;
    return *this;
}

Logger::Builder &Logger::Builder::addHandler(std::unique_ptr<LogHandler> handler) {
    handlers_.push_back(std::move(handler));
    return *this;
}

Logger *Logger::Builder::build() {
    auto *logger = new Logger(name_, level_);

    for (auto &handler : handlers_) {
        logger->addHandler(std::move(handler));
    }
    return logger;
}

void Logger::Log(Logger::LevelLogger level, const std::string &log) {
    std::lock_guard<std::mutex> lock(mutex);
    if (level > log_level) return;
    std::string result = "[";
    std::string time = get_datetime();
    result += time + "][" + logger_name + "][";
    switch (level) {
        case LOG_CRITICAL:
            result += "CRITICAL]";
            break;
        case LOG_ERROR:
            result += "ERROR]";
            break;
        case LOG_WARNING:
            result += "WARNING]";
            break;
        case LOG_INFO:
            result += "INFO]";
            break;
        case LOG_DEBUG:
            result += "DEBUG]";
            break;
    }
    result += " " + log;
    for (auto &out : handlers) {
        out->write(result);
    }
}

std::string Logger::get_datetime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %X");
    return ss.str();
}

void Logger::LogError(const std::string &log) { Log(LOG_ERROR, log); }

void Logger::LogCritic(const std::string &log) { Log(LOG_CRITICAL, log); }

void Logger::LogWarning(const std::string &log) { Log(LOG_WARNING, log); }

void Logger::LogInfo(const std::string &log) { Log(LOG_INFO, log); }

void Logger::LogDebug(const std::string &log) { Log(LOG_DEBUG, log); }

void Logger::close_logger() { handlers.clear(); }

Logger &Logger::addHandler(std::unique_ptr<LogHandler> handler) {

    std::lock_guard<std::mutex> lock(mutex);
    handlers.push_back(std::move(handler));
    return *this;
}

StreamLoggerHandler::StreamLoggerHandler(std::ostream &stream) : stream_(stream) {}

void StreamLoggerHandler::write(const std::string &message) {
    std::lock_guard<std::mutex> lock(mutex);
    stream_ << message << std::endl;
}

FileLoggerHandler::FileLoggerHandler(const std::string &filename) { out.open(filename); }

void FileLoggerHandler::write(const std::string &message) {
    std::lock_guard<std::mutex> lock(mutex);
    out << message << std::endl;
}

FileLoggerHandler::~FileLoggerHandler() {
    if (out.is_open()) out.close();
}