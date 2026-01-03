param(
    [switch]$SkipClean
)
$ErrorActionPreference = "Stop"
$msbuild = "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
$slnFile = Join-Path (Split-Path -Parent $PSScriptRoot) "HspppLib.slnx"
$configurations = @(
    @{Config="Debug"; Platform="x86"},
    @{Config="Debug"; Platform="x64"},
    @{Config="Release"; Platform="x86"},
    @{Config="Release"; Platform="x64"}
)
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "HspppLib Build All Configurations" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""
if (-not $SkipClean) {
    Write-Host "Cleaning previous builds..." -ForegroundColor Yellow
    foreach ($cfg in $configurations) {
        & $msbuild $slnFile /t:Clean /p:Configuration=$($cfg.Config) /p:Platform=$($cfg.Platform) /nologo /verbosity:quiet
    }
    Write-Host "Done" -ForegroundColor Green
    Write-Host ""
}
$succeeded = 0
$failed = 0
foreach ($cfg in $configurations) {
    $platform = $cfg.Platform
    $config = $cfg.Config
    Write-Host "Building $platform - $config..." -ForegroundColor Cyan
    & $msbuild $slnFile /p:Configuration=$config /p:Platform=$platform /m /verbosity:minimal /nologo
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  [OK] $platform - $config" -ForegroundColor Green
        $succeeded++
    } else {
        Write-Host "  [FAILED] $platform - $config" -ForegroundColor Red
        $failed++
    }
    Write-Host ""
}
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "Build Summary" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "Succeeded: $succeeded / 4" -ForegroundColor Green
if ($failed -gt 0) {
    Write-Host "Failed: $failed / 4" -ForegroundColor Red
    exit 1
}
Write-Host ""
Write-Host "All configurations built successfully!" -ForegroundColor Green
