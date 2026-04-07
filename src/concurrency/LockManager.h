#pragma once

#include "Lock.h"
#include "WaitForGraph.h"
#include "../utils/enums.h"

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

// Manages all lock acquisition and release for the Strict Two-Phase Locking
// (2PL) protocol.
//
// Lock table layout:
//   resource (key) -> list<Lock>
//      The list contains both *granted* and *waiting* (queued) lock entries.
//      Granted entries always come before waiting entries so that a linear
//      scan for compatibility is straightforward.
//
// Strict 2PL invariant: locks are held until the owning transaction commits
// or aborts.  releaseLocks() must only be called at that point.
class LockManager {
public:
    // Request a lock on 'resource' for 'txnId' with the given mode.
    //   GRANTED  – lock was immediately granted.
    //   WAITING  – a conflict exists; the request is queued and the
    //              Wait-For Graph is updated accordingly.
    //   DENIED   – txnId already aborted or an internal error occurred.
    LockStatus acquireLock(int txnId, const std::string& resource, LockMode mode);

    // Release ALL locks (granted and waiting) held or queued by txnId.
    // After removing the locks the manager attempts to grant any waiting
    // requests that are now satisfiable.
    void releaseLocks(int txnId);

    // True if txnId currently holds a granted lock of 'mode' (or stronger)
    // on 'resource'.
    bool hasLock(int txnId, const std::string& resource, LockMode mode) const;

    // Returns the set of txnIds that 'txnId' is currently waiting for
    // (i.e. outgoing edges in the Wait-For Graph).
    std::vector<int> getWaiters(int txnId) const;

    // Provide access to the Wait-For Graph so the TransactionManager can
    // run cycle detection without coupling to LockManager internals.
    WaitForGraph& getWaitForGraph();

private:
    std::unordered_map<std::string, std::list<Lock>> lockTable_;
    WaitForGraph wfg_;

    // True when the two lock modes are mutually compatible.
    // Only S+S is compatible; any combination involving X is not.
    bool isCompatible(LockMode held, LockMode requested) const;

    // Walk the waiting queue for 'resource' and grant every request that is
    // now compatible with the currently granted locks.
    void tryGrantWaiting(const std::string& resource);
};
