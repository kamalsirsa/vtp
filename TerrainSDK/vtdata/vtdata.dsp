# Microsoft Developer Studio Project File - Name="vtdata" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=vtdata - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vtdata.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vtdata.mak" CFG="vtdata - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vtdata - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "vtdata - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "vtdata - Win32 Debug Unicode" (based on "Win32 (x86) Static Library")
!MESSAGE "vtdata - Win32 Release Unicode" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TerrainSDK/vtdata", YCRAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vtdata - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
MTL=midl.exe
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".." /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "vtdata - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
MTL=midl.exe
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I ".." /D "_DEBUG" /D DEBUG=1 /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\vtdatad.lib"

!ELSEIF  "$(CFG)" == "vtdata - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "vtdata___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "vtdata___Win32_Debug_Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Target_Dir ""
MTL=midl.exe
F90=df.exe
# ADD BASE CPP /nologo /MDd /W3 /GX /Zi /Od /I ".." /D "_DEBUG" /D DEBUG=1 /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I ".." /D "_DEBUG" /D DEBUG=1 /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug\vtdatad.lib"
# ADD LIB32 /nologo /out:"Debug_Unicode\vtdataud.lib"

!ELSEIF  "$(CFG)" == "vtdata - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vtdata___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "vtdata___Win32_Release_Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Target_Dir ""
MTL=midl.exe
F90=df.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I ".." /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".." /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release_Unicode\vtdatau.lib"

!ENDIF 

# Begin Target

# Name "vtdata - Win32 Release"
# Name "vtdata - Win32 Debug"
# Name "vtdata - Win32 Debug Unicode"
# Name "vtdata - Win32 Release Unicode"
# Begin Group "Source"

# PROP Default_Filter ".cpp;.c"
# Begin Source File

SOURCE=.\Building.cpp
# End Source File
# Begin Source File

SOURCE=.\ByteOrder.cpp
# End Source File
# Begin Source File

SOURCE=.\Content.cpp
# End Source File
# Begin Source File

SOURCE=.\DLG.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\ElevationGrid.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\ElevationGridIO.cpp
# End Source File
# Begin Source File

SOURCE=.\Features.cpp
# End Source File
# Begin Source File

SOURCE=.\Fence.cpp
# End Source File
# Begin Source File

SOURCE=.\FilePath.cpp
# End Source File
# Begin Source File

SOURCE=.\Geodesic.cpp
# End Source File
# Begin Source File

SOURCE=.\GEOnet.cpp
# End Source File
# Begin Source File

SOURCE=.\HeightField.cpp
# End Source File
# Begin Source File

SOURCE=.\Icosa.cpp
# End Source File
# Begin Source File

SOURCE=.\LocalConversion.cpp
# End Source File
# Begin Source File

SOURCE=.\LULC.cpp
# End Source File
# Begin Source File

SOURCE=.\MathTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\Matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\Plants.cpp
# End Source File
# Begin Source File

SOURCE=.\PolyChecker.cpp
# End Source File
# Begin Source File

SOURCE=.\Projections.cpp
# End Source File
# Begin Source File

SOURCE=.\RoadMap.cpp
# End Source File
# Begin Source File

SOURCE=.\StructArray.cpp
# End Source File
# Begin Source File

SOURCE=.\StructImport.cpp
# End Source File
# Begin Source File

SOURCE=.\Structure.cpp
# End Source File
# Begin Source File

SOURCE=.\Triangulate.cpp
# End Source File
# Begin Source File

SOURCE=.\TripDub.cpp
# End Source File
# Begin Source File

SOURCE=.\Unarchive.cpp
# End Source File
# Begin Source File

SOURCE=.\UtilityMap.cpp
# End Source File
# Begin Source File

SOURCE=.\Vocab.cpp
# End Source File
# Begin Source File

SOURCE=.\vtDIB.cpp
# End Source File
# Begin Source File

SOURCE=.\vtLog.cpp
# End Source File
# Begin Source File

SOURCE=.\vtString.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\vtTin.cpp
# End Source File
# Begin Source File

SOURCE=.\WFSClient.cpp
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Array.h
# End Source File
# Begin Source File

SOURCE=.\Building.h
# End Source File
# Begin Source File

SOURCE=.\ByteOrder.h
# End Source File
# Begin Source File

SOURCE=.\config_vtdata.h
# End Source File
# Begin Source File

SOURCE=.\Content.h
# End Source File
# Begin Source File

SOURCE=.\DLG.h
# End Source File
# Begin Source File

SOURCE=.\ElevationGrid.h
# End Source File
# Begin Source File

SOURCE=.\Features.h
# End Source File
# Begin Source File

SOURCE=.\Fence.h
# End Source File
# Begin Source File

SOURCE=.\FilePath.h
# End Source File
# Begin Source File

SOURCE=.\GEOnet.h
# End Source File
# Begin Source File

SOURCE=.\HeightField.h
# End Source File
# Begin Source File

SOURCE=.\Icosa.h
# End Source File
# Begin Source File

SOURCE=.\LocalConversion.h
# End Source File
# Begin Source File

SOURCE=.\LULC.h
# End Source File
# Begin Source File

SOURCE=.\MathTypes.h
# End Source File
# Begin Source File

SOURCE=.\Plants.h
# End Source File
# Begin Source File

SOURCE=.\PolyChecker.h
# End Source File
# Begin Source File

SOURCE=.\Projections.h
# End Source File
# Begin Source File

SOURCE=.\RoadMap.h
# End Source File
# Begin Source File

SOURCE=.\Selectable.h
# End Source File
# Begin Source File

SOURCE=.\StatePlane.h
# End Source File
# Begin Source File

SOURCE=.\StructArray.h
# End Source File
# Begin Source File

SOURCE=.\Structure.h
# End Source File
# Begin Source File

SOURCE=.\Triangulate.h
# End Source File
# Begin Source File

SOURCE=.\TripDub.h
# End Source File
# Begin Source File

SOURCE=.\Unarchive.h
# End Source File
# Begin Source File

SOURCE=.\UtilityMap.h
# End Source File
# Begin Source File

SOURCE=.\Vocab.h
# End Source File
# Begin Source File

SOURCE=.\vtDIB.h
# End Source File
# Begin Source File

SOURCE=.\vtLog.h
# End Source File
# Begin Source File

SOURCE=.\vtString.h
# End Source File
# Begin Source File

SOURCE=.\vtTin.h
# End Source File
# End Group
# Begin Group "Boost"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\boost\directory.cpp
# End Source File
# Begin Source File

SOURCE=.\boost\directory.h
# End Source File
# End Group
# End Target
# End Project
