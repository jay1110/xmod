param(
    [string]$MsysRoot = 'C:/msys64',
    [string]$LlvmBin = "$env:USERPROFILE/emsdk/upstream/bin",
    [int]$Jobs = 8
)
& "$PSScriptRoot/build-linux-local.ps1" -Architecture i386 @PSBoundParameters
