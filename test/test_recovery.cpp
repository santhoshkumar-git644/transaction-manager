#include <cassert>
#include <iostream>
#include <set>
#include "../include/recovery/LogManager.h"
#include "../include/recovery/RecoveryManager.h"

int main() {
    std::cout << "Testing Recovery Management\n";
    
    auto log_manager = std::make_shared<LogManager>("test_recovery.log");
    log_manager->clearLogs();

    // Txn 1: committed and should be replayed during redo
    log_manager->writeLog(std::make_shared<LogRecord>(1, LogRecord::LogType::BEGIN));
    log_manager->writeLog(std::make_shared<LogRecord>(1, LogRecord::LogType::INSERT));
    log_manager->writeLog(std::make_shared<LogRecord>(1, LogRecord::LogType::COMMIT));

    // Txn 2: incomplete and should be aborted during undo
    log_manager->writeLog(std::make_shared<LogRecord>(2, LogRecord::LogType::BEGIN));
    log_manager->writeLog(std::make_shared<LogRecord>(2, LogRecord::LogType::UPDATE));
    log_manager->flushLogs();

    auto before_recovery = log_manager->readLogs();
    assert(before_recovery.size() == 5);

    RecoveryManager recovery_mgr(log_manager);

    recovery_mgr.performRecovery();

    assert(recovery_mgr.getRecoveryState() == RecoveryState::COMPLETE);

    const auto committed = recovery_mgr.getCommittedTransactionIds();
    assert(committed.size() == 1);
    assert(committed[0] == 1);

    const auto incomplete = recovery_mgr.getIncompleteTransactionIds();
    assert(incomplete.size() == 1);
    assert(incomplete[0] == 2);

    const auto redo_records = recovery_mgr.getRedoRecords();
    assert(redo_records.size() == 1);
    assert(redo_records[0]->getTransactionId() == 1);
    assert(redo_records[0]->getLogType() == LogRecord::LogType::INSERT);

    const auto undo_records = recovery_mgr.getUndoRecords();
    assert(undo_records.size() == 1);
    assert(undo_records[0]->getTransactionId() == 2);
    assert(undo_records[0]->getLogType() == LogRecord::LogType::ABORT);

    auto after_recovery = log_manager->readLogs();
    assert(after_recovery.size() == 6);
    assert(after_recovery.back()->getTransactionId() == 2);
    assert(after_recovery.back()->getLogType() == LogRecord::LogType::ABORT);

    std::cout << "Recovery analysis/redo/undo checks passed" << std::endl;
    
    return 0;
}
