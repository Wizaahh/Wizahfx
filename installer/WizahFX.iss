[Setup]
AppName=Wizah FX
AppVersion=1.0.0
AppPublisher=Wizah
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
Uninstallable=yes
CreateUninstallRegKey=yes
OutputBaseFilename=Wizah-FX-Setup
OutputDir=Output
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
PrivilegesRequired=admin
Compression=lzma
SolidCompression=yes

[Files]
Source: "..\stage\Wizah FX.vst3\*"; DestDir: "{commoncf64}\VST3\Wizah FX.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion

[Messages]
FinishedLabel=Wizah FX est installé. Dans FL Studio : Options > Manage plugins > Find installed plugins.
