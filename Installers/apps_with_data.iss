; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=VTP Software
AppVerName=VTP Software 2006.09.22
AppPublisher=Virtual Terrain Project
AppPublisherURL=http://vterrain.org/
AppSupportURL=http://vterrain.org/
AppUpdatesURL=http://vterrain.org/
DefaultDirName={pf}\VTP
DefaultGroupName=VTP
AllowNoIcons=yes
LicenseFile=C:\VTP\Installers\license.txt
OutputBaseFilename=setup_full
OutputDir=C:\Distrib
; We need the following because Windows won't turn Registry settings into Enviroment variables w/o a reboot
AlwaysRestart=yes

[Types]
Name: "standard"; Description: "Standard installation"; Flags: iscustom

[Components]
Name: "main"; Description: "The VTP applications"; Types: standard
Name: "data"; Description: "Data used by the applications"; Types: standard
Name: "docs"; Description: "Documentation for the applications"; Types: standard
Name: "proj"; Description: "Data files for coordinate systems (GDAL/PROJ.4)"; Types: standard
Name: "dlls"; Description: "Third-party DLL files (wxWidgets, OSG, etc.)"; Types: standard

[Tasks]
Name: env; Description: "Set environment variables for coordinate system data files"; GroupDescription: "Environment variables:"; Components: proj

[Registry]
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: string; ValueName: "GDAL_DATA"; ValueData: "{app}\GDAL-data"; Components: proj
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: string; ValueName: "PROJ_LIB"; ValueData: "{app}\PROJ4-data"; Components: proj

[Files]
Source: "C:\VTP\TerrainApps\BExtractor\license.txt"; DestDir: "{app}/Apps"; Flags: ignoreversion; Components: main

Source: "C:\VTP\TerrainApps\BExtractor\Release\BExtractor.exe"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\BExtractor\BE.ini"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\BExtractor\Docs\index.html"; DestDir: "{app}/Docs/BExtractor"; Flags: ignoreversion; Components: docs

Source: "C:\VTP\TerrainApps\CManager\Release_Unicode\CManager_u.exe"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\CManager\itemtypes.txt"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\CManager\tags.txt"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\CManager\Docs\index.html"; DestDir: "{app}/Docs/CManager"; Flags: ignoreversion; Components: docs
Source: "C:\VTP\TerrainApps\CManager\Docs\images\*.png"; DestDir: "{app}/Docs/CManager/images"; Flags: ignoreversion; Components: docs

Source: "C:\VTP\TerrainApps\Enviro\Release_Unicode\wxEnviro_u.exe"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\Ship\Enviro.xml"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\Docs\*.html"; DestDir: "{app}\Docs\Enviro"; Flags: ignoreversion; Components: docs
Source: "C:\VTP\TerrainApps\Enviro\Docs\Navigation\*"; DestDir: "{app}\Docs\Enviro\Navigation"; Flags: ignoreversion; Components: docs
Source: "C:\VTP\TerrainApps\Enviro\Docs\images\*"; DestDir: "{app}\Docs\Enviro\images"; Flags: ignoreversion; Components: docs

Source: "C:\VTP\TerrainApps\VTBuilder\Release_Unicode\VTBuilder_u.exe"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\VTBuilder\VTBuilder.ini"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\VTBuilder\Docs\*.html"; DestDir: "{app}\Docs\VTBuilder"; Flags: ignoreversion; Components: docs
Source: "C:\VTP\TerrainApps\VTBuilder\Docs\images\*"; DestDir: "{app}\Docs\VTBuilder\images"; Flags: ignoreversion; Components: docs

