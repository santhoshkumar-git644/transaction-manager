#include <iostream>
#include "transaction/TransactionManager.h"
#include "concurrency/LockManager.h"
#include "recovery/LogManager.h"
#include "storage/DataStore.h"

int main() {
    std::cout << "=== Transaction Manager System ===\n\n";
    
    // Initialize components
    TransactionManager tm;
    LockManager lock_mgr;
    LogManager log_mgr("transaction.log");
    DataStore data_store;
    
    // Example: Create a transaction
    auto txn = tm.beginTransaction();
    std::cout << "Created transaction ID: " << txn->getTransactionId() << std::endl;
    
    // Example: Create a record
    auto record = std::make_shared<Record>(1);
    std::vector<uint8_t> data = {0x48, 0x65, 0x6C, 0x6C, 0x6F};  // "Hello"
    record->setData(data);
    data_store.insertRecord(record);
    std::cout << "Inserted record ID: " << record->getRecordId() << std::endl;
    
    // Example: Request a lock
    lock_mgr.requestLock(txn->getTransactionId(), 1, LockType::EXCLUSIVE);
    std::cout << "Acquired exclusive lock on resource 1\n";
    
    // Example: Log the transaction
    auto log_record = std::make_shared<LogRecord>(txn->getTransactionId(), LogRecord::LogType::BEGIN);
    log_mgr.writeLog(log_record);
    std::cout << "Logged transaction begin\n";
    
    // Commit transaction
    tm.commitTransaction(txn->getTransactionId());
    std::cout << "Transaction committed\n";
    
    return 0;
}
