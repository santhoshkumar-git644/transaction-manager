CXX := g++
CXXFLAGS := -std=c++17 -Iinclude
TARGET := build/transaction_manager.exe

SOURCES := \
	src/main.cpp \
	src/transaction/Transaction.cpp \
	src/transaction/TransactionManager.cpp \
	src/concurrency/Lock.cpp \
	src/concurrency/LockManager.cpp \
	src/concurrency/WaitForGraph.cpp \
	src/recovery/LogRecord.cpp \
	src/recovery/LogManager.cpp \
	src/recovery/RecoveryManager.cpp \
	src/storage/Record.cpp \
	src/storage/DataStore.cpp

.PHONY: all build run sample clean

all: build

build: $(TARGET)

$(TARGET): $(SOURCES)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: build
	./$(TARGET)

sample: build
	printf "BEGIN T1\nREAD T1 A\nWRITE T1 A 50\nBEGIN T2\nREAD T2 A\nWRITE T2 A 100\nCOMMIT T1\nCOMMIT T2\n" | ./$(TARGET)

clean:
	rm -f $(TARGET)
