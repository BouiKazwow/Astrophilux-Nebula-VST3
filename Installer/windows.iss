#define AppName "Astrophilux Nebula"
#define AppVersion "0.3.0"

[Setup]
AppId={{A57A0F3B-35B1-4E15-95B6-A5700B3BCA11}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=Astrophilux
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
OutputDir=..\dist
OutputBaseFilename=Astrophilux_Nebula_Windows_Setup
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
Uninstallable=yes

[Files]
Source: "..\build\AstrophiluxNebula_artefacts\Release\VST3\Astrophilux Nebula.vst3\*"; DestDir: "{app}\Astrophilux Nebula.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Uninstall Astrophilux Nebula"; Filename: "{uninstallexe}"
