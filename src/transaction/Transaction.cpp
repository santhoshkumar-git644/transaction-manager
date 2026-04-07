#include "Transaction.h"

#include <iostream>

Transaction::Transaction(int txnId)
    : txnId_(txnId), state_(TransactionState::ACTIVE) {}

// ---------------------------------------------------------------------------
// Identity and state
// ---------------------------------------------------------------------------

int Transaction::getId() const {
    return txnId_;
}

TransactionState Transaction::getState() const {
    return state_;
}

void Transaction::setState(TransactionState state) {
    state_ = state;
}

bool Transaction::isActive() const {
    return state_ == TransactionState::ACTIVE;
}

bool Transaction::isBlocked() const {
    return state_ == TransactionState::BLOCKED;
}

bool Transaction::isCommitted() const {
    return state_ == TransactionState::COMMITTED;
}

bool Transaction::isAborted() const {
    return state_ == TransactionState::ABORTED;
}

// ---------------------------------------------------------------------------
// Write tracking
// ---------------------------------------------------------------------------

void Transaction::addWrittenKey(const std::string& key) {
    writtenKeys_.push_back(key);
}

const std::vector<std::string>& Transaction::getWrittenKeys() const {
    return writtenKeys_;
}

// ---------------------------------------------------------------------------
// Log record tracking
// ---------------------------------------------------------------------------

void Transaction::addLsn(int lsn) {
    lsns_.push_back(lsn);
}

const std::vector<int>& Transaction::getLsns() const {
    return lsns_;
}

// ---------------------------------------------------------------------------
// Diagnostics
// ---------------------------------------------------------------------------

static const char* stateName(TransactionState s) {
    switch (s) {
        case TransactionState::ACTIVE:    return "ACTIVE";
        case TransactionState::BLOCKED:   return "BLOCKED";
        case TransactionState::COMMITTED: return "COMMITTED";
        case TransactionState::ABORTED:   return "ABORTED";
    }
    return "UNKNOWN";
}

void Transaction::print() const {
    std::cout << "Transaction T" << txnId_
              << " [" << stateName(state_) << "]\n";
}
