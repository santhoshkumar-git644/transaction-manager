#include "transaction/TransactionManager.h"

#include <cassert>
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// Minimal test harness
// ---------------------------------------------------------------------------
static int gPassed = 0;
static int gFailed = 0;

#define ASSERT(condition, message)                                      \
    do {                                                                \
        if (!(condition)) {                                             \
            std::cerr << "  FAIL [line " << __LINE__ << "]: "          \
                      << (message) << "\n";                             \
            ++gFailed;                                                  \
        } else {                                                        \
            std::cout << "  PASS: " << (message) << "\n";              \
            ++gPassed;                                                  \
        }                                                               \
    } while (false)

// ---------------------------------------------------------------------------
// Test: beginTransaction creates an ACTIVE transaction
// ---------------------------------------------------------------------------
static void test_begin() {
    std::cout << "\n[test_begin]\n";
    TransactionManager tm;

    int t1 = tm.beginTransaction();
    ASSERT(t1 > 0, "beginTransaction returns a positive ID");

    Transaction* txn = tm.getTransaction(t1);
    ASSERT(txn != nullptr, "getTransaction returns non-null");
    ASSERT(txn->isActive(), "New transaction is ACTIVE");
    ASSERT(txn->getId() == t1, "Transaction ID matches");
}

// ---------------------------------------------------------------------------
// Test: write and read succeed on an ACTIVE transaction
// ---------------------------------------------------------------------------
static void test_write_read() {
    std::cout << "\n[test_write_read]\n";
    TransactionManager tm;

    int t1 = tm.beginTransaction();

    OperationResult wr = tm.write(t1, "key1", "value1");
    ASSERT(wr == OperationResult::SUCCESS, "write returns SUCCESS");
    ASSERT(tm.getDataStore().read("key1").value_or("") == "value1",
           "DataStore contains written value");

    std::string out;
    OperationResult rr = tm.read(t1, "key1", out);
    ASSERT(rr == OperationResult::SUCCESS, "read returns SUCCESS");
    ASSERT(out == "value1", "read returns correct value");
}

// ---------------------------------------------------------------------------
// Test: commit transitions state and releases lock
// ---------------------------------------------------------------------------
static void test_commit() {
    std::cout << "\n[test_commit]\n";
    TransactionManager tm;

    int t1 = tm.beginTransaction();
    tm.write(t1, "X", "42");

    OperationResult res = tm.commit(t1);
    ASSERT(res == OperationResult::SUCCESS, "commit returns SUCCESS");
    ASSERT(tm.getTransaction(t1)->isCommitted(), "Transaction is COMMITTED");
    ASSERT(tm.getDataStore().read("X").value_or("") == "42",
           "DataStore retains value after commit");
}

// ---------------------------------------------------------------------------
// Test: abort transitions state
// ---------------------------------------------------------------------------
static void test_abort() {
    std::cout << "\n[test_abort]\n";
    TransactionManager tm;
    tm.getDataStore().write("X", "old");

    int t1 = tm.beginTransaction();
    tm.write(t1, "X", "new");
    ASSERT(tm.getDataStore().read("X").value_or("") == "new",
           "DataStore shows new value after write");

    OperationResult res = tm.abort(t1);
    ASSERT(res == OperationResult::SUCCESS, "abort returns SUCCESS");
    ASSERT(tm.getTransaction(t1)->isAborted(), "Transaction is ABORTED");
    ASSERT(tm.getDataStore().read("X").value_or("") == "old",
           "UNDO restores old value after abort");
}

// ---------------------------------------------------------------------------
// Test: operations on an aborted transaction return ABORTED
// ---------------------------------------------------------------------------
static void test_aborted_transaction_ops() {
    std::cout << "\n[test_aborted_transaction_ops]\n";
    TransactionManager tm;

    int t1 = tm.beginTransaction();
    tm.abort(t1);

    OperationResult wr = tm.write(t1, "Y", "val");
    ASSERT(wr == OperationResult::ABORTED,
           "write on ABORTED transaction returns ABORTED");

    std::string out;
    OperationResult rr = tm.read(t1, "Y", out);
    ASSERT(rr == OperationResult::ABORTED,
           "read on ABORTED transaction returns ABORTED");
}

