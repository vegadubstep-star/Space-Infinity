$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$project = Get-Content -LiteralPath (Join-Path $root 'native-vst3/CMakeLists.txt') -Raw
if ($project -notmatch 'project\(SpaceInfinity VERSION ([0-9]+\.[0-9]+\.[0-9]+)') {
    throw 'Cannot determine the native plugin version.'
}
$version = $Matches[1]
$tag = "v$version"
$releaseDir = Join-Path $root 'release'
$setupName = "SPACE_INFINITY_${version}_Windows_x64_Setup.exe"
$archiveName = "SPACE_INFINITY_${version}_Windows_x64_VST3.zip"
$files = @($setupName, $archiveName, 'READ_ME_FIRST.md', 'TEN_CONTROL_UPDATES.md',
    'THIRD_PARTY_SAMPLES.md', 'SAMPLE_PROVENANCE.json', 'CC0-1.0.txt')
foreach ($name in $files) {
    if (-not (Test-Path -LiteralPath (Join-Path $releaseDir $name) -PathType Leaf)) {
        throw "Missing verified release file: $name"
    }
}
if (-not $env:GITHUB_REPOSITORY -or -not $env:GITHUB_SHA -or -not $env:GH_TOKEN) {
    throw 'Publishing requires the GitHub repository, commit and token from the workflow.'
}
$checksumPath = Join-Path $releaseDir 'SHA256SUMS.txt'
$checksums = foreach ($name in @($setupName, $archiveName)) {
    $hash = Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $releaseDir $name)
    "$($hash.Hash.ToLowerInvariant())  $name"
}
$checksums | Set-Content -LiteralPath $checksumPath -Encoding utf8
$files += 'SHA256SUMS.txt'
$files += 'SPACE_INFINITY_Heritage_200.zip'
$files += 'SPACE_INFINITY_Heritage_200.zip.sha256'
$files += 'HERITAGE_200_UPDATE.md'
$files += 'DISTORTION_DRIVE_UPDATE.md'
$files += "SPACE_INFINITY_${version}_Choir_Stabs.zip"
$files += "SPACE_INFINITY_${version}_Choir_Stabs.zip.sha256"
foreach ($document in @('RHYTHM_AND_METAL_UPDATE.md', 'ANALOGUE_AND_PARITY_UPDATE.md', 'SERUM2_FEATURE_AUDIT.md', 'ACID_BASS_AND_CHOIR_UPDATE.md', 'ACID_CUTOFF_UPDATE.md', 'BASS_VOICING_UPDATE.md', 'USER_CHOIR_PROVENANCE.json', 'CREATIVE_WORKFLOW_UPDATE.md', 'CHOIR_SAMPLE_PROVENANCE.json', 'CONTROL_AND_LEVEL_UPDATE.md', 'TRANCE_FACTORY_BANK.md', 'TRANCE_FACTORY_BANK_RESEARCH.md', 'TRANCE_PRESET_CATALOGUE.md', 'TRANCE_SAMPLE_PROVENANCE.json')) {
    if (Test-Path -LiteralPath (Join-Path $releaseDir $document)) { $files += $document }
}
& python (Join-Path $PSScriptRoot 'package-installer-zip.py') $releaseDir $version
if ($LASTEXITCODE -ne 0) { throw 'Installer ZIP packaging failed.' }
$files += "SPACE_INFINITY_${version}_Windows_x64_Installer.zip"
$files += "SPACE_INFINITY_${version}_Windows_x64_Installer.zip.sha256"
$repo = $env:GITHUB_REPOSITORY
$notesSource = Join-Path $root 'docs/HERITAGE_200_UPDATE.md'
if (-not (Test-Path -LiteralPath $notesSource)) { $notesSource = Join-Path $root 'docs/RHYTHM_AND_METAL_UPDATE.md' }
# Keep a new release private until its installer and archive have uploaded.
$existing = & gh release view $tag --repo $repo --json targetCommitish,isDraft 2>$null
if ($LASTEXITCODE -ne 0) {
    & gh release create $tag --repo $repo --target $env:GITHUB_SHA --draft --prerelease `
        --title "SPACE INFINITY $version - Windows x64" `
        --notes-file $notesSource
    if ($LASTEXITCODE -ne 0) { throw 'Could not create the draft release.' }
}
$notesFile = Join-Path $releaseDir 'RELEASE_NOTES.md'
$notes = (Get-Content -LiteralPath $notesSource -Raw).Replace('(images/', "(https://github.com/$repo/blob/$($env:GITHUB_SHA)/docs/images/")
$docsUrl = "https://github.com/$repo/blob/$($env:GITHUB_SHA)/docs/"
$notes = $notes.Replace('(SERUM2_FEATURE_AUDIT.md)', "(${docsUrl}SERUM2_FEATURE_AUDIT.md)").Replace('(audit/NATIVE_PARAMETER_INVENTORY.json)', "(${docsUrl}audit/NATIVE_PARAMETER_INVENTORY.json)")
$notes = $notes.Replace('(TRANCE_FACTORY_BANK_RESEARCH.md)', "(${docsUrl}TRANCE_FACTORY_BANK_RESEARCH.md)")
$notes = $notes.Replace('(DISTORTION_DRIVE_UPDATE.md)', "(${docsUrl}DISTORTION_DRIVE_UPDATE.md)")
$notes | Set-Content -LiteralPath $notesFile -Encoding utf8
$paths = @($files | ForEach-Object { Join-Path $releaseDir $_ })
$uploaded = $false
for ($attempt = 1; $attempt -le 3; $attempt++) {
    & gh release upload $tag @paths --repo $repo --clobber
    if ($LASTEXITCODE -eq 0) { $uploaded = $true; break }
    if ($attempt -lt 3) { Start-Sleep -Seconds (5 * $attempt) }
}
if (-not $uploaded) { throw 'Release upload failed after three attempts; the new release remains a draft.' }
$assetJson = & gh release view $tag --repo $repo --json assets
if ($LASTEXITCODE -ne 0) { throw 'Cannot verify the uploaded release assets.' }
$assets = ($assetJson | ConvertFrom-Json).assets
foreach ($name in $files) {
    $asset = @($assets | Where-Object { $_.name -eq $name })
    $length = (Get-Item -LiteralPath (Join-Path $releaseDir $name)).Length
    if ($asset.Count -ne 1 -or $asset[0].size -ne $length) {
        throw "Uploaded asset is missing or has the wrong size: $name"
    }
}
& gh release edit $tag --repo $repo --draft=false --prerelease `
    --title "SPACE INFINITY $version - Windows x64" `
    --notes-file $notesFile
if ($LASTEXITCODE -ne 0) { throw 'Could not publish the verified release.' }
Write-Host "Installer: https://github.com/$repo/releases/download/$tag/$setupName"
Write-Host "Release: https://github.com/$repo/releases/tag/$tag"
