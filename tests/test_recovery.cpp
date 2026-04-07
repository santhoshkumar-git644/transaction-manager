#include "transaction/TransactionManager.h"
#include "recovery/LogManager.h"
#include "recovery/RecoveryManager.h"
#include "storage/DataStore.h"

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
// Test: WAL log records are created in correct order for a single transaction
// ---------------------------------------------------------------------------
static void test_wal_record_order() {
    std::cout << "\n[test_wal_record_order]\n";
    TransactionManager tm;

    int t1 = tm.beginTransaction();
    tm.write(t1, "A", "v1");
    tm.write(t1, "B", "v2");
    tm.commit(t1);

    auto logs = tm.getLogManager().getLogsForTxn(t1);

    ASSERT(!logs.empty(), "Log is non-empty for T1");
    ASSERT(logs.front().type == LogType::BEGIN,   "First record is BEGIN");
    ASSERT(logs.back().type  == LogType::COMMIT,  "Last record is COMMIT");

    // All LSNs must be strictly increasing
    bool monotone = true;
    for (size_t i = 1; i < logs.size(); ++i) {
        if (logs[i].lsn <= logs[i-1].lsn) {
            monotone = false;
            break;
        }
    }
    ASSERT(monotone, "LSNs are strictly increasing");
}

// ---------------------------------------------------------------------------
// Test: WRITE records carry correct old and new values
// ---------------------------------------------------------------------------
static void test_wal_write_values() {
    std::cout << "\n[test_wal_write_values]\n";
    TransactionManager tm;
    tm.getDataStore().write("K", "before");

    int t1 = tm.beginTransaction();
    tm.write(t1, "K", "after");
    tm.commit(t1);

    auto logs = tm.getLogManager().getLogsForTxn(t1);
    bool foundWrite = false;
    for (const auto& r : logs) {
        if (r.type == LogType::WRITE && r.key == "K") {
            ASSERT(r.oldValue == "before", "WRITE log has correct oldValue");
            ASSERT(r.newValue == "after",  "WRITE log has correct newValue");
            foundWrite = true;
        }
    }
    ASSERT(foundWrite, "WRITE log record exists for key K");
}

// ---------------------------------------------------------------------------
// Test: UNDO restores the single-write before-image on abort
// ---------------------------------------------------------------------------
static void test_undo_single_write() {
    std::cout << "\n[test_undo_single_write]\n";
    TransactionManager tm;
    tm.getDataStore().write("X", "initial");

    int t1 = tm.beginTransaction();
    tm.write(t1, "X", "modified");
    ASSERT(tm.getDataStore().read("X").value_or("") == "modified",
           "DataStore reflects write before abort");

    tm.abort(t1);
    ASSERT(tm.getDataStore().read("X").value_or("") == "initial",
           "UNDO restores initial value after abort");
}

// ---------------------------------------------------------------------------
// Test: UNDO with multiple writes undoes in reverse order
// ---------------------------------------------------------------------------
static void test_undo_multiple_writes() {
    std::cout << "\n[test_undo_multiple_writes]\n";
    TransactionManager tm;
    tm.getDataStore().write("X", "v0");

    int t1 = tm.beginTransaction();
    tm.write(t1, "X", "v1");
    tm.write(t1, "X", "v2");
    tm.write(t1, "X", "v3");
    ASSERT(tm.getDataStore().read("X").value_or("") == "v3",
           "DataStore holds last written value");

    tm.abort(t1);
    // The UNDO of the first write (X: v0→v1) should leave X = v0
    ASSERT(tm.getDataStore().read("X").value_or("") == "v0",
           "UNDO of multiple writes restores original value");
}

// ---------------------------------------------------------------------------
// Test: UNDO removes a key that did not exist before the transaction
// ---------------------------------------------------------------------------
static void test_undo_new_key() {
    std::cout << "\n[test_undo_new_key]\n";
    TransactionManager tm;
    ASSERT(!tm.getDataStore().exists("NEWKEY"), "NEWKEY does not exist initially");

    int t1 = tm.beginTransaction();
    tm.write(t1, "NEWKEY", "somevalue");
    ASSERT(tm.getDataStore().exists("NEWKEY"), "NEWKEY exists after write");

    tm.abort(t1);
    ASSERT(!tm.getDataStore().exists("NEWKEY"),
           "NEWKEY removed by UNDO (key did not exist before transaction)");
}

