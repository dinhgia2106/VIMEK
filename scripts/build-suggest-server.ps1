# Builds dist/suggest: vimek-suggest.exe (PyInstaller, one folder) and its ONNX models.
# VIMEK starts it for "Gợi ý dấu bằng AI"; package-windows.ps1 puts the folder next to VIMEK64.exe.
#
#   ./scripts/build-suggest-server.ps1 -Virestore E:\vi-restore-llm -Models E:\vi-restore-llm\runs
#
# -Virestore: a vi-restore-llm checkout (its src/virestore is bundled).
# -Models: a folder holding ds-tagger-prefix/{config.json,tagger.int8.onnx} and
#   ds-boundary-large-span/{config.json,lexicon.json,boundary.int8.onnx}
#   (python -m virestore.export_onnx --boundary ... --quantize ...).
# Only numpy and onnxruntime are installed in the build venv, so PyTorch is never bundled.
# onnxruntime has no 32-bit Windows wheel: the server exists for x64 only.
param(
    [Parameter(Mandatory)] [string]$Virestore,
    [Parameter(Mandatory)] [string]$Models,
    [string]$Python = 'python',
    [string]$OutDir = 'dist/suggest'
)
$ErrorActionPreference = 'Stop'
$vimekRoot = Split-Path -Parent $PSScriptRoot
Push-Location $vimekRoot
try {
    $src = Join-Path $Virestore 'src'
    if (-not (Test-Path (Join-Path $src 'virestore/serve.py'))) { throw "No virestore package under $src." }
    $files = @{
        'tagger/config.json'          = 'ds-tagger-prefix/config.json'
        'tagger/tagger.int8.onnx'     = 'ds-tagger-prefix/tagger.int8.onnx'
        'boundary/config.json'        = 'ds-boundary-large-span/config.json'
        'boundary/lexicon.json'       = 'ds-boundary-large-span/lexicon.json'
        'boundary/boundary.int8.onnx' = 'ds-boundary-large-span/boundary.int8.onnx'
    }
    foreach ($f in $files.Values) { if (-not (Test-Path (Join-Path $Models $f))) { throw "Missing model file $f under $Models." } }

    $venv = 'build/suggest-venv'
    if (-not (Test-Path "$venv/Scripts/python.exe")) { & $Python -m venv $venv; if ($LASTEXITCODE) { throw 'venv failed.' } }
    $py = (Resolve-Path "$venv/Scripts/python.exe").Path
    & $py -m pip install --disable-pip-version-check -q 'numpy>=1.26,<3' 'onnxruntime>=1.18,<2' 'pyinstaller>=6.6,<7'
    if ($LASTEXITCODE) { throw 'pip install failed.' }

    $work = 'build/suggest-pyinstaller'
    & $py -m PyInstaller --noconfirm --clean --onedir --noconsole --name vimek-suggest `
        --distpath $work/dist --workpath $work/work --specpath $work `
        --paths $src --hidden-import virestore.serve --hidden-import virestore.boundary `
        --hidden-import virestore.models.tagger --hidden-import virestore.ime `
        --exclude-module torch --exclude-module tkinter --exclude-module fastapi --exclude-module uvicorn `
        Sources/VIMEK/windows/suggest/vimek_suggest.py
    if ($LASTEXITCODE) { throw 'PyInstaller failed.' }

    if (Test-Path $OutDir) { Remove-Item -Recurse -Force $OutDir }
    Copy-Item -Recurse "$work/dist/vimek-suggest" $OutDir
    foreach ($k in $files.Keys) {
        $dest = Join-Path $OutDir "models/$k"
        New-Item -ItemType Directory -Force (Split-Path $dest) | Out-Null
        Copy-Item (Join-Path $Models $files[$k]) $dest
    }
    # Licenses of what the folder contains: vi-restore-llm, Python, numpy, onnxruntime, PyInstaller.
    $licenses = Join-Path $OutDir 'licenses'
    New-Item -ItemType Directory -Force $licenses | Out-Null
    Copy-Item (Join-Path $Virestore 'LICENSE') (Join-Path $licenses 'vi-restore-llm.txt')
    $base = & $py -c 'import sys; print(sys.base_prefix)'
    Copy-Item (Join-Path $base 'LICENSE.txt') (Join-Path $licenses 'Python.txt')
    $site = & $py -c 'import sysconfig; print(sysconfig.get_paths()["purelib"])'
    foreach ($pkg in 'numpy', 'onnxruntime', 'pyinstaller') {
        # numpy and PyInstaller keep them in dist-info; onnxruntime in its package folder
        $dirs = @(Get-ChildItem $site -Directory | Where-Object { $_.Name -like "$pkg-*.dist-info" -or $_.Name -eq $pkg })
        if (-not $dirs) { throw "No $pkg in the build venv." }
        $n = 0
        foreach ($f in $dirs | ForEach-Object { Get-ChildItem $_.FullName -File -Recurse -Depth 1 } | Where-Object { $_.Name -match '^(LICEN[CS]E|COPYING|NOTICE|ThirdPartyNotices)' }) {
            $n++
            Copy-Item $f.FullName (Join-Path $licenses "$pkg-$($f.Name)")
        }
        if (-not $n) { throw "No license file in $($info.Name)." }
    }
    $mb = [math]::Round(((Get-ChildItem -Recurse $OutDir | Measure-Object Length -Sum).Sum) / 1MB, 1)
    Write-Host "Built $OutDir ($mb MB)."
} finally { Pop-Location }
