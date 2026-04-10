#include "../../include/concurrency/Lock.h"

Lock::Lock(uint32_t transaction_id, LockType lock_type)
    : transaction_id_(transaction_id), lock_type_(lock_type) {
}

Lock::~Lock() {
}

uint32_t Lock::getTransactionId() const {
    return transaction_id_;
}

LockType Lock::getLockType() const {
    return lock_type_;
}
