param([string]$OutputDir = "dist")
$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent $ScriptDir
$DistDir = Join-Path $RootDir $OutputDir
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "HspppLib Artifact Collection" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""
$configurations = @(@{Config="Debug"; Platform="x86"},@{Config="Debug"; Platform="x64"},@{Config="Release"; Platform="x86"},@{Config="Release"; Platform="x64"})
Write-Host "Creating output directory structure..." -ForegroundColor Yellow
if (Test-Path $DistDir) { Remove-Item -Path $DistDir -Recurse -Force }
New-Item -ItemType Directory -Path $DistDir -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $DistDir "lib") -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $DistDir "include\module") -Force | Out-Null
foreach ($conf in $configurations) { New-Item -ItemType Directory -Path (Join-Path $DistDir "lib\$($conf.Platform)\$($conf.Config)") -Force | Out-Null }
Write-Host "Done" -ForegroundColor Green
Write-Host ""
Write-Host "Copying library files..." -ForegroundColor Yellow
$libCopied = 0
$libMissing = 0
foreach ($conf in $configurations) {
    $platform = $conf.Platform
    $config = $conf.Config
    if ($platform -eq "x64") { $sourcePath = Join-Path $RootDir "x64\$config\HspppLib.lib" } else { $sourcePath = Join-Path $RootDir "$config\HspppLib.lib" }
    $destPath = Join-Path $DistDir "lib\$platform\$config\HspppLib.lib"
    if (Test-Path $sourcePath) {
        Copy-Item -Path $sourcePath -Destination $destPath -Force
        $fileSize = [math]::Round((Get-Item $sourcePath).Length / 1KB, 1)
        Write-Host "  [OK] $platform/$config : HspppLib.lib ($fileSize KB)" -ForegroundColor Green
        $libCopied++
    } else {
        Write-Host "  [NG] $platform/$config : HspppLib.lib (not found)" -ForegroundColor Red
        $libMissing++
    }
}
Write-Host ""
Write-Host "Copying module files..." -ForegroundColor Yellow
$moduleSourceDir = Join-Path $RootDir "HspppLib\module"
$moduleDestDir = Join-Path $DistDir "include\module"
if (Test-Path $moduleSourceDir) {
    $moduleFiles = Get-ChildItem -Path $moduleSourceDir -Filter "*.ixx"
    foreach ($file in $moduleFiles) {
        Copy-Item -Path $file.FullName -Destination $moduleDestDir -Force
        Write-Host "  [OK] $($file.Name)" -ForegroundColor Green
    }
    Write-Host ""
    Write-Host "Module files: $($moduleFiles.Count) files copied" -ForegroundColor Green
} else {
    Write-Host "  [NG] Module directory not found: $moduleSourceDir" -ForegroundColor Red
}
Write-Host ""
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "Collection Complete" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "Libraries: Copied $libCopied / 4" -ForegroundColor $(if ($libCopied -eq 4) { "Green" } else { "Yellow" })
if ($libMissing -gt 0) { Write-Host "Not built: $libMissing / 4" -ForegroundColor Yellow }
Write-Host ""
Write-Host "Output: $DistDir" -ForegroundColor Cyan
Write-Host ""
Write-Host "Directory structure:" -ForegroundColor Cyan
Get-ChildItem -Path $DistDir -Recurse | Where-Object { -not $_.PSIsContainer } | ForEach-Object {
    $relativePath = $_.FullName.Replace($DistDir, "").TrimStart("\")
    Write-Host "  $relativePath" -ForegroundColor Gray
}
