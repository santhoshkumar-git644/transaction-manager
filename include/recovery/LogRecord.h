#ifndef LOG_RECORD_H
#define LOG_RECORD_H

#include <cstdint>
#include <string>
#include <ctime>

class LogRecord {
public:
    enum class LogType {
        BEGIN,
        INSERT,
        UPDATE,
        DELETE,
        COMMIT,
        ABORT,
        CHECKPOINT
    };
    
    LogRecord(uint32_t transaction_id, LogType type);
    ~LogRecord();
    
    uint32_t getTransactionId() const;
    LogType getLogType() const;
    time_t getTimestamp() const;
    
    // TODO: Add methods for serialization
    
private:
    uint32_t transaction_id_;
    LogType type_;
    time_t timestamp_;
};

#endif // LOG_RECORD_H
