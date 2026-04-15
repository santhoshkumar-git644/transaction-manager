#include "../../include/recovery/LogManager.h"

#include <fstream>
#include <string>

LogManager::LogManager(const std::string& log_file_path) {
    log_file_path_ = log_file_path;
    log_file_.open(log_file_path_, std::ios::app);
}

LogManager::~LogManager() {
    flushLogs();
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void LogManager::writeLog(const std::shared_ptr<LogRecord>& record) {
    if (!record) {
        return;
    }

    log_buffer_.push_back(record);

    if (!log_file_.is_open()) {
        log_file_.open(log_file_path_, std::ios::app);
    }

    if (log_file_.is_open()) {
        log_file_ << record->toString() << '\n';
    }
}

std::vector<std::shared_ptr<LogRecord>> LogManager::readLogs() {
    std::vector<std::shared_ptr<LogRecord>> records;
    std::ifstream input(log_file_path_);
    if (!input.is_open()) {
        return records;
    }

    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        std::shared_ptr<LogRecord> record = LogRecord::fromString(line);
        if (record != nullptr) {
            records.push_back(record);
        }
    }

    return records;
}

void LogManager::flushLogs() {
    if (log_file_.is_open()) {
        log_file_.flush();
    }
}

void LogManager::clearLogs() {
    log_buffer_.clear();

    if (log_file_.is_open()) {
        log_file_.close();
    }

    std::ofstream truncate_file(log_file_path_, std::ios::trunc);
    truncate_file.close();

    log_file_.open(log_file_path_, std::ios::app);
}
