#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <cstdint>
#include <chrono>
#include <vector>
#include <string>
#include "../utils/enums.h"

/**
 * @class Transaction
 * @brief Represents a single database transaction with state management.
 *
 * A Transaction encapsulates:
 * - Unique transaction ID
 * - Current state (ACTIVE, COMMITTED, ABORTED, WAITING)
 * - Timestamps (start, end, last activity)
 * - List of accessed record IDs and operation counts
 * - Error tracking
 *
 * Transactions follow ACID properties:
 * - Atomicity: All-or-nothing execution
 * - Consistency: Data integrity preservation
 * - Isolation: No interference from concurrent transactions
 * - Durability: Committed changes persist
 */
class Transaction {
public:
    /**
     * @brief Construct a new Transaction with given ID.
     * @param transaction_id Unique transaction identifier
     */
    Transaction(uint32_t transaction_id);
    ~Transaction();

    /**
     * @brief Get the transaction's unique ID.
     * @return Transaction ID
     */
    uint32_t getTransactionId() const;

    /**
     * @brief Get the current transaction state.
     * @return Current TransactionState (ACTIVE, COMMITTED, ABORTED, WAITING)
     */
    TransactionState getState() const;

    /**
     * @brief Set the transaction state.
     * @param state New TransactionState
     */
    void setState(TransactionState state);
    
    /**
     * @brief Get the transaction start time.
     * @return System clock time point when transaction began
     */
    std::chrono::system_clock::time_point getStartTime() const;
    
    /**
     * @brief Get the transaction end time.
     * @return System clock time point when transaction completed, or epoch if still active
     */
    std::chrono::system_clock::time_point getEndTime() const;
    
    /**
     * @brief Get the transaction last activity time.
     * @return System clock time point of last operation
     */
    std::chrono::system_clock::time_point getLastActivityTime() const;
    
    /**
     * @brief Record that transaction accessed a record.
     * @param record_id ID of accessed record
     */
    void recordAccessedRecord(uint32_t record_id);
    
    /**
     * @brief Get all record IDs accessed by this transaction.
     * @return Vector of unique record IDs
     */
    const std::vector<uint32_t>& getAccessedRecords() const;
    
    /**
     * @brief Get the number of read operations.
     * @return Count of read operations
     */
    uint32_t getReadCount() const;
    
    /**
     * @brief Get the number of write operations.
     * @return Count of write operations
     */
    uint32_t getWriteCount() const;
    
    /**
     * @brief Increment read operation counter.
     */
    void incrementReadCount();
    
    /**
     * @brief Increment write operation counter.
     */
    void incrementWriteCount();
    
    /**
     * @brief Check if transaction has completed (committed or aborted).
     * @return true if state is COMMITTED or ABORTED
     */
    bool isCompleted() const;
    
    /**
     * @brief Get error message if transaction failed.
     * @return Error message, or empty string if no error
     */
    const std::string& getErrorMessage() const;
    
    /**
     * @brief Set error message for failed transaction.
     * @param error_msg Error description
     */
    void setErrorMessage(const std::string& error_msg);
    
private:
    uint32_t transaction_id_;                                    ///< Unique transaction ID
    TransactionState state_;                                     ///< Current transaction state
    std::chrono::system_clock::time_point start_time_;          ///< Transaction start time
    std::chrono::system_clock::time_point end_time_;            ///< Transaction end time
    std::chrono::system_clock::time_point last_activity_time_;  ///< Last activity timestamp
    std::vector<uint32_t> accessed_records_;                     ///< IDs of accessed records
    uint32_t read_count_;                                        ///< Number of read operations
    uint32_t write_count_;                                       ///< Number of write operations
    std::string error_message_;                                  ///< Error message if any
};

#endif // TRANSACTION_H
