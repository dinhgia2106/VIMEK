param(
    [ValidateSet('x64','x86')] [string]$Platform = 'x64',
    [string]$BinaryRoot = 'dist',
    [ValidatePattern('^[0-9]+\.[0-9]+\.[0-9]+(?:-[A-Za-z0-9.-]+)?$')] [string]$Version = '0.2.0'
)
$ErrorActionPreference = 'Stop'
$vimekRoot = Split-Path -Parent $PSScriptRoot
Push-Location $vimekRoot
try {
    $vimekBits = if ($Platform -eq 'x64') { '64' } else { '32' }
    $vimekBinaryRoot = [IO.Path]::GetFullPath((Join-Path $vimekRoot $BinaryRoot))
    if (-not $vimekBinaryRoot.StartsWith($vimekRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'BinaryRoot must be inside this workspace.' }
    $vimekBinaryDir = Join-Path $vimekBinaryRoot $Platform
    $vimekBinary = Join-Path $vimekBinaryDir "VIMEK$vimekBits.exe"
    if (-not (Test-Path -LiteralPath $vimekBinary)) { throw 'Run build-windows.ps1 first.' }
    Copy-Item README.md "$vimekBinaryDir/README.md"
    Copy-Item README.vi.md "$vimekBinaryDir/README.vi.md"
    $vimekItems = @($vimekBinary,"$vimekBinaryDir/LICENSE","$vimekBinaryDir/NOTICE.md","$vimekBinaryDir/README.md","$vimekBinaryDir/README.vi.md")
    # The AI suggestion server (build-suggest-server.ps1), next to the exe: x64 only, onnxruntime has no 32-bit build.
    if ($Platform -eq 'x64') {
        if (-not (Test-Path -LiteralPath 'dist/suggest/vimek-suggest.exe')) { throw 'Run build-suggest-server.ps1 first: the x64 package carries the suggestion server.' }
        $vimekItems += (Resolve-Path 'dist/suggest').Path
    }
    Compress-Archive -Path $vimekItems -DestinationPath "dist/VIMEK-$Version-Windows-$Platform.zip" -Force

    # Include actual working-tree changes and new files, preserving directories.
    # Exclude the Git database, compiler and outputs via .gitignore.
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $vimekArchivePath = Join-Path $vimekRoot "dist/VIMEK-$Version-source.zip"
    $vimekStream = [System.IO.File]::Open($vimekArchivePath, [System.IO.FileMode]::Create)
    $vimekArchive = [System.IO.Compression.ZipArchive]::new($vimekStream, [System.IO.Compression.ZipArchiveMode]::Create)
    try {
        $vimekFiles = git -c core.quotepath=false ls-files -co --exclude-standard
        if ($LASTEXITCODE -ne 0) { throw 'Cannot enumerate source files.' }
        foreach ($vimekFile in ($vimekFiles | Sort-Object -Unique)) {
            $vimekAbsolute = Join-Path $vimekRoot $vimekFile
            if (Test-Path -LiteralPath $vimekAbsolute -PathType Leaf) {
                [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($vimekArchive, $vimekAbsolute, $vimekFile.Replace('\','/'), [System.IO.Compression.CompressionLevel]::Optimal) | Out-Null
            }
        }
    } finally { $vimekArchive.Dispose(); $vimekStream.Dispose() }
    Get-FileHash "dist/VIMEK-$Version-Windows-$Platform.zip",$vimekArchivePath -Algorithm SHA256 | ForEach-Object { "$($_.Hash)  $(Split-Path -Leaf $_.Path)" } | Set-Content -Encoding utf8 "dist/SHA256-$Platform.txt"
    Write-Host "Packaged Windows $Platform and corresponding VIMEK source."
} finally { Pop-Location }
