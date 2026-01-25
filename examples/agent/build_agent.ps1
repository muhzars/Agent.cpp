# build_agent.ps1
# This script finds MSBuild and builds the agent project.

$VSPaths = @(
    "C:\Program Files\Microsoft Visual Studio",
    "C:\Program Files (x86)\Microsoft Visual Studio"
)

Write-Host "Searching for MSBuild.exe..."
$msbuildPath = Get-ChildItem -Path $VSPaths -Filter "MSBuild.exe" -Recurse -Depth 10 -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName

if (-not $msbuildPath) {
    Write-Error "MSBuild.exe not found in standard Visual Studio paths. Please ensure Visual Studio is installed."
    exit 1
}

Write-Host "Found MSBuild: $msbuildPath"

# Ensure build directory exists
if (-not (Test-Path "../../build")) {
    New-Item -ItemType Directory -Path "../../build"
}

cd ../../build

Write-Host "Building agent target..."
& $msbuildPath examples/agent/agent.vcxproj /p:Configuration=Release

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nBuild Successful!"
    # Search for the EXE to tell the user where it is
    $exePath = Get-ChildItem -Filter "agent.exe" -Recurse | Select-Object -First 1 -ExpandProperty FullName
    if ($exePath) {
        Write-Host "Executable location: $exePath"
    }
} else {
    Write-Host "`nBuild Failed."
}
