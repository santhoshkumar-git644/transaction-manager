#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <cstddef>

// ============================================================================
// BUFFER MANAGEMENT CONSTANTS
// ============================================================================

/** Size of a single page in the buffer pool (4 KB) */
constexpr size_t PAGE_SIZE = 4096;

/** Maximum number of pages in buffer pool (1024 pages = 4 MB total) */
constexpr size_t BUFFER_POOL_SIZE = 1024;

/** Buffer pool size in bytes (4 MB) */
constexpr size_t BUFFER_POOL_BYTES = BUFFER_POOL_SIZE * PAGE_SIZE;

// ============================================================================
// TRANSACTION MANAGEMENT CONSTANTS
// ============================================================================

/** Maximum number of concurrent transactions allowed */
constexpr uint32_t MAX_TRANSACTIONS = 1000;

/** Default transaction timeout in milliseconds (5 seconds) */
constexpr uint32_t DEFAULT_TIMEOUT_MS = 5000;

/** Maximum lock wait time before deadlock detection (10 seconds) */
constexpr uint32_t LOCK_WAIT_TIMEOUT_MS = 10000;

/** Initial transaction ID counter value */
constexpr uint32_t INITIAL_TXN_ID = 1;

// ============================================================================
// LOGGING & RECOVERY CONSTANTS
// ============================================================================

/** Size of log buffer before flushing to disk (8 KB) */
constexpr size_t LOG_BUFFER_SIZE = 8192;

/** Default path for transaction log file */
constexpr const char* LOG_FILE_PATH = "transaction.log";

/** Default path for checkpoint file */
constexpr const char* CHECKPOINT_FILE_PATH = "checkpoint.dat";

/** Maximum log file size before rotation (100 MB) */
constexpr size_t MAX_LOG_FILE_SIZE = 100 * 1024 * 1024;

/** Checkpoint interval in number of log records */
constexpr uint32_t CHECKPOINT_INTERVAL = 10000;

// ============================================================================
// LOCK MANAGER CONSTANTS
// ============================================================================

/** Maximum number of locks that can be held simultaneously */
constexpr uint32_t MAX_LOCKS = 5000;

/** Deadlock detection cycle check interval (milliseconds) */
constexpr uint32_t DEADLOCK_CHECK_INTERVAL_MS = 1000;

// ============================================================================
// DATA STORAGE CONSTANTS
// ============================================================================

/** Initial record ID counter value */
constexpr uint32_t INITIAL_RECORD_ID = 1;

/** Maximum record size in bytes (1 MB) */
constexpr size_t MAX_RECORD_SIZE = 1024 * 1024;

#endif // CONSTANTS_H
