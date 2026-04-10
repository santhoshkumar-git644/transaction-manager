#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include <map>
#include <memory>
#include <cstdint>
#include "Transaction.h"

class TransactionManager {
public:
    TransactionManager();
    ~TransactionManager();
    
    std::shared_ptr<Transaction> beginTransaction();
    void commitTransaction(uint32_t transaction_id);
    void abortTransaction(uint32_t transaction_id);
    
    std::shared_ptr<Transaction> getTransaction(uint32_t transaction_id) const;
    
private:
    std::map<uint32_t, std::shared_ptr<Transaction>> active_transactions_;
    uint32_t next_transaction_id_;
};

#endif // TRANSACTION_MANAGER_H
