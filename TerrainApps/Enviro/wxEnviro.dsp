# Microsoft Developer Studio Project File - Name="wxEnviro" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=wxEnviro - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wxEnviro.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wxEnviro.mak" CFG="wxEnviro - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wxEnviro - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "wxEnviro - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "wxEnviro___Win32_Debug"
# PROP BASE Intermediate_Dir "wxEnviro___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug-wx"
# PROP Intermediate_Dir "Debug-wx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "\wx2\include" /I "\plib-1.2.0\src\ssg" /I "\plib-1.2.0\src\sg" /I "\TerrainSDK" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D DEBUG=1 /D "__WXDEBUG__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /FR /Yu"wx/wxprec.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\TerrainSDK" /D "_DEBUG" /D "__WXDEBUG__" /D "WIN32" /D "_WINDOWS" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /D VTLIB_OSG=1 /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /i "\wx2\include" /d "_DEBUG"
# ADD RSC /l 0x809 /i "\APIs\wx2\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxdlld.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib opengl32.lib glu32.lib ssg.lib sg.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libcid.lib" /out:"DebugDLL/isosurf.exe" /pdbtype:sept /libpath:"\wx2\lib" /libpath:"\plib-1.2.0\src\ssg\Debug" /libpath:"\plib-1.2.0\src\sg\Debug" /libpath:"\TerrainSDK\NetCDF"
# ADD LINK32 osg.lib osgDB.lib osgUtil.lib osgText.lib wx22_9d.lib opengl32.lib libpng.lib zlib.lib netcdf.lib gdal_i.lib gdi32.lib glu32.lib jpeg.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmtd.lib" /pdbtype:sept

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "wxEnviro___Win32_Release"
# PROP BASE Intermediate_Dir "wxEnviro___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release-wx"
# PROP Intermediate_Dir "Release-wx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O1 /Ob2 /I "../../../include" /I "../../../contrib/include" /I "\wx2\include" /I "\plib-1.2.0\src\ssg" /I "\plib-1.2.0\src\sg" /I "\TerrainSDK" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O1 /Ob2 /I "..\..\TerrainSDK" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /D VTLIB_OSG=1 /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /i "\wx2\include" /d "NDEBUG"
# ADD RSC /l 0x809 /i "\APIs\wx2\include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib opengl32.lib glu32.lib ssg.lib sg.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libci.lib" /out:"ReleaseDLL/isosurf.exe" /libpath:"\wx2\lib"
# ADD LINK32 osg.lib osgDB.lib osgUtil.lib osgText.lib wx22_9.lib gdi32.lib opengl32.lib libpng.lib zlib.lib netcdf.lib gdal_i.lib glu32.lib  jpeg.lib  /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"MSVCRTD.lib"

!ENDIF 

# Begin Target

# Name "wxEnviro - Win32 Debug"
# Name "wxEnviro - Win32 Release"
# Begin Group "Source-wx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wx\app.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wx\BuildingDlg3d.cpp
# End Source File
# Begin Source File

SOURCE=.\wx\CameraDlg.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wx\canvas.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wx\ChooseDlg.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wx\enviro_wdr.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wx\FenceDlg.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wx\frame.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wx\LocationDlg.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wx\PlantDlg.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wx\SceneGraphDlg.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wx\StartupDlg.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wx\TParamsDlg.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wx\UtilDlg.cpp
# End Source File
# End Group
# Begin Group "Headers-wx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wx\app.h
# End Source File
# Begin Source File

SOURCE=.\wx\BuildingDlg3d.h
# End Source File
# Begin Source File

SOURCE=.\wx\CameraDlg.h
# End Source File
# Begin Source File

SOURCE=.\wx\canvas.h
# End Source File
# Begin Source File

SOURCE=.\wx\ChooseDlg.h
# End Source File
# Begin Source File

SOURCE=.\wx\enviro_wdr.h
# End Source File
# Begin Source File

SOURCE=.\wx\FenceDlg.h
# End Source File
# Begin Source File

SOURCE=.\wx\frame.h
# End Source File
# Begin Source File

SOURCE=.\wx\LocationDlg.h
# End Source File
# Begin Source File

SOURCE=.\wx\menu_id.h
# End Source File
# Begin Source File

SOURCE=.\wx\PlantDlg.h
# End Source File
# Begin Source File

SOURCE=.\wx\SceneGraphDlg.h
# End Source File
# Begin Source File

SOURCE=.\wx\StartupDlg.h
# End Source File
# Begin Source File

SOURCE=.\wx\TParamsDlg.h
# End Source File
# Begin Source File

SOURCE=.\wx\UtilDlg.h
# End Source File
# End Group
# Begin Group "Source-vtlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Engines.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"vtlib/vtlib.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Enviro.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"vtlib/vtlib.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Globe.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"vtlib/vtlib.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Hawaii.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"vtlib/vtlib.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Nevada.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"vtlib/vtlib.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Options.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"vtlib/vtlib.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PTerrain.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"vtlib/vtlib.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TransitTerrain.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"vtlib/vtlib.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Vehicles.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"vtlib/vtlib.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Wings.cpp

!IF  "$(CFG)" == "wxEnviro - Win32 Debug"

# ADD CPP /YX"vtlib/vtlib.h"

!ELSEIF  "$(CFG)" == "wxEnviro - Win32 Release"

!ENDIF 

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

SOURCE=.\TransitTerrain.h
# End Source File
# Begin Source File

SOURCE=.\Vehicles.h
# End Source File
# Begin Source File

SOURCE=.\Wings.h
# End Source File
# End Group
# Begin Group "Resources-wx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wx\bitmap\camera.bmp
# End Source File
# Begin Source File

SOURCE=".\wx\enviro-wx.rc"
# End Source File
# Begin Source File

SOURCE=.\wx\Enviro.ico
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\fence.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\maintain.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\mondrian.ico
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\move.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\nav.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\nav_fast.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\nav_set.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\nav_slow.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\points.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\select.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\sgraph.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\space.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\sun.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\terrain.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\bitmap\tree.bmp
# End Source File
# End Group
# Begin Group "SceneGraph Icons"

# PROP Default_Filter ".ico"
# Begin Source File

SOURCE=.\wx\icons\camera.ico
# End Source File
# Begin Source File

SOURCE=.\wx\icons\engine.ico
# End Source File
# Begin Source File

SOURCE=.\wx\icons\geom.ico
# End Source File
# Begin Source File

SOURCE=.\wx\icons\group.ico
# End Source File
# Begin Source File

SOURCE=.\wx\icons\light.ico
# End Source File
# Begin Source File

SOURCE=.\wx\icons\lod.ico
# End Source File
# Begin Source File

SOURCE=.\wx\icons\mesh.ico
# End Source File
# Begin Source File

SOURCE=.\wx\icons\top.ico
# End Source File
# Begin Source File

SOURCE=.\wx\icons\unknown.ico
# End Source File
# Begin Source File

SOURCE=.\wx\icons\xform.ico
# End Source File
# End Group
# End Target
# End Project
