# Microsoft Developer Studio Project File - Name="glutSimple" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=glutSimple - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "glutSimple.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "glutSimple.mak" CFG="glutSimple - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "glutSimple - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "glutSimple - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "glutSimple - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\..\TerrainSDK" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D VTLIB_OSG=1 /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 osg.lib osgDB.lib osgUtil.lib netcdf.lib libpng.lib zlib.lib gdal_i.lib /nologo /subsystem:console /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcmt.lib" /libpath:"\APIs\libpng-1.0.8" /libpath:"\APIs\netcdf-3.5.0.win32bin\lib"

!ELSEIF  "$(CFG)" == "glutSimple - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "glutSimple___Win32_Debug"
# PROP BASE Intermediate_Dir "glutSimple___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /I "..\..\TerrainSDK" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D VTLIB_OSG=1 /FR /YX /FD /Zm1000 /GZ /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 osg.lib osgDB.lib osgUtil.lib netcdf.lib libpng.lib zlib.lib gdal_i.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /pdbtype:sept /libpath:"\APIs\libpng-1.0.8" /libpath:"\APIs\netcdf-3.5.0.win32bin\lib"
# SUBTRACT LINK32 /verbose

!ENDIF 

# Begin Target

# Name "glutSimple - Win32 Release"
# Name "glutSimple - Win32 Debug"
# Begin Source File

SOURCE=.\app.cpp
# End Source File
# End Target
# End Project
