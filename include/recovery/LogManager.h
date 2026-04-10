#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <fstream>
#include <vector>
#include <memory>
#include "LogRecord.h"

class LogManager {
public:
    LogManager(const std::string& log_file_path);
    ~LogManager();
    
    void writeLog(const std::shared_ptr<LogRecord>& record);
    std::vector<std::shared_ptr<LogRecord>> readLogs();
    void flushLogs();
    void clearLogs();
    
    // TODO: Implement checkpointing
    
private:
    std::ofstream log_file_;
    std::vector<std::shared_ptr<LogRecord>> log_buffer_;
};

#endif // LOG_MANAGER_H
