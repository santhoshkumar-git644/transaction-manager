#include "../../include/transaction/Transaction.h"
#include <algorithm>

Transaction::Transaction(uint32_t transaction_id)
    : transaction_id_(transaction_id), 
      state_(TransactionState::ACTIVE),
      start_time_(std::chrono::system_clock::now()),
      end_time_(std::chrono::system_clock::time_point()),  // epoch
      last_activity_time_(std::chrono::system_clock::now()),
      read_count_(0),
      write_count_(0) {
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
    // Record end time when transaction completes
    if (state == TransactionState::COMMITTED || state == TransactionState::ABORTED) {
        end_time_ = std::chrono::system_clock::now();
    }
}

std::chrono::system_clock::time_point Transaction::getStartTime() const {
    return start_time_;
}

std::chrono::system_clock::time_point Transaction::getEndTime() const {
    return end_time_;
}

std::chrono::system_clock::time_point Transaction::getLastActivityTime() const {
    return last_activity_time_;
}

void Transaction::recordAccessedRecord(uint32_t record_id) {
    // Only add if not already in the list
    auto it = std::find(accessed_records_.begin(), accessed_records_.end(), record_id);
    if (it == accessed_records_.end()) {
        accessed_records_.push_back(record_id);
    }
    // Update last activity time
    last_activity_time_ = std::chrono::system_clock::now();
}

const std::vector<uint32_t>& Transaction::getAccessedRecords() const {
    return accessed_records_;
}

uint32_t Transaction::getReadCount() const {
    return read_count_;
}

uint32_t Transaction::getWriteCount() const {
    return write_count_;
}

void Transaction::incrementReadCount() {
    read_count_++;
    last_activity_time_ = std::chrono::system_clock::now();
}

void Transaction::incrementWriteCount() {
    write_count_++;
    last_activity_time_ = std::chrono::system_clock::now();
}

bool Transaction::isCompleted() const {
    return state_ == TransactionState::COMMITTED || state_ == TransactionState::ABORTED;
}

const std::string& Transaction::getErrorMessage() const {
    return error_message_;
}

void Transaction::setErrorMessage(const std::string& error_msg) {
    error_message_ = error_msg;
}
