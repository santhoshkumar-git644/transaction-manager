#ifndef LOG_RECORD_H
#define LOG_RECORD_H

#include <cstdint>
#include <string>
#include <ctime>
#include <memory>

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
    LogRecord(uint32_t transaction_id, LogType type, time_t timestamp);
    ~LogRecord();
    
    uint32_t getTransactionId() const;
    LogType getLogType() const;
    time_t getTimestamp() const;
    
    std::string toString() const;
    static std::shared_ptr<LogRecord> fromString(const std::string& serialized);

    static std::string logTypeToString(LogType type);
    static bool stringToLogType(const std::string& value, LogType& out_type);
    
private:
    uint32_t transaction_id_;
    LogType type_;
    time_t timestamp_;
};

#endif // LOG_RECORD_H
