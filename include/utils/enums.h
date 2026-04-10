#ifndef ENUMS_H
#define ENUMS_H

/**
 * @brief Transaction state during its lifecycle
 * ACTIVE       - Transaction is currently executing
 * COMMITTED    - Transaction completed successfully, changes are persistent
 * ABORTED      - Transaction rolled back, no changes made
 * WAITING      - Transaction waiting for locks or resources
 */
enum class TransactionState {
    ACTIVE,
    COMMITTED,
    ABORTED,
    WAITING
};

/**
 * @brief Lock types for concurrency control (2-Phase Locking)
 * SHARED              - Multiple transactions can read simultaneously
 * EXCLUSIVE           - Only one transaction can access the resource
 * INTENTION_SHARED    - Intent to acquire shared locks on children (for hierarchical locking)
 * INTENTION_EXCLUSIVE - Intent to acquire exclusive locks on children
 */
enum class LockType {
    SHARED,
    EXCLUSIVE,
    INTENTION_SHARED,
    INTENTION_EXCLUSIVE
};

/**
 * @brief Log record operation types for ARIES recovery
 * BEGIN        - Transaction begins
 * WRITE        - Write operation to data item
 * COMMIT       - Transaction commits
 * ABORT        - Transaction aborts
 * END          - End of transaction (for cleanup)
 * CHECKPOINT   - Checkpoint record
 */
enum class LogOperationType {
    BEGIN,
    WRITE,
    COMMIT,
    ABORT,
    END,
    CHECKPOINT
};

/**
 * @brief Recovery phases in ARIES algorithm
 * ANALYSIS  - Scan log to identify active transactions and dirty pages
 * REDO      - Replay committed and uncommitted transactions (crash recovery)
 * UNDO      - Rollback uncommitted transactions in reverse chronological order
 * COMPLETE  - Recovery finished, system operational
 */
enum class RecoveryState {
    ANALYSIS,
    REDO,
    UNDO,
    COMPLETE
};

#endif // ENUMS_H
