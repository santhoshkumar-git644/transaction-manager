#include "../../include/recovery/RecoveryManager.h"

#include <algorithm>

RecoveryManager::RecoveryManager(std::shared_ptr<LogManager> log_manager)
    : log_manager_(log_manager), recovery_state_(RecoveryState::ANALYSIS) {
}

RecoveryManager::~RecoveryManager() {
}

void RecoveryManager::performRecovery() {
    recovery_state_ = RecoveryState::ANALYSIS;
    analyze();
    
    recovery_state_ = RecoveryState::REDO;
    redo();
    
    recovery_state_ = RecoveryState::UNDO;
    undo();
    
    recovery_state_ = RecoveryState::COMPLETE;
}

void RecoveryManager::redo() {
    redo_records_.clear();

    const auto logs = log_manager_->readLogs();
    for (const auto& record : logs) {
        if (record == nullptr) {
            continue;
        }

        if (committed_txns_.find(record->getTransactionId()) == committed_txns_.end()) {
            continue;
        }

        if (isDataModificationType(record->getLogType())) {
            redo_records_.push_back(record);
        }
    }
}

void RecoveryManager::undo() {
    undo_records_.clear();

    std::vector<uint32_t> txn_ids(incomplete_txns_.begin(), incomplete_txns_.end());
    std::sort(txn_ids.begin(), txn_ids.end());

    for (uint32_t txn_id : txn_ids) {
        auto abort_record = std::make_shared<LogRecord>(txn_id, LogRecord::LogType::ABORT);
        log_manager_->writeLog(abort_record);
        undo_records_.push_back(abort_record);
    }

    if (!txn_ids.empty()) {
        log_manager_->flushLogs();
    }
}

RecoveryState RecoveryManager::getRecoveryState() const {
    return recovery_state_;
}

std::vector<uint32_t> RecoveryManager::getCommittedTransactionIds() const {
    return std::vector<uint32_t>(committed_txns_.begin(), committed_txns_.end());
}

std::vector<uint32_t> RecoveryManager::getIncompleteTransactionIds() const {
    return std::vector<uint32_t>(incomplete_txns_.begin(), incomplete_txns_.end());
}

std::vector<std::shared_ptr<LogRecord>> RecoveryManager::getRedoRecords() const {
    return redo_records_;
}

std::vector<std::shared_ptr<LogRecord>> RecoveryManager::getUndoRecords() const {
    return undo_records_;
}

void RecoveryManager::analyze() {
    committed_txns_.clear();
    incomplete_txns_.clear();

    std::set<uint32_t> active_txns;
    const auto logs = log_manager_->readLogs();

    for (const auto& record : logs) {
        if (record == nullptr) {
            continue;
        }

        const uint32_t txn_id = record->getTransactionId();
        switch (record->getLogType()) {
            case LogRecord::LogType::BEGIN:
                active_txns.insert(txn_id);
                break;
            case LogRecord::LogType::COMMIT:
                committed_txns_.insert(txn_id);
                active_txns.erase(txn_id);
                break;
            case LogRecord::LogType::ABORT:
                active_txns.erase(txn_id);
                break;
            default:
                break;
        }
    }

    incomplete_txns_ = active_txns;
}

bool RecoveryManager::isDataModificationType(LogRecord::LogType type) const {
    return type == LogRecord::LogType::INSERT ||
           type == LogRecord::LogType::UPDATE ||
           type == LogRecord::LogType::DELETE;
}
