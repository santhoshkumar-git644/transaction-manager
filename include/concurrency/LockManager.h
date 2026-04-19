#ifndef LOCK_MANAGER_H
#define LOCK_MANAGER_H

#include <map>
#include <set>
#include <deque>
#include <memory>
#include <cstdint>
#include "../utils/enums.h"
#include "Lock.h"

class LockManager {
public:
    LockManager();
    ~LockManager();
    
    void setResourceParent(uint32_t child_resource_id, uint32_t parent_resource_id);
    bool requestLock(uint32_t transaction_id, uint32_t resource_id, LockType lock_type);
    bool releaseLock(uint32_t transaction_id, uint32_t resource_id);
    bool completeTransaction(uint32_t transaction_id);
    
    bool hasLock(uint32_t transaction_id, uint32_t resource_id) const;
    LockType getLockType(uint32_t resource_id) const;
    
private:
    struct LockRequest {
        uint32_t transaction_id;
        LockType lock_type;
    };

    std::map<uint32_t, std::set<std::shared_ptr<Lock>>> resource_locks_;
    std::map<uint32_t, std::deque<LockRequest>> wait_queues_;
    std::map<uint32_t, std::set<uint32_t>> txn_locks_;
    std::set<uint32_t> completed_transactions_;
    std::map<uint32_t, uint32_t> resource_parent_;

    bool requestLockOnResource(uint32_t transaction_id, uint32_t resource_id, LockType lock_type);
    bool ensureAncestorIntentionLocks(uint32_t transaction_id, uint32_t resource_id, LockType requested_type);
    LockType getAncestorIntentionType(LockType requested_type) const;
    bool isCompatible(LockType existing, LockType requested) const;
    void processWaitQueue(uint32_t resource_id);
};

#endif // LOCK_MANAGER_H