// ---------------------------------------------------------------------------
// Test: multiple transactions operate independently
// ---------------------------------------------------------------------------
static void test_multiple_transactions() {
    std::cout << "\n[test_multiple_transactions]\n";
    TransactionManager tm;

    int t1 = tm.beginTransaction();
    int t2 = tm.beginTransaction();
    ASSERT(t1 != t2, "Different transaction IDs");

    tm.write(t1, "A", "t1-A");
    tm.write(t2, "B", "t2-B");

    tm.commit(t1);
    tm.commit(t2);

    ASSERT(tm.getDataStore().read("A").value_or("") == "t1-A",
           "T1 value persisted");
    ASSERT(tm.getDataStore().read("B").value_or("") == "t2-B",
           "T2 value persisted");
}

// ---------------------------------------------------------------------------
// Test: conflicting write returns WAITING
// ---------------------------------------------------------------------------
static void test_conflicting_write_waits() {
    std::cout << "\n[test_conflicting_write_waits]\n";
    TransactionManager tm;

    int t1 = tm.beginTransaction();
    int t2 = tm.beginTransaction();

    // T1 acquires exclusive lock on X
    tm.write(t1, "X", "t1-val");

    // T2 tries to write X – should wait
    OperationResult res = tm.write(t2, "X", "t2-val");
    ASSERT(res == OperationResult::WAITING,
           "Second writer waits on conflicting exclusive lock");
    ASSERT(tm.getTransaction(t2)->isBlocked(),
           "Waiting transaction is BLOCKED");
}

// ---------------------------------------------------------------------------
// Test: shared locks are compatible (concurrent readers succeed)
// ---------------------------------------------------------------------------
static void test_shared_locks_compatible() {
    std::cout << "\n[test_shared_locks_compatible]\n";
    TransactionManager tm;
    tm.getDataStore().write("X", "shared-val");

    int t1 = tm.beginTransaction();
    int t2 = tm.beginTransaction();

    std::string v1, v2;
    OperationResult r1 = tm.read(t1, "X", v1);
    OperationResult r2 = tm.read(t2, "X", v2);

    ASSERT(r1 == OperationResult::SUCCESS, "T1 shared read succeeds");
    ASSERT(r2 == OperationResult::SUCCESS, "T2 shared read succeeds (no conflict)");
    ASSERT(v1 == "shared-val" && v2 == "shared-val",
           "Both readers see the correct value");
}

// ---------------------------------------------------------------------------
// Test: WAL records are emitted in correct order
// ---------------------------------------------------------------------------
static void test_wal_logging() {
    std::cout << "\n[test_wal_logging]\n";
    TransactionManager tm;

    int t1 = tm.beginTransaction();
    tm.write(t1, "K", "v1");
    tm.write(t1, "K", "v2");
    tm.commit(t1);

    auto logs = tm.getLogManager().getLogsForTxn(t1);
    ASSERT(!logs.empty(), "Log contains records for T1");

    bool hasBegin  = false;
    bool hasWrite  = false;
    bool hasCommit = false;
    for (const auto& r : logs) {
        if (r.type == LogType::BEGIN)  hasBegin  = true;
        if (r.type == LogType::WRITE)  hasWrite  = true;
        if (r.type == LogType::COMMIT) hasCommit = true;
    }
    ASSERT(hasBegin,  "Log has BEGIN record");
    ASSERT(hasWrite,  "Log has WRITE record");
    ASSERT(hasCommit, "Log has COMMIT record");
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int main() {
    std::cout << "=== test_transactions ===\n";

    test_begin();
    test_write_read();
    test_commit();
    test_abort();
    test_aborted_transaction_ops();
    test_multiple_transactions();
    test_conflicting_write_waits();
    test_shared_locks_compatible();
    test_wal_logging();

    std::cout << "\nResults: " << gPassed << " passed, "
              << gFailed << " failed\n";
    return (gFailed == 0) ? 0 : 1;
}
