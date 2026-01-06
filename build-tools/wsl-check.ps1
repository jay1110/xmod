# WSL2 Detection Script for xmod Build
# Checks if WSL2 is available and can run Linux builds

$ErrorActionPreference = "Stop"

Write-Host "=== WSL2 Detection ===" -ForegroundColor Cyan
Write-Host ""

# Check if WSL is installed
try {
    $wslVersion = wsl --version 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "WSL is not installed or not accessible." -ForegroundColor Yellow
        Write-Host ""
        Write-Host "To install WSL2:" -ForegroundColor White
        Write-Host "  1. Run PowerShell as Administrator" -ForegroundColor Gray
        Write-Host "  2. Execute: wsl --install" -ForegroundColor Gray
        Write-Host "  3. Restart your computer" -ForegroundColor Gray
        Write-Host "  4. Set up Ubuntu (default distribution)" -ForegroundColor Gray
        Write-Host ""
        return $false
    }
} catch {
    Write-Host "WSL is not installed." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "To install WSL2:" -ForegroundColor White
    Write-Host "  1. Run PowerShell as Administrator" -ForegroundColor Gray
    Write-Host "  2. Execute: wsl --install" -ForegroundColor Gray
    Write-Host "  3. Restart your computer" -ForegroundColor Gray
    Write-Host ""
    return $false
}

Write-Host "WSL is installed." -ForegroundColor Green

# Check if any Linux distributions are installed
try {
    $distributions = wsl --list --quiet 2>&1
    if ($distributions.Count -eq 0 -or $distributions -match "no installed distributions") {
        Write-Host "No Linux distributions are installed." -ForegroundColor Yellow
        Write-Host ""
        Write-Host "To install Ubuntu:" -ForegroundColor White
        Write-Host "  wsl --install -d Ubuntu" -ForegroundColor Gray
        Write-Host ""
        return $false
    }
} catch {
    Write-Host "Could not list WSL distributions." -ForegroundColor Yellow
    return $false
}

Write-Host "Linux distributions found:" -ForegroundColor Green
wsl --list --verbose

# Check if WSL2 is being used
$wsl2Found = $false
try {
    $distList = wsl --list --verbose
    if ($distList -match "2") {
        $wsl2Found = $true
    }
} catch {
    # Fallback check
}

if (-not $wsl2Found) {
    Write-Host ""
    Write-Host "WARNING: No WSL2 distributions found. WSL1 may not work correctly." -ForegroundColor Yellow
    Write-Host "To convert to WSL2:" -ForegroundColor White
    Write-Host "  wsl --set-version <DistroName> 2" -ForegroundColor Gray
    Write-Host ""
}

# Test if we can run a simple command
Write-Host ""
Write-Host "Testing WSL execution..." -ForegroundColor Cyan
try {
    $testResult = wsl echo "WSL is working"
    if ($testResult -eq "WSL is working") {
        Write-Host "WSL execution test: SUCCESS" -ForegroundColor Green
        Write-Host ""
        return $true
    } else {
        Write-Host "WSL execution test: FAILED" -ForegroundColor Red
        Write-Host ""
        return $false
    }
} catch {
    Write-Host "WSL execution test: FAILED" -ForegroundColor Red
    Write-Host "Error: $_" -ForegroundColor Red
    Write-Host ""
    return $false
}
