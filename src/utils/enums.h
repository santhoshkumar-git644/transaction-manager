#pragma once

// Transaction lifecycle states
enum class TransactionState {
    ACTIVE,
    BLOCKED,
    COMMITTED,
    ABORTED
};

// Lock granularity modes (Strict 2PL)
enum class LockMode {
    SHARED,    // Read lock – multiple transactions may hold concurrently
    EXCLUSIVE  // Write lock – only one transaction may hold
};

// Result of a lock acquisition attempt
enum class LockStatus {
    GRANTED,  // Lock was immediately granted
    WAITING,  // Lock request queued (conflict exists)
    DENIED    // Lock cannot be granted (e.g., already aborted)
};

// Type tag stored in each WAL log record
enum class LogType {
    BEGIN,
    WRITE,
    COMMIT,
    ABORT
};

// High-level result returned by TransactionManager operations
enum class OperationResult {
    SUCCESS,
    WAITING,   // Blocked on a lock held by another transaction
    ABORTED,   // Transaction has been aborted (e.g., deadlock victim)
    ERROR      // Invalid state or bad arguments
};
