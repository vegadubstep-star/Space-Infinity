param([string]$InnoCompiler)
$ErrorActionPreference = "Stop"
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
if (-not $InnoCompiler) {
    $command = Get-Command ISCC.exe -ErrorAction SilentlyContinue
    if ($command) { $InnoCompiler = $command.Source }
    else { $InnoCompiler = Join-Path ${env:ProgramFiles(x86)} "Inno Setup 6\ISCC.exe" }
}
if (-not (Test-Path -LiteralPath $InnoCompiler -PathType Leaf)) {
    throw "Install Inno Setup 6 or supply its compiler path using -InnoCompiler."
}

& (Join-Path $PSScriptRoot "build-windows.ps1") -Configuration Release
$bundle = Join-Path $ProjectRoot "native-vst3\build\SpaceInfinity_artefacts\Release\VST3\SPACE INFINITY.vst3"
$plugin = Join-Path $bundle "Contents\x86_64-win\SPACE INFINITY.vst3"
if (-not (Test-Path -LiteralPath $plugin -PathType Leaf)) {
    throw "The Windows x64 VST3 module is missing from the build output."
}
$reader = [System.IO.BinaryReader]::new([System.IO.File]::OpenRead($plugin))
try {
    if ($reader.ReadUInt16() -ne 0x5A4D) { throw "The VST3 module is not a Windows PE file." }
    $reader.BaseStream.Position = 0x3C
    $reader.BaseStream.Position = $reader.ReadInt32()
    if ($reader.ReadUInt32() -ne 0x4550 -or $reader.ReadUInt16() -ne 0x8664) {
        throw "The VST3 module is not a Windows x64 PE file."
    }
} finally { $reader.Dispose() }

$release = Join-Path $ProjectRoot "release"
New-Item -ItemType Directory -Path $release -Force | Out-Null
$definition = Join-Path $ProjectRoot "installer\SpaceInfinity.iss"
& $InnoCompiler "/DVst3Bundle=$bundle" "/DReleaseDir=$release" $definition
if ($LASTEXITCODE -ne 0) { throw "Inno Setup compilation failed." }
$setup = Join-Path $release "SPACE_INFINITY_0.1.0_Windows_x64_Setup.exe"
if (-not (Test-Path -LiteralPath $setup -PathType Leaf)) { throw "Installer output is missing." }

$archive = Join-Path $release "SPACE_INFINITY_0.1.0_Windows_x64_VST3.zip"
Compress-Archive -LiteralPath $bundle -DestinationPath $archive -Force
Get-FileHash -Algorithm SHA256 -LiteralPath $setup,$archive


foreach ($notice in @("THIRD_PARTY_SAMPLES.md", "SAMPLE_PROVENANCE.json", "CC0-1.0.txt")) {
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "native-vst3/Samples/$notice") -Destination $release -Force
}
Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/TESTING_STUDIO_BUILD.md") -Destination (Join-Path $release "READ_ME_FIRST.md") -Force
