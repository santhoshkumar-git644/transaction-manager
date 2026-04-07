#include "LockManager.h"

#include <algorithm>
#include <iostream>

// ---------------------------------------------------------------------------
// Compatibility
// ---------------------------------------------------------------------------

bool LockManager::isCompatible(LockMode held, LockMode requested) const {
    // Only Shared + Shared is compatible; anything with Exclusive is not
    return held == LockMode::SHARED && requested == LockMode::SHARED;
}

// ---------------------------------------------------------------------------
// Lock acquisition
// ---------------------------------------------------------------------------

LockStatus LockManager::acquireLock(int txnId,
                                    const std::string& resource,
                                    LockMode mode) {
    auto& locks = lockTable_[resource];

    // -----------------------------------------------------------------------
    // Case 1: The transaction already holds a granted lock on this resource
    // -----------------------------------------------------------------------
    for (auto& lock : locks) {
        if (lock.txnId == txnId && lock.granted) {
            if (lock.mode == LockMode::EXCLUSIVE) {
                return LockStatus::GRANTED; // Exclusive covers everything
            }
            if (mode == LockMode::SHARED) {
                return LockStatus::GRANTED; // S → S: already satisfied
            }
            // S → X upgrade: allow only when no OTHER transaction holds a lock
            bool onlyHolder = true;
            for (const auto& other : locks) {
                if (other.granted && other.txnId != txnId) {
                    onlyHolder = false;
                    break;
                }
            }
            if (onlyHolder) {
                lock.mode = LockMode::EXCLUSIVE;
                return LockStatus::GRANTED;
            }
            // Upgrade blocked – fall through to the conflict/wait logic
        }
    }

    // -----------------------------------------------------------------------
    // Case 2: Check compatibility against all currently *granted* locks
    // -----------------------------------------------------------------------
    std::vector<int> conflictingTxns;
    for (const auto& lock : locks) {
        if (lock.granted && lock.txnId != txnId) {
            if (!isCompatible(lock.mode, mode)) {
                conflictingTxns.push_back(lock.txnId);
            }
        }
    }

    if (conflictingTxns.empty()) {
        // No conflict: grant immediately
        locks.emplace_back(txnId, resource, mode, /*granted=*/true);
        return LockStatus::GRANTED;
    }

    // -----------------------------------------------------------------------
    // Case 3: Conflict – queue the request and update the Wait-For Graph
    // -----------------------------------------------------------------------
    locks.emplace_back(txnId, resource, mode, /*granted=*/false);

    for (int conflictTxn : conflictingTxns) {
        wfg_.addEdge(txnId, conflictTxn);
    }

    return LockStatus::WAITING;
}

// ---------------------------------------------------------------------------
// Lock release (called at commit or abort)
// ---------------------------------------------------------------------------

void LockManager::releaseLocks(int txnId) {
    // Identify resources that have any entry for txnId
    std::vector<std::string> affected;
    for (const auto& [resource, locks] : lockTable_) {
        for (const auto& lock : locks) {
            if (lock.txnId == txnId) {
                affected.push_back(resource);
                break;
            }
        }
    }

    for (const auto& resource : affected) {
        auto& locks = lockTable_[resource];

        // Remove ALL entries (granted or waiting) belonging to txnId
        locks.erase(
            std::remove_if(locks.begin(), locks.end(),
                           [txnId](const Lock& l) { return l.txnId == txnId; }),
            locks.end());

        // Attempt to promote waiting requests now that the conflict may have cleared
        tryGrantWaiting(resource);
    }

    // Remove all Wait-For Graph edges involving this transaction
    wfg_.removeEdgesForTxn(txnId);
}

// ---------------------------------------------------------------------------
// Grant waiting requests (called after a transaction releases its locks)
// ---------------------------------------------------------------------------

void LockManager::tryGrantWaiting(const std::string& resource) {
    auto& locks = lockTable_[resource];

    for (auto& candidate : locks) {
        if (candidate.granted) {
            continue; // Skip already-granted entries
        }

        // Check compatibility with every currently granted lock
        bool compatible = true;
        for (const auto& other : locks) {
            if (other.granted && other.txnId != candidate.txnId) {
                if (!isCompatible(other.mode, candidate.mode)) {
                    compatible = false;
                    break;
                }
            }
        }

        if (compatible) {
            candidate.granted = true;
            // The transaction is no longer waiting → remove its outgoing WFG edges
            wfg_.removeOutgoingEdges(candidate.txnId);
        }
    }
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool LockManager::hasLock(int txnId,
                           const std::string& resource,
                           LockMode mode) const {
    auto it = lockTable_.find(resource);
    if (it == lockTable_.end()) {
        return false;
    }
    for (const auto& lock : it->second) {
        if (lock.txnId == txnId && lock.granted) {
            // Exclusive covers Shared
            if (lock.mode == LockMode::EXCLUSIVE) return true;
            if (lock.mode == mode)                 return true;
        }
    }
    return false;
}

std::vector<int> LockManager::getWaiters(int txnId) const {
    // Return the outgoing neighbours of txnId in the WFG (read-only access
    // via detectCycle is sufficient for most callers, but expose raw waiters
    // for diagnostic / test purposes).
    std::vector<int> result;
    for (const auto& [resource, locks] : lockTable_) {
        for (const auto& lock : locks) {
            if (!lock.granted && lock.txnId == txnId) {
                // This transaction is waiting on this resource;
                // find the current holder(s)
                for (const auto& other : locks) {
                    if (other.granted && other.txnId != txnId) {
                        result.push_back(other.txnId);
                    }
                }
            }
        }
    }
    return result;
}

WaitForGraph& LockManager::getWaitForGraph() {
    return wfg_;
}
