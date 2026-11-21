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
$setup = Join-Path $release "SPACE_INFINITY_0.9.4_Windows_x64_Setup.exe"
if (-not (Test-Path -LiteralPath $setup -PathType Leaf)) { throw "Installer output is missing." }

$archive = Join-Path $release "SPACE_INFINITY_0.9.4_Windows_x64_VST3.zip"
Compress-Archive -LiteralPath $bundle -DestinationPath $archive -Force
Get-FileHash -Algorithm SHA256 -LiteralPath $setup,$archive


foreach ($notice in @("THIRD_PARTY_SAMPLES.md", "SAMPLE_PROVENANCE.json", "CC0-1.0.txt")) {
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "native-vst3/Samples/$notice") -Destination $release -Force
}
Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/TESTING_STUDIO_BUILD.md") -Destination (Join-Path $release "READ_ME_FIRST.md") -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/TEN_CONTROL_UPDATES.md") -Destination (Join-Path $release "TEN_CONTROL_UPDATES.md") -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/RHYTHM_AND_METAL_UPDATE.md") -Destination (Join-Path $release "RHYTHM_AND_METAL_UPDATE.md") -Force

foreach ($document in @("ANALOGUE_AND_PARITY_UPDATE.md", "SERUM2_FEATURE_AUDIT.md", "CREATIVE_WORKFLOW_UPDATE.md", "CONTROL_AND_LEVEL_UPDATE.md", "TRANCE_FACTORY_BANK.md", "TRANCE_FACTORY_BANK_RESEARCH.md", "TRANCE_PRESET_CATALOGUE.md")) {
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/$document") -Destination (Join-Path $release $document) -Force
}
New-Item -ItemType Directory -Path (Join-Path $release "audit") -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/audit/NATIVE_PARAMETER_INVENTORY.json") -Destination (Join-Path $release "audit/NATIVE_PARAMETER_INVENTORY.json") -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "native-vst3/Samples/Trance/PROVENANCE.json") -Destination (Join-Path $release "TRANCE_SAMPLE_PROVENANCE.json") -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "native-vst3/Samples/Trance/CHOIR_PROVENANCE.json") -Destination (Join-Path $release "CHOIR_SAMPLE_PROVENANCE.json") -Force

$choirFolder = Join-Path $release "ChoirStabs"
$smoke = Join-Path $ProjectRoot "native-vst3/build/SpaceInfinitySmoke_artefacts/Release/SpaceInfinitySmoke.exe"
& $smoke --export-choir-stabs $choirFolder
if ($LASTEXITCODE -ne 0) { throw "Owner choir preset export failed." }
& python (Join-Path $PSScriptRoot "package-choir-stabs.py") $choirFolder "0.9.4" (Join-Path $release "SPACE_INFINITY_0.9.4_Choir_Stabs.zip")
if ($LASTEXITCODE -ne 0) { throw "Owner choir pack verification failed." }
Copy-Item -LiteralPath (Join-Path $ProjectRoot "native-vst3/Samples/Trance/USER_CHOIR_PROVENANCE.json") -Destination (Join-Path $release "USER_CHOIR_PROVENANCE.json") -Force
Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/ACID_BASS_AND_CHOIR_UPDATE.md") -Destination $release -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/ACID_CUTOFF_UPDATE.md") -Destination $release -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/BASS_VOICING_UPDATE.md") -Destination $release -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "native-vst3/Presets/Heritage200/SPACE_INFINITY_Heritage_200.zip") -Destination $release -Force
Copy-Item -LiteralPath (Join-Path $ProjectRoot "native-vst3/Presets/Heritage200/SPACE_INFINITY_Heritage_200.zip.sha256") -Destination $release -Force
Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/HERITAGE_200_UPDATE.md") -Destination $release -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/DISTORTION_DRIVE_UPDATE.md") -Destination $release -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/HARDWARE_TIMING_NOISE_UPDATE.md") -Destination $release -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/MODULATION_CHORUS_MELT_UPDATE.md") -Destination $release -Force
New-Item -ItemType Directory -Path (Join-Path $release "research") -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/research/CHORUS_DIMENSION_RESEARCH.md") -Destination (Join-Path $release "research") -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "native-vst3/Samples/Trance/FLAT_CHOIR_PROVENANCE.json") -Destination $release -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/TRACK_READY_080_UPDATE.md") -Destination $release -Force

foreach ($audit in @("TRACK_READY_080_AUDIO_LEVELS.json", "TRACK_READY_080_NATIVE_TESTS.txt", "TRACK_READY_080_SIMILARITY.json")) {
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/audit/$audit") -Destination (Join-Path $release "audit") -Force
}

Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/CHARACTER_FILTER_SAMPLER_090_UPDATE.md") -Destination $release -Force

Copy-Item -LiteralPath (Join-Path $ProjectRoot "native-vst3/Presets/Lush20/SPACE_INFINITY_Lush_Trance_20.zip") -Destination $release -Force
Copy-Item -LiteralPath (Join-Path $ProjectRoot "native-vst3/Presets/Lush20/SPACE_INFINITY_Lush_Trance_20.zip.sha256") -Destination $release -Force

foreach ($audit in @("CHARACTER_090_NATIVE_TESTS.txt", "LEADS_090_AUDIO_LEVELS.json")) {
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/audit/$audit") -Destination (Join-Path $release "audit") -Force
}

Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/STUDIO_RESEARCH_091_UPDATE.md") -Destination $release -Force
Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/research/STUDIO_RESEARCH_091.md") -Destination (Join-Path $release "research") -Force
foreach ($audit in @("STUDIO_RESEARCH_091_NATIVE_TESTS.txt", "STUDIO_RESEARCH_091_AUDIO_LEVELS.json", "STUDIO_RESEARCH_091_RECIPES.json", "STUDIO_RESEARCH_091_FACTORY_PARITY.json")) {
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/audit/$audit") -Destination (Join-Path $release "audit") -Force
}

foreach ($file in @("BROWSING_SAMPLE_GLIDE_092_UPDATE.md", "audit/BROWSING_SAMPLE_GLIDE_092_NATIVE_TESTS.txt")) {
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/$file") -Destination (Join-Path $release $file) -Force
}

foreach ($file in @("SAMPLER_FULL_RANGE_093_UPDATE.md", "ORBIT_SAMPLES_093_PROVENANCE.json", "audit/SAMPLER_FULL_RANGE_093_NATIVE_TESTS.txt", "audit/SAMPLER_FULL_RANGE_093_AUDIO_LEVELS.json", "audit/SAMPLER_FULL_RANGE_093_CURATION.json")) {
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/$file") -Destination (Join-Path $release $file) -Force
}
foreach ($file in @("SPACE_INFINITY_Orbit_Voices.zip", "SPACE_INFINITY_Orbit_Voices.zip.sha256", "SPACE_INFINITY_Retired_093_Backup.zip")) {
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "native-vst3/Presets/UserVoices093/$file") -Destination $release -Force
}

foreach ($file in @("INSTALLED_BANK_094_UPDATE.md", "audit/INSTALLED_BANK_094_LEVELS.json", "audit/INSTALLED_BANK_094_NATIVE_TESTS.txt")) {
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "docs/$file") -Destination (Join-Path $release $file) -Force
}
Copy-Item -LiteralPath (Join-Path $ProjectRoot "native-vst3/Presets/Installed094.json") -Destination (Join-Path $release "audit/INSTALLED_BANK_094_CATALOGUE.json") -Force
