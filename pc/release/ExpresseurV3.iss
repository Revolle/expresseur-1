; Expresseur V3

[Setup]
AppName=ExpresseurV3
AppVersion=3.0
DefaultDirName={pf}\ExpresseurV3
DefaultGroupName=ExpresseurV3
Compression=lzma2
SolidCompression=yes
OutputDir=.
OutputBaseFilename=setup_expresseurv3

[Components]
Name: "expresseur"; Description: "Expresseur"; Types: full compact ; Flags: fixed
Name: "music"; Description: "Samples score and chord-grid"; Types: full
Name: "sf2"; Description: "SF2-soundfont piano and guitar"; Types: full
Name: "asio4all"; Description: "Asio4all : generic ASIO audio driver for low latency"; Types: full
Name: "loopbe"; Description: "LoopBe : Virtual Midi-cable for software instrument"; Types: full

[Files]
Source: "instruments\*"; DestDir: "{userdocs}\ExpresseurV3\instruments\" ; Components : sf2
Source: "bin\*"; DestDir: "{app}" ; 
Source: "lua\*"; DestDir: "{app}\lua\" ; 
Source: "MNL folder\*"; DestDir: "{app}\MNL folder\" ; Flags: recursesubdirs
Source: "score\*"; DestDir: "{userdocs}\ExpresseurV3\" ; Components : music
Source: "Pizzicato.ttf"; DestDir: "{fonts}"; FontInstall: "Pizzicato"; Flags: onlyifdoesntexist

[Icons]
Name: "{group}\expresseurV3"; Filename: "{app}\expresseur.exe" ; IconFilename: "{app}\expresseur.ico"

[Run]
Filename: "{app}\vcredist_x86.exe"; StatusMsg: "Installing vcredist_x86 ..."  ; 
Filename: "{app}\ASIO4ALL_2_13_English.exe"; StatusMsg: "Installing asio4all ..." ;  Components: asio4all
Filename: "{app}\setuploopbe1.exe";  StatusMsg: "Installing loopbe ..." ;  Components: loopbe


