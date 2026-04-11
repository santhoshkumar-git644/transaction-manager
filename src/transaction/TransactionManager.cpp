#include "../../include/transaction/TransactionManager.h"

TransactionManager::TransactionManager() 
    : next_transaction_id_(INITIAL_TXN_ID),
      committed_count_(0),
      aborted_count_(0) {
}

TransactionManager::~TransactionManager() {
    abortAllTransactions();
}

std::shared_ptr<Transaction> TransactionManager::beginTransaction() {
    auto txn = std::make_shared<Transaction>(next_transaction_id_++);
    active_transactions_[txn->getTransactionId()] = txn;
    return txn;
}

bool TransactionManager::commitTransaction(uint32_t transaction_id) {
    auto it = active_transactions_.find(transaction_id);
    if (it != active_transactions_.end()) {
        auto txn = it->second;
        txn->setState(TransactionState::COMMITTED);
        committed_count_++;
        
        // Move to completed transactions history
        completed_transactions_[transaction_id] = txn;
        active_transactions_.erase(it);
        return true;
    }
    return false;
}

bool TransactionManager::abortTransaction(uint32_t transaction_id) {
    auto it = active_transactions_.find(transaction_id);
    if (it != active_transactions_.end()) {
        auto txn = it->second;
        txn->setState(TransactionState::ABORTED);
        aborted_count_++;
        
        // Move to completed transactions history
        completed_transactions_[transaction_id] = txn;
        active_transactions_.erase(it);
        return true;
    }
    return false;
}

std::shared_ptr<Transaction> TransactionManager::getTransaction(uint32_t transaction_id) const {
    // First check active transactions
    auto active_it = active_transactions_.find(transaction_id);
    if (active_it != active_transactions_.end()) {
        return active_it->second;
    }
    
    // Then check completed transactions
    auto completed_it = completed_transactions_.find(transaction_id);
    if (completed_it != completed_transactions_.end()) {
        return completed_it->second;
    }
    
    return nullptr;
}

bool TransactionManager::isTransactionActive(uint32_t transaction_id) const {
    auto it = active_transactions_.find(transaction_id);
    return it != active_transactions_.end() && it->second->getState() == TransactionState::ACTIVE;
}

std::vector<uint32_t> TransactionManager::getActiveTransactionIds() const {
    std::vector<uint32_t> ids;
    for (const auto& [id, txn] : active_transactions_) {
        ids.push_back(id);
    }
    return ids;
}

size_t TransactionManager::getActiveTransactionCount() const {
    return active_transactions_.size();
}

size_t TransactionManager::getTotalTransactionCount() const {
    return active_transactions_.size() + completed_transactions_.size();
}

uint32_t TransactionManager::getCommittedCount() const {
    return committed_count_;
}

uint32_t TransactionManager::getAbortedCount() const {
    return aborted_count_;
}

void TransactionManager::clearCompletedTransactions() {
    completed_transactions_.clear();
}

void TransactionManager::abortAllTransactions() {
    // Collect IDs first to avoid modification during iteration
    auto ids = getActiveTransactionIds();
    for (uint32_t id : ids) {
        abortTransaction(id);
    }
}
