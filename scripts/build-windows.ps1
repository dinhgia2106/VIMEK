param(
    [ValidateSet('x64','x86')] [string]$Platform = 'x64',
    [string]$Toolchain = '',
    [string]$OutputRoot = 'dist',
    [switch]$TestsOnly
)
$ErrorActionPreference = 'Stop'
$vimekRoot = Split-Path -Parent $PSScriptRoot
Push-Location $vimekRoot
try {
    $vimekOutputRoot = [IO.Path]::GetFullPath((Join-Path $vimekRoot $OutputRoot))
    if (-not $vimekOutputRoot.StartsWith($vimekRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'OutputRoot must be inside this workspace.' }
    $vimekOutput = Join-Path $vimekOutputRoot $Platform
    if (-not $Toolchain) {
        $vimekPortable = Get-ChildItem -LiteralPath (Join-Path $vimekRoot '.tools') -Directory -Filter 'llvm-mingw-*' -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($vimekPortable) { $Toolchain = Join-Path $vimekPortable.FullName 'bin' }
    }
    $vimekArch = if ($Platform -eq 'x64') { 'x86_64' } else { 'i686' }
    $vimekCompilerName = "$vimekArch-w64-mingw32-clang++.exe"
    $vimekCompiler = if ($Toolchain) { Join-Path $Toolchain $vimekCompilerName } else { (Get-Command $vimekCompilerName -ErrorAction Stop).Source }
    $vimekWindres = if ($Toolchain) { Join-Path $Toolchain "$vimekArch-w64-mingw32-windres.exe" } else { (Get-Command "$vimekArch-w64-mingw32-windres.exe" -ErrorAction Stop).Source }
    New-Item -ItemType Directory -Force "build/$Platform", $vimekOutput | Out-Null
    $vimekEngine = @(Get-ChildItem 'Sources/VIMEK/engine' -Filter '*.cpp' | ForEach-Object FullName)
    $vimekFlags = @('-std=c++14','-O2','-static','-DUNICODE','-D_UNICODE','-D_CRT_SECURE_NO_WARNINGS','-Wno-deprecated-declarations','-Wno-ignored-pragmas')
    & $vimekCompiler @vimekFlags '-ISources/VIMEK/engine' @vimekEngine 'tests/engine_tests.cpp' '-o' "build/$Platform/engine_tests.exe"
    if ($LASTEXITCODE -ne 0) { throw 'Engine test compilation failed.' }
    & "./build/$Platform/engine_tests.exe"
    if ($LASTEXITCODE -ne 0) { throw 'Engine tests failed.' }
    if (-not $TestsOnly) {
        $vimekWin = Join-Path $vimekRoot 'Sources/VIMEK/windows/App'
        Push-Location $vimekWin
        try {
            & $vimekWindres '--codepage=65001' '-DVIMEK_PORTABLE_BUILD' '-i' 'VIMEK.rc' '-o' "$vimekRoot/build/$Platform/vimek.res" '-O' 'coff'
            if ($LASTEXITCODE -ne 0) { throw 'Resource compilation failed.' }
        } finally { Pop-Location }
        $vimekSources = @(Get-ChildItem $vimekWin -Filter '*.cpp' | ForEach-Object FullName)
        $vimekBits = if ($Platform -eq 'x64') { '64' } else { '32' }
        & $vimekCompiler @vimekFlags '-DNDEBUG' '-municode' '-mwindows' @vimekEngine @vimekSources "build/$Platform/vimek.res" '-o' "$vimekOutput/VIMEK$vimekBits.exe" '-lcomctl32' '-lcomdlg32' '-lshell32' '-lole32' '-luuid' '-lversion' '-lurlmon' '-luxtheme' '-limm32' '-lpsapi' '-lgdiplus' '-ldwmapi'
        if ($LASTEXITCODE -ne 0) { throw 'Windows application compilation failed.' }
        Copy-Item LICENSE "$vimekOutput/LICENSE"
        Copy-Item NOTICE.md "$vimekOutput/NOTICE.md"
        Write-Host "Built $vimekOutput/VIMEK$vimekBits.exe"
    }
} finally { Pop-Location }
