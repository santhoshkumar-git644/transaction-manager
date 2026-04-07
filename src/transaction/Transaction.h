#pragma once

#include "../utils/enums.h"

#include <string>
#include <vector>

// Represents a single database transaction.
//
// A Transaction object tracks:
//   - its unique ID and current state
//   - the keys it has modified (for lock-release bookkeeping)
//   - the LSNs of the log records it has generated (linked-list through prevLsn)
class Transaction {
public:
    explicit Transaction(int txnId);

    // -----------------------------------------------------------------------
    // Identity and state
    // -----------------------------------------------------------------------
    int               getId()    const;
    TransactionState  getState() const;
    void              setState(TransactionState state);

    // Convenience state predicates
    bool isActive()    const;
    bool isBlocked()   const;
    bool isCommitted() const;
    bool isAborted()   const;

    // -----------------------------------------------------------------------
    // Write tracking (used by RecoveryManager to know what to undo)
    // -----------------------------------------------------------------------
    void                          addWrittenKey(const std::string& key);
    const std::vector<std::string>& getWrittenKeys() const;

    // -----------------------------------------------------------------------
    // Log record tracking
    // -----------------------------------------------------------------------
    void                  addLsn(int lsn);
    const std::vector<int>& getLsns() const;

    // Print a summary to stdout
    void print() const;

private:
    int                  txnId_;
    TransactionState     state_;
    std::vector<std::string> writtenKeys_;
    std::vector<int>     lsns_;
};
