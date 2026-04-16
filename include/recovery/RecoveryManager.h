#ifndef RECOVERY_MANAGER_H
#define RECOVERY_MANAGER_H

#include <memory>
#include <set>
#include <vector>
#include "LogManager.h"
#include "../utils/enums.h"

class RecoveryManager {
public:
    RecoveryManager(std::shared_ptr<LogManager> log_manager);
    ~RecoveryManager();
    
    void performRecovery();
    void redo();
    void undo();
    
    RecoveryState getRecoveryState() const;
    std::vector<uint32_t> getCommittedTransactionIds() const;
    std::vector<uint32_t> getIncompleteTransactionIds() const;
    std::vector<std::shared_ptr<LogRecord>> getRedoRecords() const;
    std::vector<std::shared_ptr<LogRecord>> getUndoRecords() const;
    
private:
    void analyze();
    bool isDataModificationType(LogRecord::LogType type) const;

    std::shared_ptr<LogManager> log_manager_;
    RecoveryState recovery_state_;
    std::set<uint32_t> committed_txns_;
    std::set<uint32_t> incomplete_txns_;
    std::vector<std::shared_ptr<LogRecord>> redo_records_;
    std::vector<std::shared_ptr<LogRecord>> undo_records_;
};

#endif // RECOVERY_MANAGER_H