// ---------------------------------------------------------------------------
// Test: committed transaction's changes persist (REDO semantics verified)
// ---------------------------------------------------------------------------
static void test_committed_changes_persist() {
    std::cout << "\n[test_committed_changes_persist]\n";
    TransactionManager tm;

    int t1 = tm.beginTransaction();
    tm.write(t1, "P", "committed-value");
    tm.commit(t1);

    ASSERT(tm.getDataStore().read("P").value_or("") == "committed-value",
           "Committed value persists in DataStore");
    ASSERT(tm.getTransaction(t1)->isCommitted(), "Transaction is COMMITTED");
}

// ---------------------------------------------------------------------------
// Test: RecoveryManager::recover() undoes in-flight transactions
// ---------------------------------------------------------------------------
static void test_crash_recovery_undo_inflight() {
    std::cout << "\n[test_crash_recovery_undo_inflight]\n";

    // Simulate a crash scenario using LogManager + DataStore + RecoveryManager directly
    LogManager  logMgr;
    DataStore   ds;
    RecoveryManager rm(logMgr, ds);

    ds.write("M", "original");

    // Simulate T1: BEGIN, WRITE, then crash (no COMMIT/ABORT)
    logMgr.appendLog(10, LogType::BEGIN);
    logMgr.appendLog(10, LogType::WRITE, "M", "original", "crashed-value");
    ds.write("M", "crashed-value"); // applied but not committed

    ASSERT(ds.read("M").value_or("") == "crashed-value",
           "DataStore contains in-flight value before recovery");

    // recover() should UNDO T10 because it has no COMMIT/ABORT
    rm.recover();

    ASSERT(ds.read("M").value_or("") == "original",
           "Recovery undoes in-flight transaction and restores original value");
}

// ---------------------------------------------------------------------------
// Test: RecoveryManager::recover() redoes committed transactions
// ---------------------------------------------------------------------------
static void test_crash_recovery_redo_committed() {
    std::cout << "\n[test_crash_recovery_redo_committed]\n";

    LogManager  logMgr;
    DataStore   ds;
    RecoveryManager rm(logMgr, ds);

    // Log T1 as committed (committed before crash) but data might be lost
    logMgr.appendLog(1, LogType::BEGIN);
    logMgr.appendLog(1, LogType::WRITE, "N", "", "committed-value");
    logMgr.appendLog(1, LogType::COMMIT);

    // DataStore is empty (simulating data loss after crash)
    ASSERT(!ds.exists("N"), "DataStore empty before REDO");

    rm.recover();

    ASSERT(ds.read("N").value_or("") == "committed-value",
           "Recovery redoes committed transaction and restores value");
}

// ---------------------------------------------------------------------------
// Test: ABORT log record is emitted when a transaction aborts
// ---------------------------------------------------------------------------
static void test_abort_log_record() {
    std::cout << "\n[test_abort_log_record]\n";
    TransactionManager tm;

    int t1 = tm.beginTransaction();
    tm.write(t1, "Z", "z-value");
    tm.abort(t1);

    auto logs = tm.getLogManager().getLogsForTxn(t1);
    bool hasAbort = false;
    for (const auto& r : logs) {
        if (r.type == LogType::ABORT) hasAbort = true;
    }
    ASSERT(hasAbort, "ABORT log record present after transaction abort");
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int main() {
    std::cout << "=== test_recovery ===\n";

    test_wal_record_order();
    test_wal_write_values();
    test_undo_single_write();
    test_undo_multiple_writes();
    test_undo_new_key();
    test_committed_changes_persist();
    test_crash_recovery_undo_inflight();
    test_crash_recovery_redo_committed();
    test_abort_log_record();

    std::cout << "\nResults: " << gPassed << " passed, "
              << gFailed << " failed\n";
    return (gFailed == 0) ? 0 : 1;
}
