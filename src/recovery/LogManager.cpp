#include "LogManager.h"

#include "../utils/constants.h"

#include <iostream>
#include <iomanip>

// ---------------------------------------------------------------------------
// Append
// ---------------------------------------------------------------------------

int LogManager::appendLog(int txnId, LogType type,
                          const std::string& key,
                          const std::string& oldValue,
                          const std::string& newValue) {
    int prevLsn = getLastLsnForTxn(txnId);
    int lsn     = nextLsn_++;

    log_.emplace_back(lsn, txnId, type, key, oldValue, newValue, prevLsn);
    lastLsn_[txnId] = lsn;

    return lsn;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

const std::vector<LogRecord>& LogManager::getLog() const {
    return log_;
}

std::vector<LogRecord> LogManager::getLogsForTxn(int txnId) const {
    std::vector<LogRecord> result;
    for (const auto& rec : log_) {
        if (rec.txnId == txnId) {
            result.push_back(rec);
        }
    }
    return result;
}

int LogManager::getLastLsnForTxn(int txnId) const {
    auto it = lastLsn_.find(txnId);
    if (it != lastLsn_.end()) {
        return it->second;
    }
    return Constants::NO_PREV_LSN;
}

// ---------------------------------------------------------------------------
// Diagnostics
// ---------------------------------------------------------------------------

static const char* logTypeName(LogType t) {
    switch (t) {
        case LogType::BEGIN:  return "BEGIN";
        case LogType::WRITE:  return "WRITE";
        case LogType::COMMIT: return "COMMIT";
        case LogType::ABORT:  return "ABORT";
    }
    return "UNKNOWN";
}

void LogManager::printLog() const {
    std::cout << "WAL Log:\n";
    std::cout << std::left
              << std::setw(6)  << "LSN"
              << std::setw(6)  << "TXN"
              << std::setw(9)  << "TYPE"
              << std::setw(10) << "KEY"
              << std::setw(14) << "OLD_VALUE"
              << "NEW_VALUE\n";
    std::cout << std::string(55, '-') << "\n";

    for (const auto& r : log_) {
        std::cout << std::left
                  << std::setw(6)  << r.lsn
                  << std::setw(6)  << r.txnId
                  << std::setw(9)  << logTypeName(r.type)
                  << std::setw(10) << r.key
                  << std::setw(14) << r.oldValue
                  << r.newValue
                  << "\n";
    }
}
