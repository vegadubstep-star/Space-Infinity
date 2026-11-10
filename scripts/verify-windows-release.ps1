$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$release = Join-Path $root 'release'
$setup = @(Get-ChildItem -LiteralPath $release -Filter '*_Setup.exe')
$archive = @(Get-ChildItem -LiteralPath $release -Filter '*_VST3.zip')
if ($setup.Count -ne 1 -or $archive.Count -ne 1) { throw 'Expected one installer and one VST3 archive.' }
Write-Host '1. Both release files exist.'
$temp = Join-Path $env:RUNNER_TEMP ('space-infinity-' + [guid]::NewGuid())
try {
    Expand-Archive -LiteralPath $archive[0].FullName -DestinationPath $temp
    $bundle = Join-Path $temp 'SPACE INFINITY.vst3'
    $modulePath = 'Contents\x86_64-win\SPACE INFINITY.vst3'
    $binary = Join-Path $bundle $modulePath
    if (-not (Test-Path -LiteralPath $binary -PathType Leaf)) { throw 'VST3 archive has an invalid bundle layout.' }
    Write-Host '2. The manual archive contains the Windows x64 VST3 module.'
    $manifest = Get-Content -LiteralPath (Join-Path $bundle 'Contents\Resources\moduleinfo.json') -Raw | ConvertFrom-Json
    if ($manifest.Name -ne 'SPACE INFINITY' -or $manifest.Classes.Count -lt 1) { throw 'Invalid VST3 manifest.' }
    Write-Host '3. The VST3 manifest identifies the plugin and lists its classes.'
    $log = Join-Path $temp 'setup.log'
    $setupArgs = '/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /LOG="' + $log + '"'
    $install = Start-Process -FilePath $setup[0].FullName -ArgumentList $setupArgs -Wait -PassThru
    if ($install.ExitCode -ne 0) { Get-Content -LiteralPath $log; throw "Installer exit $($install.ExitCode)." }
    Write-Host '4. The installer ran successfully.'
    $installed = Join-Path $env:CommonProgramFiles 'VST3\SPACE INFINITY.vst3'
    $installedBinary = Join-Path $installed $modulePath
    if (-not (Test-Path -LiteralPath $installedBinary -PathType Leaf)) { throw 'Installer did not populate the standard VST3 directory.' }
    Write-Host '5. The plugin was installed in the standard VST3 directory.'
    if ((Get-FileHash -LiteralPath $binary).Hash -ne (Get-FileHash -LiteralPath $installedBinary).Hash) { throw 'Installed and archived binaries differ.' }
    Write-Host '6. The installed binary matches the manual VST3 archive byte for byte.'
    $uninstallRoot = Join-Path $env:ProgramData 'Orbital Audio\SPACE INFINITY\Uninstall'
    $uninstaller = @(Get-ChildItem -LiteralPath $uninstallRoot -Filter 'unins*.exe')
    if ($uninstaller.Count -ne 1) { throw 'Expected one uninstaller.' }
    $uninstall = Start-Process -FilePath $uninstaller[0].FullName -ArgumentList '/VERYSILENT /SUPPRESSMSGBOXES /NORESTART' -Wait -PassThru
    if ($uninstall.ExitCode -ne 0 -or (Test-Path -LiteralPath $installedBinary)) { throw 'Uninstall did not remove the VST3.' }
    Write-Host '7. The uninstaller removed the installed VST3.'
} finally {
    if (Test-Path -LiteralPath $temp) { Remove-Item -LiteralPath $temp -Recurse -Force }
}
