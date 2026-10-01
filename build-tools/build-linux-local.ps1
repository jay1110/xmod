param(
    [ValidateSet('amd64', 'i386')]
    [string]$Architecture = 'amd64',
    [string]$MsysRoot = 'C:/msys64',
    [string]$LlvmBin = "$env:USERPROFILE/emsdk/upstream/bin",
    [ValidateRange(1, 128)]
    [int]$Jobs = 8
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$python = (Get-Command python.exe -ErrorAction Stop).Source
$bash = Join-Path $MsysRoot 'usr/bin/bash.exe'
foreach ($tool in @($bash, "$LlvmBin/clang++.exe", "$LlvmBin/lld.exe", "$LlvmBin/llvm-ar.exe")) {
    if (!(Test-Path -LiteralPath $tool)) { throw "Tool missing: $tool" }
}
$platform = if ($Architecture -eq 'amd64') { 'linux64' } else { 'linux' }
$crossDirectory = if ($Architecture -eq 'amd64') { 'linux-cross' } else { 'linux-cross-i386' }
$suffix = if ($Architecture -eq 'amd64') { '*.mp.x86_64.so' } else { '*.mp.i386.so' }
$outputName = if ($Architecture -eq 'amd64') { 'linux64' } else { 'linux32' }
Push-Location $repo
$saved = @{}
foreach ($name in @('XMOD_LLVM_BIN','XMOD_REPO','XMOD_CXX','XMOD_AR','XMOD_JOBS','XMOD_ZSTD','XMOD_LINUX_ARCH','XMOD_LINUX_PLATFORM')) {
    $saved[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
}
try {
    $env:XMOD_ZSTD = "$MsysRoot/usr/bin/zstd.exe"
    & $python "$PSScriptRoot/setup_linux_sysroot.py" --arch $Architecture
    if ($LASTEXITCODE) { throw 'Linux sysroot preparation failed' }
    Copy-Item -LiteralPath "$LlvmBin/lld.exe" -Destination "build/$crossDirectory/ld.lld.exe"
    $env:XMOD_LLVM_BIN = $LlvmBin
    $env:XMOD_REPO = $repo.Replace('\','/')
    $env:XMOD_CXX = '"' + $python.Replace('\','/') + '" "' + $PSScriptRoot.Replace('\','/') + '/linux_cross_cxx.py"'
    $env:XMOD_AR = '"' + $LlvmBin.Replace('\','/') + '/llvm-ar.exe"'
    $env:XMOD_JOBS = [string]$Jobs
    $env:XMOD_LINUX_ARCH = $Architecture
    $env:XMOD_LINUX_PLATFORM = $platform
    & $bash -lc 'cd "$XMOD_REPO" && make PLATFORM="$XMOD_LINUX_PLATFORM" VARIANT=release CXX="$XMOD_CXX" AR="$XMOD_AR" -j"$XMOD_JOBS" all'
    if ($LASTEXITCODE) { throw 'Linux build failed' }
    $output = "release/$outputName-2.0.4"
    New-Item -ItemType Directory -Force $output | Out-Null
    foreach ($module in @('cgame','game','ui')) {
        $files = @(Get-ChildItem "build.$platform-release/$module" -Filter $suffix)
        if ($files.Count -ne 1) { throw "Expected one built Linux module in $module, found $($files.Count)" }
        $files | Copy-Item -Destination $output
    }
    Get-ChildItem $output -Filter '*.so' | Get-FileHash -Algorithm SHA256 | ForEach-Object {
        $_.Hash.ToLower() + '  ' + [IO.Path]::GetFileName($_.Path)
    } | Set-Content "$output/SHA256SUMS.txt"
    Write-Host "Linux modules: $repo/$output"
} finally {
    foreach ($name in $saved.Keys) { [Environment]::SetEnvironmentVariable($name, $saved[$name], 'Process') }
    Pop-Location
}
