#include "../../include/concurrency/LockManager.h"

LockManager::LockManager() {
}

LockManager::~LockManager() {
}

bool LockManager::requestLock(uint32_t transaction_id, uint32_t resource_id, LockType lock_type) {
    // TODO: Implement lock request with compatibility checking
    auto lock = std::make_shared<Lock>(transaction_id, lock_type);
    resource_locks_[resource_id].insert(lock);
    return true;
}

bool LockManager::releaseLock(uint32_t transaction_id, uint32_t resource_id) {
    auto it = resource_locks_.find(resource_id);
    if (it != resource_locks_.end()) {
        auto& locks = it->second;
        // Remove locks held by this transaction
        for (auto lock_it = locks.begin(); lock_it != locks.end(); ) {
            if ((*lock_it)->getTransactionId() == transaction_id) {
                lock_it = locks.erase(lock_it);
                return true;
            } else {
                ++lock_it;
            }
        }
    }
    return false;
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
    if (it != resource_locks_.end() && !it->second.empty()) {
        return (*it->second.begin())->getLockType();
    }
    return LockType::SHARED;
}

bool LockManager::isCompatible(LockType existing, LockType requested) const {
    // TODO: Implement lock compatibility matrix
    return true;
}
