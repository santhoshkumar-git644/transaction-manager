#include <cassert>
#include <iostream>
#include "../include/concurrency/WaitForGraph.h"
#include "../include/concurrency/LockManager.h"

int main() {
    std::cout << "Testing Deadlock Detection and Lock Compatibility\n";

    {
        WaitForGraph graph;

        graph.addEdge(1, 2);
        graph.addEdge(2, 3);
        assert(!graph.hasCycle());

        graph.addEdge(3, 1);
        assert(graph.hasCycle());
    }

    {
        LockManager lock_mgr;
        const uint32_t resource = 42;

        // Shared + shared is compatible.
        assert(lock_mgr.requestLock(1, resource, LockType::SHARED));
        assert(lock_mgr.requestLock(2, resource, LockType::SHARED));
        assert(lock_mgr.hasLock(1, resource));
        assert(lock_mgr.hasLock(2, resource));

        // Exclusive conflicts and should wait.
        assert(!lock_mgr.requestLock(3, resource, LockType::EXCLUSIVE));
        assert(!lock_mgr.hasLock(3, resource));

        // Strict 2PL: locks are released only at transaction completion.
        assert(lock_mgr.completeTransaction(1));
        assert(!lock_mgr.hasLock(3, resource));
        assert(lock_mgr.completeTransaction(2));
        assert(lock_mgr.hasLock(3, resource));
        assert(lock_mgr.getLockType(resource) == LockType::EXCLUSIVE);
    }

    std::cout << "All deadlock and lock manager checks passed\n";
    return 0;
}
