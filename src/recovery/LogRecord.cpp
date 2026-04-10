#include "../../include/recovery/LogRecord.h"

LogRecord::LogRecord(uint32_t transaction_id, LogType type)
    : transaction_id_(transaction_id), type_(type), timestamp_(time(nullptr)) {
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
