#include "../../include/transaction/Transaction.h"

Transaction::Transaction(uint32_t transaction_id)
    : transaction_id_(transaction_id), 
      state_(TransactionState::ACTIVE),
      start_time_(std::chrono::system_clock::now()) {
}

Transaction::~Transaction() {
}

uint32_t Transaction::getTransactionId() const {
    return transaction_id_;
}

TransactionState Transaction::getState() const {
    return state_;
}

void Transaction::setState(TransactionState state) {
    state_ = state;
}

std::chrono::system_clock::time_point Transaction::getStartTime() const {
    return start_time_;
}
