#include "../../include/transaction/TransactionManager.h"

TransactionManager::TransactionManager() : next_transaction_id_(1) {
}

TransactionManager::~TransactionManager() {
}

std::shared_ptr<Transaction> TransactionManager::beginTransaction() {
    auto txn = std::make_shared<Transaction>(next_transaction_id_++);
    active_transactions_[txn->getTransactionId()] = txn;
    return txn;
}

void TransactionManager::commitTransaction(uint32_t transaction_id) {
    auto it = active_transactions_.find(transaction_id);
    if (it != active_transactions_.end()) {
        it->second->setState(TransactionState::COMMITTED);
        // TODO: Implement commit logic
        active_transactions_.erase(it);
    }
}

void TransactionManager::abortTransaction(uint32_t transaction_id) {
    auto it = active_transactions_.find(transaction_id);
    if (it != active_transactions_.end()) {
        it->second->setState(TransactionState::ABORTED);
        // TODO: Implement abort logic
        active_transactions_.erase(it);
    }
}

std::shared_ptr<Transaction> TransactionManager::getTransaction(uint32_t transaction_id) const {
    auto it = active_transactions_.find(transaction_id);
    if (it != active_transactions_.end()) {
        return it->second;
    }
    return nullptr;
}
