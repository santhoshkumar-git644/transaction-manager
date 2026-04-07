#pragma once

#include "../utils/enums.h"
#include "../utils/constants.h"

#include <string>

// A single Write-Ahead Log (WAL) record.
//
// Every modification is journalled *before* it is applied to the DataStore so
// that recovery can UNDO an aborted transaction and REDO a committed one.
struct LogRecord {
    int         lsn;       // Log Sequence Number (monotonically increasing)
    int         txnId;     // Transaction that generated this record
    LogType     type;      // BEGIN | WRITE | COMMIT | ABORT

    // Only meaningful for WRITE records:
    std::string key;
    std::string oldValue;  // Value before the write (used for UNDO)
    std::string newValue;  // Value after  the write (used for REDO)

    int prevLsn;           // LSN of the previous record for this transaction
                           // (Constants::NO_PREV_LSN for the first record)

    LogRecord() = default;

    LogRecord(int lsn, int txnId, LogType type,
              const std::string& key      = "",
              const std::string& oldValue = "",
              const std::string& newValue = "",
              int prevLsn                 = Constants::NO_PREV_LSN)
        : lsn(lsn), txnId(txnId), type(type),
          key(key), oldValue(oldValue), newValue(newValue),
          prevLsn(prevLsn) {}
};
