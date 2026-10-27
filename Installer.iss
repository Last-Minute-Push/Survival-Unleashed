[Setup]

; SignTool=ThirdSign $f
AppName=Survival Unleashed
AppVersion=v0.0.3
; AppVerName used in Apps & Features list
AppVerName=Survival Unleashed
AppPublisher=Last Minute Push
AppPublisherURL=https://last-minute-push.itch.io/survival-unleashed
WizardStyle=modern
;DefaultDirName={autopf}\My Example Program
DefaultDirName={autopf64}\FullSail\Survival Unleashed
DefaultGroupName=FullSail
; Pulls the icon from the installed exe to use in Control Panel listing
UninstallDisplayIcon={app}\Source\Survival Unleashed.exe
UninstallFilesDir={app}\.uninstall
Compression=lzma2
SolidCompression=yes
;OutputDir=userdocs:Inno Setup Examples Output 
OutputBaseFilename=SurvivalUnleashed-Installer

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}";

[Files]
Source: "C:\Packaged Build\Build\SurvivalUnleashed.exe"; DestDir: "{app}\";
Source: "C:\Packaged Build\Build\Engine\*"; DestDir: "{app}\Engine"; Flags: ignoreversion recursesubdirs
Source: "C:\Packaged Build\Build\SurvivalUnleashed\*"; DestDir: "{app}\SurvivalUnleashed"; Flags: ignoreversion recursesubdirs
Source: "C:\Packaged Build\Build\Content\SU.ico"; DestDir: "{app}\Content";

[Dirs]
Name: {app}\.uninstall; Attribs: hidden;

[Run]
Filename: "{app}\SurvivalUnleashed.exe"; Description: "Launch SurvivalUnleashed"; Flags: postinstall skipifsilent unchecked

[Icons]
; Uses an icon from a designated source
Name: "{autodesktop}\Survival Unleashed"; IconFilename:"{app}\Content\SU.ico"; Filename: "{app}\SurvivalUnleashed.exe"; Tasks: desktopicon