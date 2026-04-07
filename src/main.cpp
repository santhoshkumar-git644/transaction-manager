#include "transaction/TransactionManager.h"

#include <iostream>
#include <string>

// Helper to print a section header
static void header(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n"
              << "  " << title << "\n"
              << std::string(60, '=') << "\n";
}

// ---------------------------------------------------------------------------
// Scenario 1 – Normal transaction (begin / write / read / commit)
// ---------------------------------------------------------------------------
static void scenario1() {
    header("Scenario 1: Normal Transaction");

    TransactionManager tm;

    int t1 = tm.beginTransaction();

    tm.write(t1, "X", "100");
    tm.write(t1, "Y", "200");

    std::string val;
    tm.read(t1, "X", val);
    std::cout << "  T1 sees X = \"" << val << "\"\n";

    tm.commit(t1);

    std::cout << "\nDataStore after T1 commits:\n";
    tm.getDataStore().print();
}

// ---------------------------------------------------------------------------
// Scenario 2 – Conflicting transactions (lock wait, then resolve)
// ---------------------------------------------------------------------------
static void scenario2() {
    header("Scenario 2: Conflicting Transactions (Lock Wait)");

    TransactionManager tm;

    // Pre-populate the store with an initial value
    tm.getDataStore().write("X", "original");

    int t1 = tm.beginTransaction();
    int t2 = tm.beginTransaction();

    // T1 acquires exclusive lock on X and writes
    tm.write(t1, "X", "T1-value");

    // T2 tries to write X – T1 holds the exclusive lock → WAITING
    OperationResult res = tm.write(t2, "X", "T2-value");
    if (res == OperationResult::WAITING) {
        std::cout << "  [INFO] T" << t2
                  << " is blocked waiting for the lock on X\n";
    }

    // T1 commits → releases lock on X
    tm.commit(t1);
    std::cout << "\nT1 committed. T2 can now retry its write.\n";

    // T2 retries – the lock is now free
    // Reset T2's state to ACTIVE (was BLOCKED during the wait)
    tm.getTransaction(t2)->setState(TransactionState::ACTIVE);
    tm.write(t2, "X", "T2-value");
    tm.commit(t2);

    std::cout << "\nDataStore after both transactions commit:\n";
    tm.getDataStore().print();
}

// ---------------------------------------------------------------------------
// Scenario 3 – Deadlock: T3 waits for T4, T4 waits for T3
// ---------------------------------------------------------------------------
static void scenario3() {
    header("Scenario 3: Deadlock Detection and Resolution");

    TransactionManager tm;
    tm.getDataStore().write("X", "x0");
    tm.getDataStore().write("Y", "y0");

    int t3 = tm.beginTransaction();
    int t4 = tm.beginTransaction();

    // T3 locks X, T4 locks Y
    tm.write(t3, "X", "x-by-T3");
    tm.write(t4, "Y", "y-by-T4");

    // T3 tries to lock Y (held by T4) → WAITING, edge T3→T4 added to WFG
    OperationResult r1 = tm.write(t3, "Y", "y-by-T3");
    if (r1 == OperationResult::WAITING) {
        std::cout << "  [INFO] T" << t3 << " waiting for Y (held by T" << t4 << ")\n";
    }

    // T4 tries to lock X (held by T3) → WAITING, edge T4→T3 added to WFG
    //   The WFG now contains the cycle T3→T4→T3 → deadlock!
    OperationResult r2 = tm.write(t4, "X", "x-by-T4");
    if (r2 == OperationResult::WAITING) {
        std::cout << "  [INFO] T" << t4 << " waiting for X (held by T" << t3 << ")\n";
    }

    std::cout << "\nWait-For Graph before deadlock resolution:\n";
    tm.getLockManager().getWaitForGraph().print();

    // Detect and resolve the deadlock (youngest victim = highest ID = T4)
    int victim = tm.detectAndResolveDeadlock();
    std::cout << "  Victim aborted: T" << victim << "\n";

    // T3 can now acquire Y (T4 released it on abort)
    // Reset T3's state to ACTIVE
    tm.getTransaction(t3)->setState(TransactionState::ACTIVE);
    tm.write(t3, "Y", "y-by-T3");
    tm.commit(t3);

    std::cout << "\nDataStore after deadlock resolution:\n";
    tm.getDataStore().print();
}

// ---------------------------------------------------------------------------
// Scenario 4 – Rollback after abort (UNDO recovery)
// ---------------------------------------------------------------------------
static void scenario4() {
    header("Scenario 4: Rollback After Abort (UNDO Recovery)");

    TransactionManager tm;
    // Pre-load the store with known values
    tm.getDataStore().write("Z", "original-value");

    std::cout << "DataStore before T5:\n";
    tm.getDataStore().print();

    int t5 = tm.beginTransaction();
    tm.write(t5, "Z", "modified-by-T5");

    std::cout << "\nDataStore after T5 writes (before commit/abort):\n";
    tm.getDataStore().print();

    // Abort T5 – RecoveryManager will UNDO the write
    tm.abort(t5);

    std::cout << "\nDataStore after T5 aborts (UNDO applied):\n";
    tm.getDataStore().print();

    std::cout << "\nWAL contents:\n";
    tm.getLogManager().printLog();
}

// ---------------------------------------------------------------------------
// Scenario 5 – Read / write mix with shared and exclusive locks
// ---------------------------------------------------------------------------
static void scenario5() {
    header("Scenario 5: Shared Locks (Concurrent Readers)");

    TransactionManager tm;
    tm.getDataStore().write("A", "alpha");

    int t6 = tm.beginTransaction();
    int t7 = tm.beginTransaction();

    // Both transactions read A concurrently (shared locks are compatible)
    std::string v6, v7;
    tm.read(t6, "A", v6);
    tm.read(t7, "A", v7);

    std::cout << "  T" << t6 << " read A = \"" << v6 << "\"\n";
    std::cout << "  T" << t7 << " read A = \"" << v7 << "\"\n";
    std::cout << "  (Both shared reads succeed without blocking)\n";

    // T6 upgrades to exclusive (T7 still holds shared) → WAITING
    OperationResult res = tm.write(t6, "A", "alpha-updated");
    if (res == OperationResult::WAITING) {
        std::cout << "  [INFO] T" << t6
                  << " cannot upgrade to X-lock while T" << t7
                  << " holds S-lock\n";
    }

    // T7 commits, releasing the shared lock
    tm.commit(t7);

    // T6 retries the write
    tm.getTransaction(t6)->setState(TransactionState::ACTIVE);
    tm.write(t6, "A", "alpha-updated");
    tm.commit(t6);

    std::cout << "\nDataStore after scenario 5:\n";
    tm.getDataStore().print();
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main() {
    std::cout << "Transaction Manager Demo\n";
    std::cout << "========================\n";

    scenario1();
    scenario2();
    scenario3();
    scenario4();
    scenario5();

    std::cout << "\n[Demo complete]\n";
    return 0;
}
