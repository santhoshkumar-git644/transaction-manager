#ifndef LOCK_MANAGER_H
#define LOCK_MANAGER_H

#include <map>
#include <set>
#include <memory>
#include <cstdint>
#include "../utils/enums.h"
#include "Lock.h"

class LockManager {
public:
    LockManager();
    ~LockManager();
    
    bool requestLock(uint32_t transaction_id, uint32_t resource_id, LockType lock_type);
    bool releaseLock(uint32_t transaction_id, uint32_t resource_id);
    
    bool hasLock(uint32_t transaction_id, uint32_t resource_id) const;
    LockType getLockType(uint32_t resource_id) const;
    
    // TODO: Implement lock compatibility matrix
    
private:
    std::map<uint32_t, std::set<std::shared_ptr<Lock>>> resource_locks_;
    bool isCompatible(LockType existing, LockType requested) const;
};

#endif // LOCK_MANAGER_H
