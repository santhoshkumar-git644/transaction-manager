#include "TransactionManager.h"

#include <iostream>

TransactionManager::TransactionManager()
    : recoveryManager_(logManager_, dataStore_) {}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

bool TransactionManager::isValidActive(int txnId) const {
    auto it = transactions_.find(txnId);
    if (it == transactions_.end()) return false;
    return it->second->isActive();
}

// ---------------------------------------------------------------------------
// Transaction control
// ---------------------------------------------------------------------------

int TransactionManager::beginTransaction() {
    int id = nextTxnId_++;
    transactions_[id] = std::make_unique<Transaction>(id);
    logManager_.appendLog(id, LogType::BEGIN);
    std::cout << "[TM] T" << id << " BEGIN\n";
    return id;
}

// ---------------------------------------------------------------------------
// Read
// ---------------------------------------------------------------------------

OperationResult TransactionManager::read(int txnId,
                                          const std::string& key,
                                          std::string& outValue) {
    auto it = transactions_.find(txnId);
    if (it == transactions_.end()) {
        std::cerr << "[TM] Unknown transaction T" << txnId << "\n";
        return OperationResult::ERROR;
    }
    Transaction* txn = it->second.get();

    if (txn->isAborted()) {
        std::cerr << "[TM] T" << txnId << " is already ABORTED\n";
        return OperationResult::ABORTED;
    }
    if (!txn->isActive()) {
        std::cerr << "[TM] T" << txnId << " is not ACTIVE (state="
                  << static_cast<int>(txn->getState()) << ")\n";
        return OperationResult::ERROR;
    }

    // Acquire shared lock
    LockStatus ls = lockManager_.acquireLock(txnId, key, LockMode::SHARED);
    if (ls == LockStatus::WAITING) {
        txn->setState(TransactionState::BLOCKED);
        std::cout << "[TM] T" << txnId << " READ  " << key
                  << " -> WAITING (lock conflict)\n";
        return OperationResult::WAITING;
    }

    auto val = dataStore_.read(key);
    outValue  = val.value_or("");
    std::cout << "[TM] T" << txnId << " READ  " << key
              << " = \"" << outValue << "\"\n";
    return OperationResult::SUCCESS;
}

// ---------------------------------------------------------------------------
// Write (WAL: log before apply)
// ---------------------------------------------------------------------------

OperationResult TransactionManager::write(int txnId,
                                           const std::string& key,
                                           const std::string& value) {
    auto it = transactions_.find(txnId);
    if (it == transactions_.end()) {
        std::cerr << "[TM] Unknown transaction T" << txnId << "\n";
        return OperationResult::ERROR;
    }
    Transaction* txn = it->second.get();

    if (txn->isAborted()) {
        std::cerr << "[TM] T" << txnId << " is already ABORTED\n";
        return OperationResult::ABORTED;
    }
    if (!txn->isActive()) {
        std::cerr << "[TM] T" << txnId << " is not ACTIVE\n";
        return OperationResult::ERROR;
    }

    // Acquire exclusive lock
    LockStatus ls = lockManager_.acquireLock(txnId, key, LockMode::EXCLUSIVE);
    if (ls == LockStatus::WAITING) {
        txn->setState(TransactionState::BLOCKED);
        std::cout << "[TM] T" << txnId << " WRITE " << key
                  << " = \"" << value << "\" -> WAITING (lock conflict)\n";
        return OperationResult::WAITING;
    }

    // WAL: record old value before applying the write
    std::string oldValue = dataStore_.read(key).value_or("");
    int lsn = logManager_.appendLog(txnId, LogType::WRITE, key, oldValue, value);
    txn->addLsn(lsn);
    txn->addWrittenKey(key);

    // Apply write to DataStore
    dataStore_.write(key, value);

    std::cout << "[TM] T" << txnId << " WRITE " << key
              << " = \"" << value << "\" (oldValue=\"" << oldValue << "\")\n";
    return OperationResult::SUCCESS;
}

// ---------------------------------------------------------------------------
// Commit
// ---------------------------------------------------------------------------

OperationResult TransactionManager::commit(int txnId) {
    auto it = transactions_.find(txnId);
    if (it == transactions_.end()) {
        std::cerr << "[TM] Unknown transaction T" << txnId << "\n";
        return OperationResult::ERROR;
    }
    Transaction* txn = it->second.get();

    if (!txn->isActive()) {
        std::cerr << "[TM] T" << txnId << " cannot commit (not ACTIVE)\n";
        return OperationResult::ERROR;
    }

    logManager_.appendLog(txnId, LogType::COMMIT);
    lockManager_.releaseLocks(txnId);
    txn->setState(TransactionState::COMMITTED);

    std::cout << "[TM] T" << txnId << " COMMIT\n";
    return OperationResult::SUCCESS;
}

// ---------------------------------------------------------------------------
// Abort (UNDO recovery + release locks)
// ---------------------------------------------------------------------------

OperationResult TransactionManager::abort(int txnId) {
    auto it = transactions_.find(txnId);
    if (it == transactions_.end()) {
        std::cerr << "[TM] Unknown transaction T" << txnId << "\n";
        return OperationResult::ERROR;
    }
    Transaction* txn = it->second.get();

    if (txn->isAborted()) {
        return OperationResult::ABORTED; // Already aborted
    }

    // UNDO all writes made by this transaction
    recoveryManager_.undo(txnId);

    logManager_.appendLog(txnId, LogType::ABORT);
    lockManager_.releaseLocks(txnId);
    txn->setState(TransactionState::ABORTED);

    std::cout << "[TM] T" << txnId << " ABORT (rolled back)\n";
    return OperationResult::SUCCESS;
}

// ---------------------------------------------------------------------------
// Deadlock detection and resolution
// ---------------------------------------------------------------------------

int TransactionManager::detectAndResolveDeadlock() {
    int victim = lockManager_.getWaitForGraph().detectCycle();
    if (victim == -1) {
        return -1; // No deadlock
    }

    std::cout << "[TM] Deadlock detected! Aborting victim T" << victim << "\n";
    abort(victim);
    return victim;
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

Transaction* TransactionManager::getTransaction(int txnId) {
    auto it = transactions_.find(txnId);
    if (it != transactions_.end()) {
        return it->second.get();
    }
    return nullptr;
}

DataStore& TransactionManager::getDataStore() {
    return dataStore_;
}

LogManager& TransactionManager::getLogManager() {
    return logManager_;
}

LockManager& TransactionManager::getLockManager() {
    return lockManager_;
}
