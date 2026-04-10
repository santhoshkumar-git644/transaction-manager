#include <iostream>
#include "../include/transaction/TransactionManager.h"

int main() {
    std::cout << "Transaction Manager - Running Tests\n";
    
    TransactionManager tm;
    
    // Test 1: Create transactions
    auto txn1 = tm.beginTransaction();
    auto txn2 = tm.beginTransaction();
    
    std::cout << "Transaction 1 ID: " << txn1->getTransactionId() << std::endl;
    std::cout << "Transaction 2 ID: " << txn2->getTransactionId() << std::endl;
    
    // TODO: Add more comprehensive transaction tests
    
    return 0;
}
