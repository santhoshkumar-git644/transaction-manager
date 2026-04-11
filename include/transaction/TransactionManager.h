#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include <map>
#include <memory>
#include <cstdint>
#include <vector>
#include "Transaction.h"
#include "../utils/constants.h"

/**
 * @class TransactionManager
 * @brief Manages the lifecycle of all database transactions.
 *
 * Responsibilities:
 * - Create and begin new transactions
 * - Track active transactions
 * - Coordinate transaction commit/abort with other components
 * - Provide transaction statistics and monitoring
 * - Enforce transaction state machine
 *
 * Transaction lifecycle:
 * BEGIN -> ACTIVE -> (COMMIT -> COMMITTED) or (ABORT -> ABORTED)
 */
class TransactionManager {
public:
    /**
     * @brief Construct a new TransactionManager.
     */
    TransactionManager();
    ~TransactionManager();
    
    /**
     * @brief Begin a new transaction.
     * @return Shared pointer to newly created transaction
     * @note Transaction is automatically assigned a unique ID
     */
    std::shared_ptr<Transaction> beginTransaction();
    
    /**
     * @brief Commit a transaction.
     * @param transaction_id ID of transaction to commit
     * @return true if commit successful, false if transaction not found
     * @note Sets transaction state to COMMITTED and removes from active set
     */
    bool commitTransaction(uint32_t transaction_id);
    
    /**
     * @brief Abort a transaction.
     * @param transaction_id ID of transaction to abort
     * @return true if abort successful, false if transaction not found
     * @note Sets transaction state to ABORTED and removes from active set
     */
    bool abortTransaction(uint32_t transaction_id);
    
    /**
     * @brief Get a transaction by ID.
     * @param transaction_id ID of transaction to retrieve
     * @return Shared pointer to transaction, or nullptr if not found
     * @note Searches both active and completed transactions
     */
    std::shared_ptr<Transaction> getTransaction(uint32_t transaction_id) const;
    
    /**
     * @brief Check if a transaction is active.
     * @param transaction_id ID of transaction to check
     * @return true if transaction exists and is ACTIVE
     */
    bool isTransactionActive(uint32_t transaction_id) const;
    
    /**
     * @brief Get all active transaction IDs.
     * @return Vector of active transaction IDs
     */
    std::vector<uint32_t> getActiveTransactionIds() const;
    
    /**
     * @brief Get the number of active transactions.
     * @return Count of active transactions
     */
    size_t getActiveTransactionCount() const;
    
    /**
     * @brief Get total number of transactions (active + completed).
     * @return Count of all transactions
     */
    size_t getTotalTransactionCount() const;
    
    /**
     * @brief Get the number of committed transactions.
     * @return Count of successfully committed transactions
     */
    uint32_t getCommittedCount() const;
    
    /**
     * @brief Get the number of aborted transactions.
     * @return Count of aborted transactions
     */
    uint32_t getAbortedCount() const;
    
    /**
     * @brief Clear all completed transactions from history.
     * @note Does not affect active transactions
     */
    void clearCompletedTransactions();
    
    /**
     * @brief Abort all active transactions.
     * @note Used during system shutdown or recovery
     */
    void abortAllTransactions();
    
private:
    std::map<uint32_t, std::shared_ptr<Transaction>> active_transactions_;   ///< Currently active transactions
    std::map<uint32_t, std::shared_ptr<Transaction>> completed_transactions_;///< History of completed transactions
    uint32_t next_transaction_id_;                                           ///< Next transaction ID to assign
    uint32_t committed_count_;                                               ///< Statistics: committed transactions
    uint32_t aborted_count_;                                                 ///< Statistics: aborted transactions
};

#endif // TRANSACTION_MANAGER_H
