# scripts/generate_sources.ps1
param(
    [string]$WorkspaceRoot = "$PSScriptRoot/..",
    [string]$OutputFile = "$PSScriptRoot/../build/sources.rsp"
)

# Normalize paths (resolve .. and slashes to avoid substring errors)
$WorkspaceRoot = (Resolve-Path $WorkspaceRoot).Path
$OutputFile = (Resolve-Path $OutputFile).Path

# Ensure build directory exists
$buildDir = Join-Path $WorkspaceRoot "build"
if (!(Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
}

# Define folders to include
$includeFolders = @(
    "VulcanEngine",
    "Applications"
)

# Define patterns to exclude
$excludePatterns = @(
    "*/obj/*",
    "*/bin/*",
    "*/build/*",
    "*/Dependencies/*",
    "*/third_party/*"
)

# Collect all .cpp files recursively
$sources = @()
foreach ($folder in $includeFolders) {
    $fullPath = Join-Path $WorkspaceRoot $folder
    if (Test-Path $fullPath) {
        $fullPath = (Resolve-Path $fullPath).Path  # Normalize folder path too
        
        $files = Get-ChildItem -Path $fullPath -Recurse -Include *.cpp -File -ErrorAction SilentlyContinue |
                 Where-Object {
                     $path = $_.FullName
                     $exclude = $false
                     foreach ($excludePattern in $excludePatterns) {
                         if ($path -like "*$excludePattern*") {
                             $exclude = $true
                             break
                         }
                     }
                     return !$exclude
                 } |
                 ForEach-Object {
                     # Robust relative path calculation
                     $relPath = $_.FullName.Substring($WorkspaceRoot.Length).TrimStart('\') -replace '\\', '/'
                     $relPath
                 }
        $sources += $files
    }
}

# Write to response file (Compatible with PowerShell 5.1)
if ($sources.Count -gt 0) {
    Write-Host "Generated $($sources.Count) source files: $OutputFile"
    
    # Write without BOM using StreamWriter
    $writer = [System.IO.StreamWriter]::new($OutputFile, $false, [System.Text.UTF8Encoding]::new($false))
    foreach ($source in $sources) {
        $writer.WriteLine($source)
    }
    $writer.Close()
    $writer.Dispose()
} else {
    Write-Host "Warning: No .cpp files found in VulcanEngine/Applications"
    
    # Create empty file without BOM
    $writer = [System.IO.StreamWriter]::new($OutputFile, $false, [System.Text.UTF8Encoding]::new($false))
    $writer.Close()
    $writer.Dispose()
}