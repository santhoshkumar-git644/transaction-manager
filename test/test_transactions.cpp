#include <cassert>
#include <iostream>
#include <vector>

#include "../include/transaction/TransactionManager.h"

int main() {
    std::cout << "Testing Transaction Manager\n";

    TransactionManager tm;

    // Begin transactions and verify ID allocation and active set.
    auto txn1 = tm.beginTransaction();
    auto txn2 = tm.beginTransaction();
    auto txn3 = tm.beginTransaction();

    assert(txn1 != nullptr);
    assert(txn2 != nullptr);
    assert(txn3 != nullptr);
    assert(txn2->getTransactionId() == txn1->getTransactionId() + 1);
    assert(txn3->getTransactionId() == txn2->getTransactionId() + 1);
    assert(tm.getActiveTransactionCount() == 3);
    assert(tm.getTotalTransactionCount() == 3);

    // Update transaction-local metrics and metadata.
    txn1->incrementReadCount();
    txn1->incrementReadCount();
    txn1->incrementWriteCount();
    txn1->recordAccessedRecord(10);
    txn1->recordAccessedRecord(10);  // duplicate should not be added twice
    txn1->recordAccessedRecord(22);
    txn1->setErrorMessage("synthetic error");

    assert(txn1->getReadCount() == 2);
    assert(txn1->getWriteCount() == 1);
    assert(txn1->getAccessedRecords().size() == 2);
    assert(txn1->getErrorMessage() == "synthetic error");
    assert(!txn1->isCompleted());

    // Commit and abort paths.
    assert(tm.commitTransaction(txn1->getTransactionId()));
    assert(tm.abortTransaction(txn2->getTransactionId()));
    assert(txn1->isCompleted());
    assert(txn2->isCompleted());
    assert(txn1->getState() == TransactionState::COMMITTED);
    assert(txn2->getState() == TransactionState::ABORTED);

    assert(tm.getCommittedCount() == 1);
    assert(tm.getAbortedCount() == 1);
    assert(tm.getActiveTransactionCount() == 1);
    assert(tm.getTotalTransactionCount() == 3);
    assert(!tm.isTransactionActive(txn1->getTransactionId()));
    assert(!tm.isTransactionActive(txn2->getTransactionId()));
    assert(tm.isTransactionActive(txn3->getTransactionId()));

    // Transaction lookup should include completed and active transactions.
    assert(tm.getTransaction(txn1->getTransactionId()) != nullptr);
    assert(tm.getTransaction(txn2->getTransactionId()) != nullptr);
    assert(tm.getTransaction(txn3->getTransactionId()) != nullptr);
    assert(tm.getTransaction(999999) == nullptr);

    // Negative commit/abort should fail for unknown IDs.
    assert(!tm.commitTransaction(999999));
    assert(!tm.abortTransaction(999999));

    // Abort all should move remaining active transactions to aborted.
    tm.abortAllTransactions();
    assert(tm.getActiveTransactionCount() == 0);
    assert(tm.getAbortedCount() == 2);
    assert(tm.getTotalTransactionCount() == 3);

    tm.clearCompletedTransactions();
    assert(tm.getTotalTransactionCount() == 0);

    std::cout << "Transaction lifecycle and metrics checks passed\n";
    return 0;
}
