#ifndef Vst3Bundle
  #error Pass /DVst3Bundle=<absolute path to the compiled SPACE INFINITY.vst3 folder>
#endif
#ifndef ReleaseDir
  #error Pass /DReleaseDir=<absolute output directory>
#endif

[Setup]
AppId=OrbitalAudio.SpaceInfinity.VST3
AppName=SPACE INFINITY
AppVersion=0.9.3
AppPublisher=Orbital Audio
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
UninstallFilesDir={commonappdata}\Orbital Audio\SPACE INFINITY\Uninstall
OutputDir={#ReleaseDir}
OutputBaseFilename=SPACE_INFINITY_0.9.3_Windows_x64_Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
CloseApplications=no

[Files]
Source: "{#Vst3Bundle}\*"; DestDir: "{app}\SPACE INFINITY.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
