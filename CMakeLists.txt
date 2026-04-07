cmake_minimum_required(VERSION 3.14)
project(TransactionManager CXX)

# -----------------------------------------------------------------------
# Global settings
# -----------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Treat warnings as errors in CI; developers can override with -DSTRICT=OFF
option(STRICT "Enable -Werror" ON)

if(MSVC)
    set(WARN_FLAGS /W4)
    if(STRICT)
        list(APPEND WARN_FLAGS /WX)
    endif()
else()
    set(WARN_FLAGS -Wall -Wextra -Wpedantic)
    if(STRICT)
        list(APPEND WARN_FLAGS -Werror)
    endif()
endif()

# -----------------------------------------------------------------------
# Shared library of all modules (avoids recompiling for every target)
# -----------------------------------------------------------------------
set(LIB_SOURCES
    src/storage/DataStore.cpp
    src/concurrency/WaitForGraph.cpp
    src/concurrency/LockManager.cpp
    src/recovery/LogManager.cpp
    src/recovery/RecoveryManager.cpp
    src/transaction/Transaction.cpp
    src/transaction/TransactionManager.cpp
)

add_library(tm_core STATIC ${LIB_SOURCES})
target_include_directories(tm_core PUBLIC src)
target_compile_options(tm_core PRIVATE ${WARN_FLAGS})

# -----------------------------------------------------------------------
# Main demonstration executable
# -----------------------------------------------------------------------
add_executable(transaction_manager src/main.cpp)
target_link_libraries(transaction_manager PRIVATE tm_core)
target_compile_options(transaction_manager PRIVATE ${WARN_FLAGS})

# -----------------------------------------------------------------------
# Test executables
# -----------------------------------------------------------------------
enable_testing()

foreach(TEST_NAME test_transactions test_deadlock test_recovery)
    add_executable(${TEST_NAME} tests/${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} PRIVATE tm_core)
    target_compile_options(${TEST_NAME} PRIVATE ${WARN_FLAGS})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
