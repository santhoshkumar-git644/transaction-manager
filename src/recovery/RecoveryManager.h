#pragma once

#include "LogManager.h"
#include "../storage/DataStore.h"

// Implements crash-recovery operations using the Write-Ahead Log.
//
//  UNDO  – rolls back an aborted transaction by replaying its log records in
//          *reverse* order and restoring each key to its before-image.
//
//  REDO  – re-applies a committed transaction by replaying its log records in
//          *forward* order and writing each key's after-image.
//
//  recover() – a simplified crash-recovery sweep: REDO committed transactions
//              then UNDO any transaction that was neither committed nor explicitly
//              aborted (i.e. transactions that were in-flight at the time of
//              the simulated crash).
class RecoveryManager {
public:
    RecoveryManager(LogManager& logManager, DataStore& dataStore);

    // Rollback: undo all writes made by txnId (used on explicit ABORT)
    void undo(int txnId);

    // Redo all writes made by txnId (useful for crash-recovery replay)
    void redo(int txnId);

    // Full crash recovery: redo all committed transactions, then undo
    // all transactions that were neither committed nor aborted.
    void recover();

private:
    LogManager& logManager_;
    DataStore&  dataStore_;
};
