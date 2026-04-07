#pragma once

#include "Transaction.h"
#include "../concurrency/LockManager.h"
#include "../recovery/LogManager.h"
#include "../recovery/RecoveryManager.h"
#include "../storage/DataStore.h"
#include "../utils/enums.h"

#include <memory>
#include <string>
#include <unordered_map>

// Central coordinator that ties together the Lock Manager, Log Manager,
// Recovery Manager and DataStore to provide the standard transaction API.
//
// Typical usage:
//   auto tm = TransactionManager{};
//   int t1  = tm.beginTransaction();
//   tm.write(t1, "X", "42");
//   tm.commit(t1);
class TransactionManager {
public:
    TransactionManager();

    // -----------------------------------------------------------------------
    // Transaction control
    // -----------------------------------------------------------------------

    // Create a new transaction; returns its ID
    int beginTransaction();

    // -----------------------------------------------------------------------
    // Data operations (enforce strict 2PL + WAL)
    // -----------------------------------------------------------------------

    // Acquire a shared lock and read the value of 'key'.
    // outValue is populated only when OperationResult::SUCCESS is returned.
    OperationResult read(int txnId, const std::string& key, std::string& outValue);

    // Acquire an exclusive lock, log the before+after images (WAL), then
    // apply the write to the DataStore.
    // Returns WAITING if another transaction currently holds a conflicting lock.
    // Returns ABORTED  if txnId has already been aborted.
    OperationResult write(int txnId, const std::string& key, const std::string& value);

    // Flush the commit log record and release all locks.
    OperationResult commit(int txnId);

    // Undo all writes (UNDO recovery), log ABORT, then release all locks.
    OperationResult abort(int txnId);

    // -----------------------------------------------------------------------
    // Deadlock handling
    // -----------------------------------------------------------------------

    // Inspect the Wait-For Graph for a cycle.  If one is found, abort the
    // victim transaction and return its ID.  Returns -1 if no deadlock exists.
    int detectAndResolveDeadlock();

    // -----------------------------------------------------------------------
    // Accessors (useful for tests / diagnostics)
    // -----------------------------------------------------------------------
    Transaction*  getTransaction(int txnId);
    DataStore&    getDataStore();
    LogManager&   getLogManager();
    LockManager&  getLockManager();

private:
    std::unordered_map<int, std::unique_ptr<Transaction>> transactions_;
    LockManager      lockManager_;
    LogManager       logManager_;
    DataStore        dataStore_;
    RecoveryManager  recoveryManager_;
    int              nextTxnId_ = 1;

    bool isValidActive(int txnId) const;
};
