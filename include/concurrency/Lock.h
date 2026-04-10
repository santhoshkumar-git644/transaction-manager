#ifndef LOCK_H
#define LOCK_H

#include <cstdint>
#include "../utils/enums.h"

class Lock {
public:
    Lock(uint32_t transaction_id, LockType lock_type);
    ~Lock();
    
    uint32_t getTransactionId() const;
    LockType getLockType() const;
    
private:
    uint32_t transaction_id_;
    LockType lock_type_;
};

#endif // LOCK_H
