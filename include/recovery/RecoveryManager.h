#ifndef RECOVERY_MANAGER_H
#define RECOVERY_MANAGER_H

#include <memory>
#include <vector>
#include "LogManager.h"
#include "../utils/enums.h"

class RecoveryManager {
public:
    RecoveryManager(std::shared_ptr<LogManager> log_manager);
    ~RecoveryManager();
    
    void performRecovery();
    void redo();
    void undo();
    
    // TODO: Analysis phase, redo phase, undo phase
    
private:
    std::shared_ptr<LogManager> log_manager_;
    RecoveryState recovery_state_;
};

#endif // RECOVERY_MANAGER_H
