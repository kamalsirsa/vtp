# Microsoft Developer Studio Project File - Name="mfcEnviro" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=mfcEnviro - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mfcEnviro.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mfcEnviro.mak" CFG="mfcEnviro - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mfcEnviro - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "mfcEnviro - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mfcEnviro - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug-mfc"
# PROP Intermediate_Dir "Debug-mfc"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "\dism\inc" /I "\dism\terrain" /I "\dev\TFormats" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "ISM_OGL" /FR /YX"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\TerrainSDK" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "ENVIRO_MFC" /D VTLIB_OSG=1 /D "_AFXDLL" /FR /FD /c
# SUBTRACT CPP /u /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 appframed.lib terraind.lib XFrogCore.lib libfrog.lib libimage.lib glu32.lib opengl32.lib ltdis_n.lib ltfil_n.lib ltkrn_n.lib winmm.lib smldism.lib gconv32.lib \dev\TFormats\Debug\TFormatsd.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /pdbtype:sept /libpath:"\dism\xfrog\libs" /libpath:"\dism\libs"
# SUBTRACT BASE LINK32 /profile
# ADD LINK32 osg.lib osgDB.lib osgUtil.lib libpng.lib zlib.lib glu32.lib opengl32.lib winmm.lib ws2_32.lib netcdf.lib gdal_i.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /pdbtype:sept
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "mfcEnviro - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release-mfc"
# PROP Intermediate_Dir "Release-mfc"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "\dism\inc" /I "\dism\terrain" /I "\dev\TFormats" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "ISM_OGL" /YX"stdisland.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\..\TerrainSDK" /D "RELEASE" /D "WIN32" /D "_WINDOWS" /D "ENVIRO_MFC" /D VTLIB_OSG=1 /D "_AFXDLL" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "\dism\incui" /i "\dism\incui\mfc" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 appframe.lib terrain.lib glu32.lib opengl32.lib winmm.lib gconv32.lib XFrogCore.lib libfrog.lib libimage.lib \dev\TFormats\Release\TFormats.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /libpath:"\dism\libs" /libpath:"\dism\xfrog\libs"
# ADD LINK32 osg.lib osgDB.lib osgUtil.lib libpng.lib zlib.lib glu32.lib opengl32.lib winmm.lib netcdf.lib gdal_i.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib"

!ENDIF 

# Begin Target

# Name "mfcEnviro - Win32 Debug"
# Name "mfcEnviro - Win32 Release"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\mfc\res\Enviro.ico"
# End Source File
# Begin Source File

SOURCE=.\res\Enviro.ico
# End Source File
# Begin Source File

SOURCE=".\mfc\Enviro.rc"
# End Source File
# Begin Source File

SOURCE=.\res\Enviro.rc2
# End Source File
# Begin Source File

SOURCE=".\mfc\res\EnviroDoc.ico"
# End Source File
# Begin Source File

SOURCE=.\res\EnviroDoc.ico
# End Source File
# Begin Source File

SOURCE=".\mfc\res\Toolbar.bmp"
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\treeimag.bmp
# End Source File
# End Group
# Begin Group "Source-vtlib"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\Engines.cpp
# ADD CPP /YX"vtlib/vtlib.h"
# End Source File
# Begin Source File

SOURCE=.\Enviro.cpp
# ADD CPP /YX"vtlib/vtlib.h"
# End Source File
# Begin Source File

SOURCE=.\Globe.cpp
# ADD CPP /YX"vtlib/vtlib.h"
# End Source File
# Begin Source File

SOURCE=.\Hawaii.cpp
# ADD CPP /YX"vtlib/vtlib.h"
# End Source File
# Begin Source File

SOURCE=.\Nevada.cpp
# ADD CPP /YX"vtlib/vtlib.h"
# End Source File
# Begin Source File

SOURCE=.\Options.cpp
# ADD CPP /YX"vtlib/vtlib.h"
# End Source File
# Begin Source File

SOURCE=.\PTerrain.cpp
# ADD CPP /YX"vtlib/vtlib.h"
# End Source File
# Begin Source File

SOURCE=.\TransitTerrain.cpp
# ADD CPP /YX"vtlib/vtlib.h"
# End Source File
# Begin Source File

SOURCE=.\Vehicles.cpp
# ADD CPP /YX"vtlib/vtlib.h"
# End Source File
# Begin Source File

SOURCE=.\Wings.cpp
# ADD CPP /YX"vtlib/vtlib.h"
# End Source File
# End Group
# Begin Group "Source-mfc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\mfc\CameraDlg.cpp
# ADD CPP /YX"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=.\mfc\ChooseDlg.cpp
# ADD CPP /YX"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\CreateDlg.cpp"
# ADD CPP /YX"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\EnviroApp.cpp"
# ADD CPP /YX"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\EnviroFrame.cpp"
# ADD CPP /YX"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\EnviroView.cpp"
# ADD CPP /YX"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\FenceDlg.cpp"
# ADD CPP /YX"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\HtmlVw.cpp"
# ADD CPP /YX"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\PlantDlg.cpp"
# ADD CPP /YX"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=.\mfc\SimpleDoc.cpp
# ADD CPP /YX"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\StartupDlg.cpp"
# ADD CPP /YX"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\StatusDlg.cpp"
# ADD CPP /YX"StdAfx.h"
# End Source File
# End Group
# Begin Group "Source-none"

# PROP Default_Filter "cpp"
# End Group
# Begin Group "Headers-mfc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\mfc\CameraDlg.h
# End Source File
# Begin Source File

SOURCE=.\mfc\ChooseDlg.h
# End Source File
# Begin Source File

SOURCE=".\mfc\CreateDlg.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\EnviroApp.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\EnviroFrame.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\EnviroView.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\FenceDlg.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\HtmlVw.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\PlantDlg.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\resource.h"
# End Source File
# Begin Source File

SOURCE=.\mfc\SimpleDoc.h
# End Source File
# Begin Source File

SOURCE=".\mfc\StartupDlg.h"
# End Source File
# Begin Source File

SOURCE=".\mfc\StatusDlg.h"
# End Source File
# Begin Source File

SOURCE=.\mfc\StdAfx.h
# End Source File
# End Group
# Begin Group "Headers-vtlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CarEngine.h
# End Source File
# Begin Source File

SOURCE=.\Engines.h
# End Source File
# Begin Source File

SOURCE=.\Enviro.h
# End Source File
# Begin Source File

SOURCE=.\Globe.h
# End Source File
# Begin Source File

SOURCE=.\Hawaii.h
# End Source File
# Begin Source File

SOURCE=.\Nevada.h
# End Source File
# Begin Source File

SOURCE=.\Options.h
# End Source File
# Begin Source File

SOURCE=.\PTerrain.h
# End Source File
# Begin Source File

SOURCE=.\Vehicles.h
# End Source File
# Begin Source File

SOURCE=.\Wings.h
# End Source File
# End Group
# End Target
# End Project
