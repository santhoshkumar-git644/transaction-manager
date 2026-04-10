# Transaction Manager

A C++ implementation of a transaction manager for understanding operating systems and database concepts.

## Project Structure

```
transaction-manager/
├── src/
│   ├── main.cpp                          # Main entry point
│   ├── transaction/
│   │   ├── Transaction.h/cpp             # Individual transaction representation
│   │   └── TransactionManager.h/cpp      # Transaction lifecycle management
│   ├── concurrency/
│   │   ├── Lock.h/cpp                    # Lock representation
│   │   ├── LockManager.h/cpp             # Lock management & allocation
│   │   └── WaitForGraph.h/cpp            # Deadlock detection
│   ├── recovery/
│   │   ├── LogRecord.h/cpp               # Individual log record
│   │   ├── LogManager.h/cpp              # Log file management
│   │   └── RecoveryManager.h/cpp         # ARIES recovery algorithm
│   ├── storage/
│   │   ├── Record.h/cpp                  # Data record representation
│   │   └── DataStore.h/cpp               # In-memory data storage
│   └── utils/
│       ├── enums.h                       # Enum definitions
│       └── constants.h                   # Global constants
├── include/                              # Header files mirror src structure
├── test/
│   ├── test_transactions.cpp             # Transaction tests
│   ├── test_deadlock.cpp                 # Deadlock detection tests
│   └── test_recovery.cpp                 # Recovery mechanism tests
├── build/                                # Build output directory
├── docs/                                 # Documentation
├── CMakeLists.txt                        # CMake build configuration
└── README.md                             # This file
```

## Building the Project

### Prerequisites
- C++17 compatible compiler (MSVC, GCC, or Clang)
- CMake 3.10 or higher

### Build Steps

```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .

# Run main application
./transaction_manager

# Run tests
ctest
```

## Components Overview

### Transaction Management (`transaction/`)
- **Transaction**: Represents a single transaction with state tracking
- **TransactionManager**: Manages transaction lifecycle (begin, commit, abort)

### Concurrency Control (`concurrency/`)
- **Lock**: Represents a lock on a resource
- **LockManager**: Handles lock requests and compatibility checking
- **WaitForGraph**: Detects deadlocks using cycle detection

### Recovery (`recovery/`)
- **LogRecord**: Represents a single database operation log entry
- **LogManager**: Manages persistent transaction logs
- **RecoveryManager**: Implements ARIES recovery algorithm (Analysis, Redo, Undo)

### Storage (`storage/`)
- **Record**: In-memory representation of a data record
- **DataStore**: Simple in-memory data storage

### Utilities (`utils/`)
- **enums.h**: Transaction states, lock types, recovery states
- **constants.h**: Global configuration parameters

## Learning Goals

This project covers:
- **Transaction Processing**: ACID properties and transaction states
- **Concurrency Control**: Locking mechanisms and deadlock detection
- **Recovery Management**: Log-based recovery and ARIES algorithm
- **Database Architecture**: Buffer management and storage structures
- **Operating Systems**: Process synchronization and resource allocation

