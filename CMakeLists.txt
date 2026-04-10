cmake_minimum_required(VERSION 3.10)
project(TransactionManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/transaction/Transaction.cpp
    src/transaction/TransactionManager.cpp
    src/concurrency/Lock.cpp
    src/concurrency/LockManager.cpp
    src/concurrency/WaitForGraph.cpp
    src/recovery/LogRecord.cpp
    src/recovery/LogManager.cpp
    src/recovery/RecoveryManager.cpp
    src/storage/Record.cpp
    src/storage/DataStore.cpp
)

# Create executable
add_executable(transaction_manager ${SOURCES})

# Enable testing
enable_testing()
add_subdirectory(test)
