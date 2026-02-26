<#
.SYNOPSIS
    Builds the project using local sibling directories for dependencies.
.DESCRIPTION
    Configures CMake with local overrides for 'modules' and 'test-runner',
    then runs the build.
.PARAMETER Clean
    If set, deletes the 'build' directory before starting. 
    Use this if CMake gets confused about old paths.
.PARAMETER ModulesPath
    Path to the flecs-test-runner-modules directory.
    Defaults to '../flecs-test-runner-modules' if not supplied.
#>
param (
    [switch]$Build,
    [switch]$Clean,
    [string]$ModulesPath = "../flecs-test-runner-modules"
)

$BuildDir = "build"

# --- Step 1: Clean Build Directory (Optional) ---
if ($Clean) {
    if (Test-Path $BuildDir) {
        Write-Host "[-] Clean requested. Removing '$BuildDir'..." -ForegroundColor Yellow
        Remove-Item -Path $BuildDir -Recurse -Force -ErrorAction SilentlyContinue
    }
}

# --- Step 2: Configure CMake ---
Write-Host "[*] Configuring CMake with local overrides..." -ForegroundColor Cyan
Write-Host "    - Modules: $ModulesPath" -ForegroundColor Gray

# We use the Call Operator (&) and an array for clean argument handling
$CmakeArgs = @(
    "-S", ".",
    "-B", $BuildDir,
    "-DFETCHCONTENT_SOURCE_DIR_MODULES=$ModulesPath"
)

& cmake $CmakeArgs

# Check if configuration failed
if ($LASTEXITCODE -ne 0) {
    Write-Host "[!] Configuration Failed." -ForegroundColor Red
    exit 1
}

if ($Build) {
    # --- Step 3: Build the Project ---
    Write-Host "[*] Building project..." -ForegroundColor Cyan

    # --parallel uses all available CPU cores for a faster build
    cmake --build $BuildDir --parallel 

    if ($LASTEXITCODE -eq 0) {
        Write-Host "[+] Build Successful!" -ForegroundColor Green
    } else {
        Write-Host "[!] Build Failed." -ForegroundColor Red
        exit 1
    }
}