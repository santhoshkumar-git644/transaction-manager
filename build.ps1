$ErrorActionPreference = "Stop"

$sources = @(
    "src/main.cpp",
    "src/transaction/Transaction.cpp",
    "src/transaction/TransactionManager.cpp",
    "src/concurrency/Lock.cpp",
    "src/concurrency/LockManager.cpp",
    "src/concurrency/WaitForGraph.cpp",
    "src/recovery/LogRecord.cpp",
    "src/recovery/LogManager.cpp",
    "src/recovery/RecoveryManager.cpp",
    "src/storage/Record.cpp",
    "src/storage/DataStore.cpp"
)

if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

& g++ -std=c++17 -Iinclude $sources -o build/transaction_manager.exe
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host "Build successful: build/transaction_manager.exe"
