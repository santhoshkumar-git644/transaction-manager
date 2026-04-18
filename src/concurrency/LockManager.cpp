#include "../../include/concurrency/LockManager.h"

LockManager::LockManager() {
}

LockManager::~LockManager() {
}

bool LockManager::requestLock(uint32_t transaction_id, uint32_t resource_id, LockType lock_type) {
    if (completed_transactions_.find(transaction_id) != completed_transactions_.end()) {
        return false;
    }

    auto& locks = resource_locks_[resource_id];

    std::shared_ptr<Lock> existing_lock;
    for (const auto& lock : locks) {
        if (lock->getTransactionId() == transaction_id) {
            existing_lock = lock;
            break;
        }
    }

    if (existing_lock != nullptr) {
        const LockType held_type = existing_lock->getLockType();
        if (held_type == lock_type || held_type == LockType::EXCLUSIVE) {
            return true;
        }

        bool upgrade_compatible = true;
        for (const auto& lock : locks) {
            if (lock->getTransactionId() == transaction_id) {
                continue;
            }
            if (!isCompatible(lock->getLockType(), lock_type) || !isCompatible(lock_type, lock->getLockType())) {
                upgrade_compatible = false;
                break;
            }
        }

        if (upgrade_compatible) {
            locks.erase(existing_lock);
            locks.insert(std::make_shared<Lock>(transaction_id, lock_type));
            txn_locks_[transaction_id].insert(resource_id);
            return true;
        }
    } else {
        bool compatible = true;
        for (const auto& lock : locks) {
            if (!isCompatible(lock->getLockType(), lock_type) || !isCompatible(lock_type, lock->getLockType())) {
                compatible = false;
                break;
            }
        }

        if (compatible) {
            locks.insert(std::make_shared<Lock>(transaction_id, lock_type));
            txn_locks_[transaction_id].insert(resource_id);
            return true;
        }
    }

    auto& queue = wait_queues_[resource_id];
    bool already_waiting = false;
    for (const auto& request : queue) {
        if (request.transaction_id == transaction_id) {
            already_waiting = true;
            break;
        }
    }

    if (!already_waiting) {
        queue.push_back({transaction_id, lock_type});
    }

    return false;
}

bool LockManager::releaseLock(uint32_t transaction_id, uint32_t resource_id) {
    // Strict 2PL: do not allow lock release while transaction is active.
    if (completed_transactions_.find(transaction_id) == completed_transactions_.end()) {
        return false;
    }

    auto it = resource_locks_.find(resource_id);
    if (it != resource_locks_.end()) {
        auto& locks = it->second;
        bool removed = false;

        // Remove locks held by this transaction
        for (auto lock_it = locks.begin(); lock_it != locks.end(); ) {
            if ((*lock_it)->getTransactionId() == transaction_id) {
                lock_it = locks.erase(lock_it);
                removed = true;
            } else {
                ++lock_it;
            }
        }

        if (removed) {
            if (txn_locks_.find(transaction_id) != txn_locks_.end()) {
                txn_locks_[transaction_id].erase(resource_id);
                if (txn_locks_[transaction_id].empty()) {
                    txn_locks_.erase(transaction_id);
                }
            }

            processWaitQueue(resource_id);

            if (resource_locks_[resource_id].empty()) {
                resource_locks_.erase(resource_id);
            }
            if (wait_queues_[resource_id].empty()) {
                wait_queues_.erase(resource_id);
            }
            return true;
        }
    }
    return false;
}

bool LockManager::completeTransaction(uint32_t transaction_id) {
    completed_transactions_.insert(transaction_id);

    auto held_it = txn_locks_.find(transaction_id);
    if (held_it == txn_locks_.end()) {
        return true;
    }

    const auto held_resources = held_it->second;
    bool released_any = false;
    for (uint32_t resource_id : held_resources) {
        if (releaseLock(transaction_id, resource_id)) {
            released_any = true;
        }
    }

    txn_locks_.erase(transaction_id);
    return released_any;
}

bool LockManager::hasLock(uint32_t transaction_id, uint32_t resource_id) const {
    auto it = resource_locks_.find(resource_id);
    if (it != resource_locks_.end()) {
        for (const auto& lock : it->second) {
            if (lock->getTransactionId() == transaction_id) {
                return true;
            }
        }
    }
    return false;
}

LockType LockManager::getLockType(uint32_t resource_id) const {
    auto it = resource_locks_.find(resource_id);
    if (it == resource_locks_.end() || it->second.empty()) {
        return LockType::NONE;
    }

    bool has_ix = false;
    bool has_is = false;
    for (const auto& lock : it->second) {
        if (lock->getLockType() == LockType::EXCLUSIVE) {
            return LockType::EXCLUSIVE;
        }
        if (lock->getLockType() == LockType::SHARED) {
            return LockType::SHARED;
        }
        if (lock->getLockType() == LockType::INTENTION_EXCLUSIVE) {
            has_ix = true;
        }
        if (lock->getLockType() == LockType::INTENTION_SHARED) {
            has_is = true;
        }
    }

    if (has_ix) {
        return LockType::INTENTION_EXCLUSIVE;
    }
    if (has_is) {
        return LockType::INTENTION_SHARED;
    }
    return LockType::NONE;
}

bool LockManager::isCompatible(LockType existing, LockType requested) const {
    if (existing == LockType::EXCLUSIVE || requested == LockType::EXCLUSIVE) {
        return false;
    }

    if (existing == LockType::SHARED && requested == LockType::INTENTION_EXCLUSIVE) {
        return false;
    }
    if (existing == LockType::INTENTION_EXCLUSIVE && requested == LockType::SHARED) {
        return false;
    }

    return true;
}

void LockManager::processWaitQueue(uint32_t resource_id) {
    auto queue_it = wait_queues_.find(resource_id);
    if (queue_it == wait_queues_.end()) {
        return;
    }

    auto& queue = queue_it->second;
    auto& locks = resource_locks_[resource_id];

    while (!queue.empty()) {
        const LockRequest request = queue.front();

        bool compatible = true;
        for (const auto& lock : locks) {
            if (!isCompatible(lock->getLockType(), request.lock_type) ||
                !isCompatible(request.lock_type, lock->getLockType())) {
                compatible = false;
                break;
            }
        }

        if (!compatible) {
            break;
        }

        locks.insert(std::make_shared<Lock>(request.transaction_id, request.lock_type));
        txn_locks_[request.transaction_id].insert(resource_id);
        queue.pop_front();
    }
}
