#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <cstdint>
#include <chrono>
#include "../utils/enums.h"

class Transaction {
public:
    Transaction(uint32_t transaction_id);
    ~Transaction();

    uint32_t getTransactionId() const;
    TransactionState getState() const;
    void setState(TransactionState state);
    
    std::chrono::system_clock::time_point getStartTime() const;
    
    // TODO: Implement transaction-specific methods
    
private:
    uint32_t transaction_id_;
    TransactionState state_;
    std::chrono::system_clock::time_point start_time_;
};

#endif // TRANSACTION_H
