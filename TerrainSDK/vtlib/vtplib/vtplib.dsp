# Microsoft Developer Studio Project File - Name="vtplib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=vtplib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vtplib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vtplib.mak" CFG="vtplib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vtplib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "vtplib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vtplib - Win32 Release"

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
F90=df.exe
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "\TerrainSDK" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D VTLIB_PLIB=1 /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "vtplib - Win32 Debug"

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
F90=df.exe
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "\TerrainSDK" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D VTLIB_PLIB=1 /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "vtplib - Win32 Release"
# Name "vtplib - Win32 Debug"
# Begin Group "Core Source"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\core\AttribMap.cpp
# End Source File
# Begin Source File

SOURCE=..\core\BryanTerrain.cpp
# End Source File
# Begin Source File

SOURCE=..\core\Building3d.cpp
# End Source File
# Begin Source File

SOURCE=..\core\CoreMeshMat.cpp
# End Source File
# Begin Source File

SOURCE=..\core\CoreScene.cpp
# End Source File
# Begin Source File

SOURCE=..\core\CustomTerrain.cpp
# End Source File
# Begin Source File

SOURCE=..\core\DynTerrain.cpp
# End Source File
# Begin Source File

SOURCE=..\core\Engine.cpp
# End Source File
# Begin Source File

SOURCE=..\core\Fences.cpp
# End Source File
# Begin Source File

SOURCE=..\core\FrameTimer.cpp
# End Source File
# Begin Source File

SOURCE=..\core\HeightField.cpp
# End Source File
# Begin Source File

SOURCE=..\core\IntersectionEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\core\LKTerrain.cpp
# End Source File
# Begin Source File

SOURCE=..\core\LocalGrid.cpp
# End Source File
# Begin Source File

SOURCE=..\core\LocalProjection.cpp
# End Source File
# Begin Source File

SOURCE=..\core\Location.cpp
# End Source File
# Begin Source File

SOURCE=..\core\LodGrid.cpp
# End Source File
# Begin Source File

SOURCE=..\core\NavEngines.cpp
# End Source File
# Begin Source File

SOURCE=..\core\Roads.cpp
# End Source File
# Begin Source File

SOURCE=..\core\SkyDome.cpp
# End Source File
# Begin Source File

SOURCE=..\core\SMTerrain.cpp
# End Source File
# Begin Source File

SOURCE=..\core\Terrain.cpp
# End Source File
# Begin Source File

SOURCE=..\core\TerrainPatch.cpp
# End Source File
# Begin Source File

SOURCE=..\core\TerrainScene.cpp
# End Source File
# Begin Source File

SOURCE=..\core\TerrainSurface.cpp
# End Source File
# Begin Source File

SOURCE=..\core\TimeEngines.cpp
# End Source File
# Begin Source File

SOURCE=..\core\TParams.cpp
# End Source File
# Begin Source File

SOURCE=..\core\Trees.cpp
# End Source File
# Begin Source File

SOURCE=..\core\TVTerrain.cpp
# End Source File
# Begin Source File

SOURCE=..\core\vtSOG.cpp
# End Source File
# Begin Source File

SOURCE=..\core\vtString.cpp
# End Source File
# End Group
# Begin Group "Core Headers"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\core\AttribMap.h
# End Source File
# Begin Source File

SOURCE=..\core\Base.h
# End Source File
# Begin Source File

SOURCE=..\core\BryanTerrain.h
# End Source File
# Begin Source File

SOURCE=..\core\Building3d.h
# End Source File
# Begin Source File

SOURCE=..\core\CustomTerrain.h
# End Source File
# Begin Source File

SOURCE=..\core\DynTerrain.h
# End Source File
# Begin Source File

SOURCE=..\core\Engine.h
# End Source File
# Begin Source File

SOURCE=..\core\Fences.h
# End Source File
# Begin Source File

SOURCE=..\core\FP8.h
# End Source File
# Begin Source File

SOURCE=..\core\FrameTimer.h
# End Source File
# Begin Source File

SOURCE=..\core\IntersectionEngine.h
# End Source File
# Begin Source File

SOURCE=..\core\LKTerrain.h
# End Source File
# Begin Source File

SOURCE=..\core\LocalGrid.h
# End Source File
# Begin Source File

SOURCE=..\core\LocalProjection.h
# End Source File
# Begin Source File

SOURCE=..\core\Location.h
# End Source File
# Begin Source File

SOURCE=..\core\LodGrid.h
# End Source File
# Begin Source File

SOURCE=..\core\NavEngines.h
# End Source File
# Begin Source File

SOURCE=..\core\Roads.h
# End Source File
# Begin Source File

SOURCE=..\core\SkyDome.h
# End Source File
# Begin Source File

SOURCE=..\core\SMTerrain.h
# End Source File
# Begin Source File

SOURCE=..\core\Terrain.h
# End Source File
# Begin Source File

SOURCE=..\core\TerrainPatch.h
# End Source File
# Begin Source File

SOURCE=..\core\TerrainScene.h
# End Source File
# Begin Source File

SOURCE=..\core\TerrainSurface.h
# End Source File
# Begin Source File

SOURCE=..\core\TimeEngines.h
# End Source File
# Begin Source File

SOURCE=..\core\TParams.h
# End Source File
# Begin Source File

SOURCE=..\core\Trees.h
# End Source File
# Begin Source File

SOURCE=..\core\TVTerrain.h
# End Source File
# Begin Source File

SOURCE=..\vtlib.h
# End Source File
# Begin Source File

SOURCE=..\core\vtSOG.h
# End Source File
# Begin Source File

SOURCE=..\core\vtString.h
# End Source File
# End Group
# Begin Group "PLIB Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ImageSSG.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshMat.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeSSG.cpp
# End Source File
# Begin Source File

SOURCE=.\SceneSSG.cpp
# End Source File
# End Group
# Begin Group "PLIB Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ImageSSG.h
# End Source File
# Begin Source File

SOURCE=.\MeshMat.h
# End Source File
# Begin Source File

SOURCE=.\NodeSSG.h
# End Source File
# Begin Source File

SOURCE=.\SceneSSG.h
# End Source File
# End Group
# End Target
# End Project
