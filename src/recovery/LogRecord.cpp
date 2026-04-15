#include "../../include/recovery/LogRecord.h"

#include <sstream>

LogRecord::LogRecord(uint32_t transaction_id, LogType type)
    : transaction_id_(transaction_id), type_(type), timestamp_(time(nullptr)) {
}

LogRecord::LogRecord(uint32_t transaction_id, LogType type, time_t timestamp)
    : transaction_id_(transaction_id), type_(type), timestamp_(timestamp) {
}

LogRecord::~LogRecord() {
}

uint32_t LogRecord::getTransactionId() const {
    return transaction_id_;
}

LogRecord::LogType LogRecord::getLogType() const {
    return type_;
}

time_t LogRecord::getTimestamp() const {
    return timestamp_;
}

std::string LogRecord::toString() const {
    std::ostringstream out;
    out << "timestamp=" << timestamp_
        << ",txn=" << transaction_id_
        << ",type=" << logTypeToString(type_);
    return out.str();
}

std::shared_ptr<LogRecord> LogRecord::fromString(const std::string& serialized) {
    std::stringstream ss(serialized);
    std::string segment;
    time_t timestamp = 0;
    uint32_t txn_id = 0;
    LogType log_type = LogType::BEGIN;
    bool has_timestamp = false;
    bool has_txn = false;
    bool has_type = false;

    while (std::getline(ss, segment, ',')) {
        const size_t pos = segment.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        const std::string key = segment.substr(0, pos);
        const std::string value = segment.substr(pos + 1);

        if (key == "timestamp") {
            timestamp = static_cast<time_t>(std::stoll(value));
            has_timestamp = true;
        } else if (key == "txn") {
            txn_id = static_cast<uint32_t>(std::stoul(value));
            has_txn = true;
        } else if (key == "type") {
            has_type = stringToLogType(value, log_type);
        }
    }

    if (!has_timestamp || !has_txn || !has_type) {
        return nullptr;
    }

    return std::make_shared<LogRecord>(txn_id, log_type, timestamp);
}

std::string LogRecord::logTypeToString(LogType type) {
    switch (type) {
        case LogType::BEGIN:
            return "BEGIN";
        case LogType::INSERT:
            return "INSERT";
        case LogType::UPDATE:
            return "UPDATE";
        case LogType::DELETE:
            return "DELETE";
        case LogType::COMMIT:
            return "COMMIT";
        case LogType::ABORT:
            return "ABORT";
        case LogType::CHECKPOINT:
            return "CHECKPOINT";
        default:
            return "BEGIN";
    }
}

bool LogRecord::stringToLogType(const std::string& value, LogType& out_type) {
    if (value == "BEGIN") {
        out_type = LogType::BEGIN;
        return true;
    }
    if (value == "INSERT") {
        out_type = LogType::INSERT;
        return true;
    }
    if (value == "UPDATE") {
        out_type = LogType::UPDATE;
        return true;
    }
    if (value == "DELETE") {
        out_type = LogType::DELETE;
        return true;
    }
    if (value == "COMMIT") {
        out_type = LogType::COMMIT;
        return true;
    }
    if (value == "ABORT") {
        out_type = LogType::ABORT;
        return true;
    }
    if (value == "CHECKPOINT") {
        out_type = LogType::CHECKPOINT;
        return true;
    }

    return false;
}
