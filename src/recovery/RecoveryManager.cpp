#include "RecoveryManager.h"

#include <algorithm>
#include <iostream>
#include <unordered_set>

RecoveryManager::RecoveryManager(LogManager& logManager, DataStore& dataStore)
    : logManager_(logManager), dataStore_(dataStore) {}

// ---------------------------------------------------------------------------
// UNDO – restore before-images in reverse log order
// ---------------------------------------------------------------------------

void RecoveryManager::undo(int txnId) {
    auto records = logManager_.getLogsForTxn(txnId);

    // Process WRITE records in reverse order (newest first)
    for (int i = static_cast<int>(records.size()) - 1; i >= 0; --i) {
        const auto& rec = records[i];
        if (rec.type != LogType::WRITE) {
            continue;
        }

        if (rec.oldValue.empty()) {
            // The key did not exist before this write: remove it
            dataStore_.remove(rec.key);
        } else {
            dataStore_.write(rec.key, rec.oldValue);
        }
    }
}

// ---------------------------------------------------------------------------
// REDO – apply after-images in forward log order
// ---------------------------------------------------------------------------

void RecoveryManager::redo(int txnId) {
    auto records = logManager_.getLogsForTxn(txnId);

    for (const auto& rec : records) {
        if (rec.type != LogType::WRITE) {
            continue;
        }
        dataStore_.write(rec.key, rec.newValue);
    }
}

// ---------------------------------------------------------------------------
// Full crash recovery (simplified ARIES-style)
// ---------------------------------------------------------------------------

void RecoveryManager::recover() {
    const auto& log = logManager_.getLog();

    // Classify each transaction that appears in the log
    std::unordered_set<int> committed;
    std::unordered_set<int> aborted;
    std::unordered_set<int> seen;

    for (const auto& rec : log) {
        seen.insert(rec.txnId);
        if (rec.type == LogType::COMMIT) {
            committed.insert(rec.txnId);
        } else if (rec.type == LogType::ABORT) {
            aborted.insert(rec.txnId);
        }
    }

    // REDO phase: re-apply all writes of committed transactions
    std::cout << "Recovery: REDO phase\n";
    for (int txnId : committed) {
        redo(txnId);
    }

    // UNDO phase: roll back transactions that are neither committed nor aborted
    std::cout << "Recovery: UNDO phase\n";
    for (int txnId : seen) {
        if (committed.count(txnId) == 0 && aborted.count(txnId) == 0) {
            std::cout << "  Undoing incomplete transaction T" << txnId << "\n";
            undo(txnId);
        }
    }
}