; Translation files
Source: "C:\VTP\TerrainApps\Enviro\af\Enviro.mo"; DestDir: "{app}\Apps\af"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\de\Enviro.mo"; DestDir: "{app}\Apps\de"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\en_GB\Enviro.mo"; DestDir: "{app}\Apps\en_GB"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\es\Enviro.mo"; DestDir: "{app}\Apps\es"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\fr\Enviro.mo"; DestDir: "{app}\Apps\fr"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\hu\Enviro.mo"; DestDir: "{app}\Apps\hu"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\it\Enviro.mo"; DestDir: "{app}\Apps\it"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\pt\Enviro.mo"; DestDir: "{app}\Apps\pt"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\ro\Enviro.mo"; DestDir: "{app}\Apps\ro"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\sv\Enviro.mo"; DestDir: "{app}\Apps\sv"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\tr\Enviro.mo"; DestDir: "{app}\Apps\tr"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\Enviro\zh\Enviro.mo"; DestDir: "{app}\Apps\zh"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\VTBuilder\fr\VTBuilder.mo"; DestDir: "{app}\Apps\fr"; Flags: ignoreversion; Components: main
Source: "C:\VTP\TerrainApps\VTBuilder\zh\VTBuilder.mo"; DestDir: "{app}\Apps\zh"; Flags: ignoreversion; Components: main

; Core Data
Source: "C:\VTP\TerrainApps\VTBuilder\WorldMap\gnv19.*"; DestDir: "{app}\Apps\WorldMap"; Flags: ignoreversion; Components: data
Source: "G:\Data-Distro\*"; DestDir: "{app}\Data"; Flags: ignoreversion recursesubdirs; Components: data

; overview docs
Source: "C:\VTP\Docs\*.html"; DestDir: "{app}\Docs"; Flags: ignoreversion; Components: docs

; Projection Stuff
Source: "C:\APIs\gdal132\data\*"; DestDir: "{app}\GDAL-data"; Flags: ignoreversion; Components: proj
Source: "C:\VTP\proj\nad\*"; DestDir: "{app}\PROJ4-data"; Flags: ignoreversion; Components: proj

; DLLs
Source: "C:\APIs\bzip2-1.0.3-bin\bzip2.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\gdal132\bin\gdal13.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\gdal132\bin\proj.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\gdal132\bin\*.exe"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\netcdf-3.5.0.win32bin\bin\*.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\osg-1.2-bin\bin-rel-only\*.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\libcurl-7.15.0\libcurl.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\wx2.6.3\lib\vc_dll\wxbase26u_vc_custom.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\wx2.6.3\lib\vc_dll\wxmsw26u_core_vc_custom.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\wx2.6.3\lib\vc_dll\wxmsw26u_gl_vc_custom.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\xerces-c_2_3_0-win32\bin\xerces-c_2_3_0.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\libpng-1.2.8\libpng13.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls
Source: "C:\APIs\libpng-1.2.8\zlib1.dll"; DestDir: "{app}\Apps"; Flags: ignoreversion; Components: dlls

; Microsoft DLLs
Source: "C:\VTP\Installers\Redistributable_MS_DLLs\msvcp71.dll"; DestDir: "{sys}"; Flags: onlyifdoesntexist uninsneveruninstall; Components: dlls
Source: "C:\VTP\Installers\Redistributable_MS_DLLs\msvcr71.dll"; DestDir: "{sys}"; Flags: onlyifdoesntexist uninsneveruninstall; Components: dlls

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Documentation"; Filename: "{app}\Docs\index.html"
Name: "{group}\BExtractor"; Filename: "{app}\Apps\BExtractor.exe"; WorkingDir: "{app}\Apps"
Name: "{group}\CManager"; Filename: "{app}\Apps\CManager_u.exe"; WorkingDir: "{app}\Apps"
Name: "{group}\Enviro"; Filename: "{app}\Apps\wxEnviro_u.exe"; WorkingDir: "{app}\Apps"
Name: "{group}\VTBuilder"; Filename: "{app}\Apps\VTBuilder_u.exe"; WorkingDir: "{app}\Apps"
Name: "{group}\Uninstall VTP Software"; Filename: "{uninstallexe}"

