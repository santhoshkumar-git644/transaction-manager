# Test configuration

# Include the include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Test executables
add_executable(test_transactions test_transactions.cpp
    ../src/transaction/Transaction.cpp
    ../src/transaction/TransactionManager.cpp
)

add_executable(test_deadlock test_deadlock.cpp
    ../src/concurrency/Lock.cpp
    ../src/concurrency/WaitForGraph.cpp
)

add_executable(test_recovery test_recovery.cpp
    ../src/recovery/LogRecord.cpp
    ../src/recovery/LogManager.cpp
    ../src/recovery/RecoveryManager.cpp
)

# Register tests
add_test(NAME TransactionTests COMMAND test_transactions)
add_test(NAME DeadlockTests COMMAND test_deadlock)
add_test(NAME RecoveryTests COMMAND test_recovery)
