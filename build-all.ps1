# Master Build Script for xmod - Visual Studio 2022 + WSL2
# Builds all platform variants and creates release package

param(
    [switch]$SkipWindows = $false,
    [switch]$SkipLinux = $false,
    [switch]$SkipPackage = $false
)

$ErrorActionPreference = "Stop"
$OriginalLocation = Get-Location

function Write-Status {
    param([string]$Message)
    Write-Host ""
    Write-Host "=== $Message ===" -ForegroundColor Cyan
    Write-Host ""
}

function Write-Success {
    param([string]$Message)
    Write-Host $Message -ForegroundColor Green
}

function Write-BuildError {
    param([string]$Message)
    Write-Host $Message -ForegroundColor Red
}

function Write-BuildWarning {
    param([string]$Message)
    Write-Host $Message -ForegroundColor Yellow
}

try {
    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Cyan
    Write-Host "          xmod Multi-Platform Build System                  " -ForegroundColor Cyan
    Write-Host "          Visual Studio 2022 + WSL2                         " -ForegroundColor Cyan
    Write-Host "============================================================" -ForegroundColor Cyan
    Write-Host ""

    # Get repository root
    $ScriptRoot = $PSScriptRoot
    if ([string]::IsNullOrEmpty($ScriptRoot)) {
        $ScriptRoot = Get-Location
    }
    Set-Location $ScriptRoot

    Write-Host "Repository root: $ScriptRoot" -ForegroundColor Gray
    Write-Host ""

    # Clean previous release
    if (Test-Path "release") {
        Write-Status "Cleaning previous release"
        Remove-Item -Path "release" -Recurse -Force
    }

    # Create build directories
    New-Item -ItemType Directory -Path "release/xmod" -Force | Out-Null

    # ========================================
    # PHASE 1: Windows Builds (Visual Studio)
    # ========================================
    
    if (-not $SkipWindows) {
        Write-Status "Phase 1: Windows Builds (Visual Studio 2022)"

        # Find Visual Studio 2022
        $vswherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
        
        if (-not (Test-Path $vswherePath)) {
            Write-BuildError "Visual Studio Installer (vswhere.exe) not found."
            Write-BuildWarning "Please install Visual Studio 2022 with C++ workload."
            throw "Visual Studio 2022 not found"
        }

        $vsPath = & $vswherePath -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
        
        if ([string]::IsNullOrEmpty($vsPath)) {
            Write-BuildError "Visual Studio 2022 installation not found."
            throw "Visual Studio 2022 not found"
        }

        $msbuildPath = Join-Path $vsPath "MSBuild\Current\Bin\MSBuild.exe"
        
        if (-not (Test-Path $msbuildPath)) {
            Write-BuildError "MSBuild not found at: $msbuildPath"
            throw "MSBuild not found"
        }

        Write-Success "Found Visual Studio 2022 at: $vsPath"
        Write-Success "Using MSBuild: $msbuildPath"
        Write-Host ""

        # Build Windows 32-bit
        Write-Host "Building Windows 32-bit (Win32)..." -ForegroundColor White
        & $msbuildPath "xmod.sln" /p:Configuration=Release /p:Platform=Win32 /m /v:minimal
        
        if ($LASTEXITCODE -ne 0) {
            throw "Windows 32-bit build failed"
        }
        
        Write-Success "[OK] Windows 32-bit build completed"

        # Build Windows 64-bit
        Write-Host ""
        Write-Host "Building Windows 64-bit (x64)..." -ForegroundColor White
        & $msbuildPath "xmod.sln" /p:Configuration=Release /p:Platform=x64 /m /v:minimal
        
        if ($LASTEXITCODE -ne 0) {
            throw "Windows 64-bit build failed"
        }
        
        Write-Success "[OK] Windows 64-bit build completed"
        Write-Host ""

        # Verify Windows builds
        $win32Files = @(
            "build.vs2022-Win32-release\game\qagame_mp_x86.dll",
            "build.vs2022-Win32-release\cgame\cgame_mp_x86.dll",
            "build.vs2022-Win32-release\ui\ui_mp_x86.dll"
        )

        $win64Files = @(
            "build.vs2022-x64-release\game\qagame_mp_x64.dll",
            "build.vs2022-x64-release\cgame\cgame_mp_x64.dll",
            "build.vs2022-x64-release\ui\ui_mp_x64.dll"
        )

        Write-Host "Verifying Windows builds..." -ForegroundColor White
        foreach ($file in $win32Files + $win64Files) {
            if (-not (Test-Path $file)) {
                throw "Required file not found: $file"
            }
        }
        Write-Success "[OK] All Windows binaries verified"
    }
    else {
        Write-BuildWarning "Skipping Windows builds (-SkipWindows specified)"
    }

    # ========================================
    # PHASE 2: Linux Builds (WSL2)
    # ========================================
    
    if (-not $SkipLinux) {
        Write-Host ""
        Write-Status "Phase 2: Linux Builds (WSL2)"

        # Check WSL2
        $wslAvailable = & "$ScriptRoot\build-tools\wsl-check.ps1"
        
        if (-not $wslAvailable) {
            Write-BuildWarning "WSL2 is not available. Skipping Linux builds."
            Write-BuildWarning "The release package will only contain Windows binaries."
            Write-Host ""
            Write-Host "To enable Linux builds:" -ForegroundColor White
            Write-Host "  1. Install WSL2: wsl --install" -ForegroundColor Gray
            Write-Host "  2. Restart your computer" -ForegroundColor Gray
            Write-Host "  3. Re-run this build script" -ForegroundColor Gray
            Write-Host ""
        }
        else {
            Write-Success "WSL2 is available"
            Write-Host ""

            # Convert path to WSL format
            $wslPath = $ScriptRoot.Replace('\', '/').Replace('C:', '/mnt/c')
            
            Write-Host "Running Linux builds in WSL2..." -ForegroundColor White
            Write-Host "This may take several minutes..." -ForegroundColor Gray
            Write-Host ""

            # Run the Linux build script in WSL
            $wslCmd = "cd '$wslPath' && chmod +x build-tools/build-linux.sh && build-tools/build-linux.sh"
            wsl bash -c $wslCmd
            
            if ($LASTEXITCODE -ne 0) {
                throw "Linux builds failed in WSL2"
            }

            Write-Success "[OK] Linux builds completed"
            Write-Host ""

            # Verify Linux builds
            $linux32Files = @(
                "build.linux-release\game\qagame.mp.i386.so",
                "build.linux-release\cgame\cgame.mp.i386.so",
                "build.linux-release\ui\ui.mp.i386.so"
            )

            $linux64Files = @(
                "build.linux64-release\game\qagame.mp.x86_64.so",
                "build.linux64-release\cgame\cgame.mp.x86_64.so",
                "build.linux64-release\ui\ui.mp.x86_64.so"
            )

            Write-Host "Verifying Linux builds..." -ForegroundColor White
            $missingFiles = @()
            foreach ($file in $linux32Files + $linux64Files) {
                if (-not (Test-Path $file)) {
                    $missingFiles += $file
                }
            }

            if ($missingFiles.Count -gt 0) {
                Write-BuildWarning "Some Linux binaries were not found:"
                foreach ($file in $missingFiles) {
                    Write-Host "  - $file" -ForegroundColor Yellow
                }
            }
            else {
                Write-Success "[OK] All Linux binaries verified"
            }
        }
    }
    else {
        Write-BuildWarning "Skipping Linux builds (-SkipLinux specified)"
    }

    # ========================================
    # PHASE 3: Package Release
    # ========================================
    
    if (-not $SkipPackage) {
        Write-Host ""
        Write-Status "Phase 3: Creating Release Package"

        # Create temporary directory for pak contents
        $pakTemp = "release/pak-temp"
        New-Item -ItemType Directory -Path $pakTemp -Force | Out-Null

        # Copy pak data files
        Write-Host "Collecting pak data files..." -ForegroundColor White
        if (Test-Path "pak") {
            Get-ChildItem -Path "pak" -Recurse -File | ForEach-Object {
                $relativePath = $_.FullName.Substring((Get-Item "pak").FullName.Length + 1)
                $targetPath = Join-Path $pakTemp $relativePath
                $targetDir = Split-Path $targetPath -Parent
                
                if (-not (Test-Path $targetDir)) {
                    New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
                }
                
                Copy-Item $_.FullName -Destination $targetPath -Force
            }
            
            # Remove build definition files
            Remove-Item "$pakTemp\pak.defs" -ErrorAction SilentlyContinue
            Remove-Item "$pakTemp\pak.rules" -ErrorAction SilentlyContinue
            
            $pakFileCount = (Get-ChildItem -Path $pakTemp -Recurse -File).Count
            Write-Success "[OK] Collected $pakFileCount pak data files"
        }

        # Add client-side binaries to pak
        Write-Host "Adding client binaries to pak..." -ForegroundColor White
        
        # Windows 32-bit client files
        if (Test-Path "build.vs2022-Win32-release\cgame\cgame_mp_x86.dll") {
            Copy-Item "build.vs2022-Win32-release\cgame\cgame_mp_x86.dll" -Destination $pakTemp
        }
        if (Test-Path "build.vs2022-Win32-release\ui\ui_mp_x86.dll") {
            Copy-Item "build.vs2022-Win32-release\ui\ui_mp_x86.dll" -Destination $pakTemp
        }

        # Windows 64-bit client files
        if (Test-Path "build.vs2022-x64-release\cgame\cgame_mp_x64.dll") {
            Copy-Item "build.vs2022-x64-release\cgame\cgame_mp_x64.dll" -Destination $pakTemp
        }
        if (Test-Path "build.vs2022-x64-release\ui\ui_mp_x64.dll") {
            Copy-Item "build.vs2022-x64-release\ui\ui_mp_x64.dll" -Destination $pakTemp
        }

        # Linux 32-bit client files
        if (Test-Path "build.linux-release\cgame\cgame.mp.i386.so") {
            Copy-Item "build.linux-release\cgame\cgame.mp.i386.so" -Destination $pakTemp
        }
        if (Test-Path "build.linux-release\ui\ui.mp.i386.so") {
            Copy-Item "build.linux-release\ui\ui.mp.i386.so" -Destination $pakTemp
        }

        # Linux 64-bit client files
        if (Test-Path "build.linux64-release\cgame\cgame.mp.x86_64.so") {
            Copy-Item "build.linux64-release\cgame\cgame.mp.x86_64.so" -Destination $pakTemp
        }
        if (Test-Path "build.linux64-release\ui\ui.mp.x86_64.so") {
            Copy-Item "build.linux64-release\ui\ui.mp.x86_64.so" -Destination $pakTemp
        }

        $clientBinaries = (Get-ChildItem -Path $pakTemp -File -Include "*.dll","*.so").Count
        Write-Success "[OK] Added $clientBinaries client binaries"

        # Create .dat marker file
        Write-Host "Creating version marker..." -ForegroundColor White
        New-Item -Path "$pakTemp\xmod-2.0.0.dat" -ItemType File -Force | Out-Null
        Write-Success "[OK] Created xmod-2.0.0.dat"

        # Create pk3 file
        Write-Host "Creating pk3 archive..." -ForegroundColor White
        $pk3Path = "release\xmod\xmod-2.0.0.pk3"
        
        # Use PowerShell's Compress-Archive or system zip if available
        if (Get-Command "Compress-Archive" -ErrorAction SilentlyContinue) {
            # PowerShell's Compress-Archive creates .zip, so we create and rename
            $tempZip = "release\xmod\xmod-2.0.0.zip"
            Compress-Archive -Path "$pakTemp\*" -DestinationPath $tempZip -Force
            if (Test-Path $pk3Path) {
                Remove-Item $pk3Path -Force
            }
            Move-Item $tempZip $pk3Path
        }
        else {
            # Fallback: try to use WSL zip
            $wslPakPath = $pakTemp.Replace('\', '/').Replace('C:', '/mnt/c')
            $wslPk3Path = $pk3Path.Replace('\', '/').Replace('C:', '/mnt/c')
            $zipCmd = "cd '$wslPakPath' && zip -r '$wslPk3Path' *"
            wsl bash -c $zipCmd
        }

        if (Test-Path $pk3Path) {
            $pk3Size = (Get-Item $pk3Path).Length / 1MB
            $pk3SizeRounded = [math]::Round($pk3Size, 2)
            Write-Success "[OK] Created pk3 file ($pk3SizeRounded MB)"
        }
        else {
            throw "Failed to create pk3 file"
        }

        # Copy server binaries to release
        Write-Host "Copying server binaries..." -ForegroundColor White
        
        # Windows 32-bit server
        if (Test-Path "build.vs2022-Win32-release\game\qagame_mp_x86.dll") {
            Copy-Item "build.vs2022-Win32-release\game\qagame_mp_x86.dll" -Destination "release\xmod\"
        }

        # Windows 64-bit server
        if (Test-Path "build.vs2022-x64-release\game\qagame_mp_x64.dll") {
            Copy-Item "build.vs2022-x64-release\game\qagame_mp_x64.dll" -Destination "release\xmod\"
        }

        # Linux 32-bit server
        if (Test-Path "build.linux-release\game\qagame.mp.i386.so") {
            Copy-Item "build.linux-release\game\qagame.mp.i386.so" -Destination "release\xmod\"
        }

        # Linux 64-bit server
        if (Test-Path "build.linux64-release\game\qagame.mp.x86_64.so") {
            Copy-Item "build.linux64-release\game\qagame.mp.x86_64.so" -Destination "release\xmod\"
        }

        $serverBinaries = (Get-ChildItem -Path "release\xmod" -File -Include "qagame*").Count
        Write-Success "[OK] Copied $serverBinaries server binaries"

        # Copy config files
        Write-Host "Copying configuration files..." -ForegroundColor White
        
        # Process .m4 template files and copy as regular files
        if (Test-Path "pkg\README.txt.m4") {
            Copy-Item "pkg\README.txt.m4" -Destination "release\xmod\README.txt"
        }
        if (Test-Path "pkg\server.cfg.m4") {
            Copy-Item "pkg\server.cfg.m4" -Destination "release\xmod\server.cfg"
        }
        if (Test-Path "pkg\xmod.cfg.m4") {
            Copy-Item "pkg\xmod.cfg.m4" -Destination "release\xmod\xmod.cfg"
        }

        # Copy mapscripts
        if (Test-Path "pkg\mapscripts") {
            New-Item -ItemType Directory -Path "release\xmod\mapscripts" -Force | Out-Null
            Copy-Item "pkg\mapscripts\*" -Destination "release\xmod\mapscripts\" -Recurse -Force
        }

        # Copy linux scripts
        if (Test-Path "pkg\linux") {
            New-Item -ItemType Directory -Path "release\xmod\linux" -Force | Out-Null
            Get-ChildItem -Path "pkg\linux\*.m4" | ForEach-Object {
                $targetName = $_.Name -replace '\.m4$', ''
                Copy-Item $_.FullName -Destination "release\xmod\linux\$targetName"
            }
        }

        Write-Success "[OK] Configuration files copied"

        # Create final release ZIP
        Write-Host "Creating final release package..." -ForegroundColor White
        $releaseZip = "release\xmod-2.0.0.zip"
        
        if (Get-Command "Compress-Archive" -ErrorAction SilentlyContinue) {
            Compress-Archive -Path "release\xmod" -DestinationPath $releaseZip -Force
        }
        else {
            # Fallback to WSL
            $wslReleasePath = "release".Replace('\', '/').Replace('C:', '/mnt/c')
            $zipReleaseCmd = "cd '$wslReleasePath' && zip -r xmod-2.0.0.zip xmod/"
            wsl bash -c $zipReleaseCmd
        }

        if (Test-Path $releaseZip) {
            $zipSize = (Get-Item $releaseZip).Length / 1MB
            $zipSizeRounded = [math]::Round($zipSize, 2)
            Write-Success "[OK] Created release package ($zipSizeRounded MB)"
        }

        # Clean up temporary pak directory
        Remove-Item -Path $pakTemp -Recurse -Force

        # Show release contents
        Write-Host ""
        Write-Host "Release package contents:" -ForegroundColor White
        Get-ChildItem -Path "release\xmod" -Recurse | Select-Object -Property FullName, @{N='Size (KB)';E={[math]::Round($_.Length/1KB, 2)}} | Format-Table -AutoSize
    }
    else {
        Write-BuildWarning "Skipping package creation (-SkipPackage specified)"
    }

    # ========================================
    # FINAL SUMMARY
    # ========================================
    
    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host "              BUILD COMPLETED SUCCESSFULLY!                 " -ForegroundColor Green
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Release package: " -NoNewline
    Write-Host "release\xmod-2.0.0.zip" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "The release contains:" -ForegroundColor White
    Write-Host "  - xmod-2.0.0.pk3 (client binaries for all platforms + pak data)" -ForegroundColor Gray
    Write-Host "  - qagame server binaries for all platforms" -ForegroundColor Gray
    Write-Host "  - Configuration files and mapscripts" -ForegroundColor Gray
    Write-Host ""

}
catch {
    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Red
    Write-Host "                   BUILD FAILED!                            " -ForegroundColor Red
    Write-Host "============================================================" -ForegroundColor Red
    Write-Host ""
    Write-BuildError "Error: $_"
    Write-Host ""
    Write-Host "Stack trace:" -ForegroundColor Gray
    Write-Host $_.ScriptStackTrace -ForegroundColor Gray
    Write-Host ""
    exit 1
}
finally {
    Set-Location $OriginalLocation
}
