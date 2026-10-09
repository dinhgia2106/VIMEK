param(
    [ValidateSet('x64','x86')] [string]$Platform = 'x64',
    [string]$BinaryRoot = 'dist'
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
    Compress-Archive -Path $vimekBinary,"$vimekBinaryDir/LICENSE","$vimekBinaryDir/NOTICE.md","$vimekBinaryDir/README.md" -DestinationPath "dist/VIMEK-0.1.0-Windows-$Platform.zip" -Force

    # Include actual working-tree changes and new files, preserving directories.
    # Exclude the Git database, compiler and outputs via .gitignore.
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $vimekArchivePath = Join-Path $vimekRoot 'dist/VIMEK-0.1.0-source.zip'
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
    Get-FileHash "dist/VIMEK-0.1.0-Windows-$Platform.zip",$vimekArchivePath -Algorithm SHA256 | ForEach-Object { "$($_.Hash)  $(Split-Path -Leaf $_.Path)" } | Set-Content -Encoding utf8 "dist/SHA256-$Platform.txt"
    Write-Host "Packaged Windows $Platform and corresponding VIMEK source."
} finally { Pop-Location }
