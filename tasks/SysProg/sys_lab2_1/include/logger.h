#pragma once

#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <fstream>
#include <memory>
#include <utility>

class LogHandler {
public:
    virtual void write(const std::string &message) = 0;
    virtual ~LogHandler() = default;
};

class StreamLoggerHandler : public LogHandler {
protected:
    std::ostream &stream_;
public:
    explicit StreamLoggerHandler(std::ostream &stream = std::cout);
    void write(const std::string &message) override;
};

class FileLoggerHandler : public LogHandler {
    std::ofstream out;
public:
    explicit FileLoggerHandler(const std::string& filename);
    void write(const std::string& message) override;
    ~FileLoggerHandler() override;
};

class Logger {
public:
    enum LevelLogger {
        LOG_CRITICAL,
        LOG_ERROR,
        LOG_WARNING,
        LOG_INFO,
        LOG_DEBUG
    };

    class Builder {
        std::string name_;
        LevelLogger level_ = LOG_DEBUG;
        std::vector<std::unique_ptr<LogHandler>> handlers_;
    public:
        Builder& setName(const std::string& name);
        Builder& setLevel(LevelLogger level);
        Builder& addHandler(std::unique_ptr<LogHandler> handler);
        Logger* build();
    };

private:
    std::string logger_name;
    std::vector<std::unique_ptr<LogHandler>> handlers;
    LevelLogger log_level;

    static std::string get_datetime();
    explicit Logger(std::string logger_n, LevelLogger logger_level = LOG_DEBUG)
            : logger_name(std::move(logger_n)), log_level(logger_level) {}

    Logger& addHandler(std::unique_ptr<LogHandler> handler);
public:

    void Log(LevelLogger level, const std::string &log);

    void LogError(const std::string &log);
    void LogCritic(const std::string &log);
    void LogWarning(const std::string &log);
    void LogInfo(const std::string &log);
    void LogDebug(const std::string &log);

    void close_logger();

};