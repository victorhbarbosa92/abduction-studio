; Script de Instalacao Cirurgico - Abduction Studio V2
; Criado com Inno Setup 6

#define MyAppName "Abduction Studio V2"
#define MyAppVersion "2.0"
#define MyAppPublisher "Abduction Audio Labs"
#define MyAppExeName "AbductionStudioV2.exe"
#define SourceDir "C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

[Setup]
AppId={{8B5C08D7-1C34-4C23-8B37-7D581A051299}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir=C:\Users\o_vic\Desktop
OutputBaseFilename=Instalador_Abduction_Studio_V2
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
DisableProgramGroupPage=yes
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible

[Languages]
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
; 1. Executavel principal e dependencias de sistema
Source: "{#SourceDir}\AbductionStudioV2.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\AbductionUpdater.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\updater.json"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\ffmpeg.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\*.cer"; DestDir: "{app}"; Flags: ignoreversion

; 2. Scripts de auxilio, diagnostico e inicializacao rapida
Source: "{#SourceDir}\Iniciar_DAW.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\Diagnostico_e_Reparo.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\diagnostico.ps1"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\liberar_acesso.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\LEIA-ME_PRIMEIROS_PASSOS.txt"; DestDir: "{app}"; Flags: isreadme

; 3. Instalador oficial Microsoft Visual C++ 2015-2022 (extraido em tmp para auto-instalacao)
Source: "{#SourceDir}\vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall

; 4. Pastas essenciais de audio, UI e IA
Source: "{#SourceDir}\assets\*"; DestDir: "{app}\assets"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#SourceDir}\models\*"; DestDir: "{app}\models"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#SourceDir}\plugins\*"; DestDir: "{app}\plugins"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#SourceDir}\src\ui\*.ttf"; DestDir: "{app}\src\ui"; Flags: ignoreversion
Source: "{#SourceDir}\tools\*"; DestDir: "{app}\tools"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#SourceDir}\bin\*"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs createallsubdirs

; 5. Scratch (banco de samples/tracks de teste, excluindo renders temporarios pesados da pasta export)
Source: "{#SourceDir}\scratch\*"; DestDir: "{app}\scratch"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "tracks\export\*,*.zip,*.tmp"

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{autoprograms}\{#MyAppName} - Diagnostico e Auto-Reparo"; Filename: "{app}\Diagnostico_e_Reparo.bat"; WorkingDir: "{app}"
Name: "{autoprograms}\{#MyAppName} - Guia e Primeiros Passos"; Filename: "{app}\LEIA-ME_PRIMEIROS_PASSOS.txt"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
; Auto-instalacao do Microsoft Visual C++ Redistributable x64 se necessario
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/install /passive /norestart"; Check: VCRedistNeedsInstall; StatusMsg: "Instalando dependencias do Microsoft Visual C++ 2015-2022 (x64)..."; Flags: runhidden

; Desbloqueio de seguranca do Windows (Unblock-File) para prevenir bloqueios do SmartScreen
Filename: "powershell.exe"; Parameters: "-NoProfile -ExecutionPolicy Bypass -Command ""Get-ChildItem -Path '{app}' -Recurse | Unblock-File -ErrorAction SilentlyContinue"""; StatusMsg: "Configurando integridade e permissoes de seguranca..."; Flags: runhidden

; Opcao de iniciar imediatamente ao concluir
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
// Verifica se o Microsoft Visual C++ 2015-2022 x64 ja esta instalado no sistema
function VCRedistNeedsInstall: Boolean;
var
  Installed: Cardinal;
begin
  Result := True;
  if RegQueryDWordValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Installed', Installed) then
  begin
    if Installed = 1 then
      Result := False;
  end;
end;
