param([string]$Configuration = "Release")
$ErrorActionPreference = "Stop"
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$SourceDir = Join-Path $ProjectRoot "native-vst3"
$BuildDir = Join-Path $SourceDir "build"
foreach ($tool in @("cmake", "git")) {
  if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
    throw "Missing $tool. Install Visual Studio 2022 C++ workload, Git, and CMake first."
  }
}
Write-Host "Configuring SPACE INFINITY Windows x64 VST3 from $SourceDir"
& cmake -S $SourceDir -B $BuildDir -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed. Resolve errors before building." }
& cmake --build $BuildDir --config $Configuration
if ($LASTEXITCODE -ne 0) { throw "Build failed. See compiler/linker errors above." }
Write-Host "Build command succeeded. Verify the actual VST3 under $BuildDir\SpaceInfinity_artefacts\$Configuration\VST3 before installing."
Write-Host "IMPORTANT: Compilation does not establish compatibility with FL Studio; test it in the host."