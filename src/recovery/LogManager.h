#pragma once

#include "LogRecord.h"
#include "../utils/enums.h"

#include <string>
#include <unordered_map>
#include <vector>

// Append-only Write-Ahead Log (WAL).
//
// Callers must append a log record and *flush* it (force-to-disk in a real
// system) before applying the corresponding change to the DataStore.
// In this simulation the log lives entirely in memory.
class LogManager {
public:
    // Append a new record for 'txnId' of the given type.
    // For WRITE records supply key / oldValue / newValue.
    // Returns the newly assigned LSN.
    int appendLog(int txnId, LogType type,
                  const std::string& key      = "",
                  const std::string& oldValue = "",
                  const std::string& newValue = "");

    // All log records in append order
    const std::vector<LogRecord>& getLog() const;

    // Log records that belong to a specific transaction (in LSN order)
    std::vector<LogRecord> getLogsForTxn(int txnId) const;

    // LSN of the most recent record appended for txnId, or NO_PREV_LSN
    int getLastLsnForTxn(int txnId) const;

    // Print the full log to stdout (for debugging / demo)
    void printLog() const;

private:
    std::vector<LogRecord>      log_;
    std::unordered_map<int,int> lastLsn_; // txnId -> last LSN
    int                         nextLsn_ = 0;
};
