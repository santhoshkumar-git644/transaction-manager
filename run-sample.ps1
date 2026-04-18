$ErrorActionPreference = "Stop"

if (-not (Test-Path "build/transaction_manager.exe")) {
    .\build.ps1
}

@'
BEGIN T1
READ T1 A
WRITE T1 A 50
BEGIN T2
READ T2 A
WRITE T2 A 100
COMMIT T1
COMMIT T2
'@ | .\build\transaction_manager.exe
