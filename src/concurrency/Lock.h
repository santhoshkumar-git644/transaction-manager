#pragma once

#include "../utils/enums.h"

#include <string>

// Represents a single lock entry (either granted or waiting) in the lock table
struct Lock {
    int      txnId;     // Transaction that owns/requested this lock
    std::string resource; // Resource (key) being locked
    LockMode mode;      // SHARED or EXCLUSIVE
    bool     granted;   // true  = lock is currently held
                        // false = lock is queued (waiting for a conflict to clear)

    Lock(int txnId, const std::string& resource, LockMode mode, bool granted = false)
        : txnId(txnId), resource(resource), mode(mode), granted(granted) {}
};
