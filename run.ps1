param(
    [string]$InputFile = ""
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path "build/transaction_manager.exe")) {
    Write-Host "Executable missing. Building first..."
    .\build.ps1
}

if ($InputFile -and (Test-Path $InputFile)) {
    Get-Content $InputFile | .\build\transaction_manager.exe
} else {
    .\build\transaction_manager.exe
}
