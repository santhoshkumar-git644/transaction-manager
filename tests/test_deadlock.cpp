#include "transaction/TransactionManager.h"
#include "concurrency/WaitForGraph.h"

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
// Test: WaitForGraph – no cycle when empty
// ---------------------------------------------------------------------------
static void test_wfg_no_cycle_empty() {
    std::cout << "\n[test_wfg_no_cycle_empty]\n";
    WaitForGraph wfg;
    ASSERT(wfg.detectCycle() == -1, "Empty WFG has no cycle");
}

// ---------------------------------------------------------------------------
// Test: WaitForGraph – no cycle for simple chain A→B
// ---------------------------------------------------------------------------
static void test_wfg_no_cycle_chain() {
    std::cout << "\n[test_wfg_no_cycle_chain]\n";
    WaitForGraph wfg;
    wfg.addEdge(1, 2); // T1 waits for T2 – no cycle
    ASSERT(wfg.detectCycle() == -1, "Chain A->B has no cycle");
}

// ---------------------------------------------------------------------------
// Test: WaitForGraph – simple 2-node cycle T1→T2→T1
// ---------------------------------------------------------------------------
static void test_wfg_simple_cycle() {
    std::cout << "\n[test_wfg_simple_cycle]\n";
    WaitForGraph wfg;
    wfg.addEdge(1, 2); // T1 waits for T2
    wfg.addEdge(2, 1); // T2 waits for T1 → deadlock

    int victim = wfg.detectCycle();
    ASSERT(victim != -1, "Cycle T1->T2->T1 is detected");
    ASSERT(victim == 1 || victim == 2, "Victim is one of the cycle nodes");
}

// ---------------------------------------------------------------------------
// Test: WaitForGraph – 3-node cycle T1→T2→T3→T1
// ---------------------------------------------------------------------------
static void test_wfg_three_node_cycle() {
    std::cout << "\n[test_wfg_three_node_cycle]\n";
    WaitForGraph wfg;
    wfg.addEdge(1, 2);
    wfg.addEdge(2, 3);
    wfg.addEdge(3, 1);

    int victim = wfg.detectCycle();
    ASSERT(victim != -1, "Three-node cycle is detected");
    ASSERT(victim == 1 || victim == 2 || victim == 3,
           "Victim is one of the cycle nodes");
}

// ---------------------------------------------------------------------------
// Test: WaitForGraph – removeEdgesForTxn breaks the cycle
// ---------------------------------------------------------------------------
static void test_wfg_remove_breaks_cycle() {
    std::cout << "\n[test_wfg_remove_breaks_cycle]\n";
    WaitForGraph wfg;
    wfg.addEdge(1, 2);
    wfg.addEdge(2, 1);

    ASSERT(wfg.detectCycle() != -1, "Cycle exists before removal");

    wfg.removeEdgesForTxn(2); // T2 aborted / released
    ASSERT(wfg.detectCycle() == -1, "No cycle after removing T2's edges");
}

// ---------------------------------------------------------------------------
// Test: WaitForGraph – highest ID chosen as victim
// ---------------------------------------------------------------------------
static void test_wfg_victim_is_highest_id() {
    std::cout << "\n[test_wfg_victim_is_highest_id]\n";
    WaitForGraph wfg;
    wfg.addEdge(3, 7); // T3 waits for T7
    wfg.addEdge(7, 3); // T7 waits for T3 → cycle

    int victim = wfg.detectCycle();
    ASSERT(victim == 7, "Victim is the highest ID node in the cycle");
}

// ---------------------------------------------------------------------------
// Test: TransactionManager detects deadlock (T1 ↔ T2)
// ---------------------------------------------------------------------------
static void test_tm_deadlock_two_transactions() {
    std::cout << "\n[test_tm_deadlock_two_transactions]\n";
    TransactionManager tm;
    tm.getDataStore().write("X", "x0");
    tm.getDataStore().write("Y", "y0");

    int t1 = tm.beginTransaction();
    int t2 = tm.beginTransaction();

    // T1 locks X, T2 locks Y
    tm.write(t1, "X", "x1");
    tm.write(t2, "Y", "y2");

    // T1 waits for Y (held by T2) → WFG edge T1→T2
    OperationResult r1 = tm.write(t1, "Y", "y1");
    ASSERT(r1 == OperationResult::WAITING, "T1 waits for Y");

    // T2 waits for X (held by T1) → WFG edge T2→T1 → cycle!
    OperationResult r2 = tm.write(t2, "X", "x2");
    ASSERT(r2 == OperationResult::WAITING, "T2 waits for X");

    int victim = tm.detectAndResolveDeadlock();
    ASSERT(victim != -1, "Deadlock is detected");
    ASSERT(victim == t1 || victim == t2, "Victim is one of the involved transactions");

    // Victim must be ABORTED
    ASSERT(tm.getTransaction(victim)->isAborted(), "Victim transaction is ABORTED");

    // Non-victim must still be BLOCKED (waiting); after abort it will get the lock
    int survivor = (victim == t1) ? t2 : t1;
    // Survivor can now proceed
    tm.getTransaction(survivor)->setState(TransactionState::ACTIVE);
    OperationResult retry = tm.write(survivor, (victim == t1) ? "X" : "Y",
                                     "survivor-val");
    ASSERT(retry == OperationResult::SUCCESS,
           "Survivor acquires lock after deadlock resolution");

    tm.commit(survivor);
}

// ---------------------------------------------------------------------------
// Test: no deadlock detection when no cycle present
// ---------------------------------------------------------------------------
static void test_tm_no_deadlock() {
    std::cout << "\n[test_tm_no_deadlock]\n";
    TransactionManager tm;
    tm.getDataStore().write("X", "x0");

    int t1 = tm.beginTransaction();
    int t2 = tm.beginTransaction();

    // T1 locks X, T2 waits (but T2 does NOT hold anything T1 wants)
    tm.write(t1, "X", "t1-X");
    tm.write(t2, "X", "t2-X"); // T2 waits

    // No cycle: T2→T1 but T1 is not waiting for anything
    int victim = tm.detectAndResolveDeadlock();
    ASSERT(victim == -1, "No deadlock when no cycle exists");

    tm.commit(t1);
    // T2 can now proceed
    tm.getTransaction(t2)->setState(TransactionState::ACTIVE);
    tm.write(t2, "X", "t2-X");
    tm.commit(t2);
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int main() {
    std::cout << "=== test_deadlock ===\n";

    test_wfg_no_cycle_empty();
    test_wfg_no_cycle_chain();
    test_wfg_simple_cycle();
    test_wfg_three_node_cycle();
    test_wfg_remove_breaks_cycle();
    test_wfg_victim_is_highest_id();
    test_tm_deadlock_two_transactions();
    test_tm_no_deadlock();

    std::cout << "\nResults: " << gPassed << " passed, "
              << gFailed << " failed\n";
    return (gFailed == 0) ? 0 : 1;
}
