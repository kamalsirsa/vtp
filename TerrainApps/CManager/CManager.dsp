# Microsoft Developer Studio Project File - Name="CManager" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CManager - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CManager.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CManager.mak" CFG="CManager - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CManager - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CManager - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "CManager - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE "CManager - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CManager - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O1 /I "..\..\TerrainSDK" /I "\APIs" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__WXMSW__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /D VTLIB_OSG=1 /Yu"wx/wxprec.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wxmsw240.lib osg.lib osgDB.lib osgUtil.lib osgText.lib libpng.lib zlib.lib gdal_i.lib opengl32.lib jpeg.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "CManager - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\TerrainSDK" /I "\APIs" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WXMSW__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /D VTLIB_OSG=1 /FR /Yu"wx/wxprec.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "\APIs\wx2\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wxmsw240d.lib osgd.lib osgDBd.lib osgUtild.lib osgTextd.lib libpng.lib zlib.lib gdal_i.lib opengl32.lib jpeg.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"\APIs\plib-1.3.1\lib\Debug" /libpath:"\APIs\NetCDF" /libpath:"\APIs\libpng-1.0.8"

!ELSEIF  "$(CFG)" == "CManager - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "CManager___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "CManager___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /Zd /O1 /I "..\..\TerrainSDK" /I "\APIs" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__WXMSW__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /D VTLIB_OSG=1 /Yu"wx/wxprec.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O1 /I "..\..\TerrainSDK" /I "\APIs" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__WXMSW__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /D VTLIB_OSG=1 /D "_UNICODE" /D "UNICODE" /Yu"wx/wxprec.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmsw240.lib osg.lib osgDB.lib osgUtil.lib osgText.lib libpng.lib zlib.lib gdal_i.lib opengl32.lib jpeg.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 wxmsw240u.lib osg.lib osgDB.lib osgUtil.lib osgText.lib libpng.lib zlib.lib gdal_i.lib opengl32.lib jpeg.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /out:"Release_Unicode/CManager_u.exe"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "CManager - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "CManager___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "CManager___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\TerrainSDK" /I "\APIs" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WXMSW__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /D VTLIB_OSG=1 /FR /Yu"wx/wxprec.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\TerrainSDK" /I "\APIs" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WXMSW__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /D VTLIB_OSG=1 /D "_UNICODE" /D "UNICODE" /FR /Yu"wx/wxprec.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "\APIs\wx2\include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "\APIs\wx2\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmsw240d.lib osgd.lib osgDBd.lib osgUtild.lib osgTextd.lib libpng.lib zlib.lib gdal_i.lib opengl32.lib jpeg.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"\APIs\plib-1.3.1\lib\Debug" /libpath:"\APIs\NetCDF" /libpath:"\APIs\libpng-1.0.8"
# ADD LINK32 wxmsw240ud.lib osgd.lib osgDBd.lib osgUtild.lib osgTextd.lib libpng.lib zlib.lib gdal_i.lib opengl32.lib jpeg.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /out:"Debug_Unicode/CManager_u.exe" /pdbtype:sept /libpath:"\APIs\plib-1.3.1\lib\Debug" /libpath:"\APIs\NetCDF" /libpath:"\APIs\libpng-1.0.8"

!ENDIF 

# Begin Target

# Name "CManager - Win32 Release"
# Name "CManager - Win32 Debug"
# Name "CManager - Win32 Release Unicode"
# Name "CManager - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\app.cpp

!IF  "$(CFG)" == "CManager - Win32 Release"

# ADD CPP /Yu"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "CManager - Win32 Debug"

# ADD CPP /Yu

!ELSEIF  "$(CFG)" == "CManager - Win32 Release Unicode"

# ADD BASE CPP /Yu"wx/wxprec.h"
# ADD CPP /Yu"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "CManager - Win32 Debug Unicode"

# ADD BASE CPP /Yu
# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\canvas.cpp
# ADD CPP /Yu"wx/wxprec.h"
# End Source File
# Begin Source File

SOURCE=.\cmanager_wdr.cpp
# ADD CPP /Yu"wx/wxprec.h"
# End Source File
# Begin Source File

SOURCE=.\frame.cpp
# ADD CPP /Yu"wx/wxprec.h"
# End Source File
# Begin Source File

SOURCE=.\ModelDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\pre.cpp
# ADD CPP /Yc"wx/wxprec.h"
# End Source File
# Begin Source File

SOURCE=.\PropDlg.cpp

!IF  "$(CFG)" == "CManager - Win32 Release"

!ELSEIF  "$(CFG)" == "CManager - Win32 Debug"

# ADD CPP /Yu"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "CManager - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "CManager - Win32 Debug Unicode"

# ADD BASE CPP /Yu"wx/wxprec.h"
# ADD CPP /Yu"wx/wxprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SceneGraphDlg.cpp
# ADD CPP /Yu"wx/wxprec.h"
# End Source File
# Begin Source File

SOURCE=.\TagDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TreeView.cpp

!IF  "$(CFG)" == "CManager - Win32 Release"

!ELSEIF  "$(CFG)" == "CManager - Win32 Debug"

# ADD CPP /Yu"wx/wxprec.h"

!ELSEIF  "$(CFG)" == "CManager - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "CManager - Win32 Debug Unicode"

# ADD BASE CPP /Yu"wx/wxprec.h"
# ADD CPP /Yu"wx/wxprec.h"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\app.h
# End Source File
# Begin Source File

SOURCE=.\canvas.h
# End Source File
# Begin Source File

SOURCE=.\cmanager_wdr.h
# End Source File
# Begin Source File

SOURCE=.\frame.h
# End Source File
# Begin Source File

SOURCE=.\menu_id.h
# End Source File
# Begin Source File

SOURCE=.\ModelDlg.h
# End Source File
# Begin Source File

SOURCE=.\PropDlg.h
# End Source File
# Begin Source File

SOURCE=.\SceneGraphDlg.h
# End Source File
# Begin Source File

SOURCE=.\TagDlg.h
# End Source File
# Begin Source File

SOURCE=.\TreeView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\wx\msw\blank.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\bullseye.cur
# End Source File
# Begin Source File

SOURCE=.\icons\camera.ico
# End Source File
# Begin Source File

SOURCE=.\CManager.rc
# End Source File
# Begin Source File

SOURCE=.\icons\engine.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\error.ico
# End Source File
# Begin Source File

SOURCE=.\icons\geom.ico
# End Source File
# Begin Source File

SOURCE=.\icons\group.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\hand.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\info.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\item_rem.bmp
# End Source File
# Begin Source File

SOURCE=.\icons\light.ico
# End Source File
# Begin Source File

SOURCE=.\icons\lod.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\magnif1.cur
# End Source File
# Begin Source File

SOURCE=.\icons\mesh.ico
# End Source File
# Begin Source File

SOURCE=.\mondrian.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\noentry.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pbrush.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pencil.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pntleft.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pntright.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\query.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\question.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\roller.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\size.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\tip.ico
# End Source File
# Begin Source File

SOURCE=.\icons\top.ico
# End Source File
# Begin Source File

SOURCE=.\icons\unknown.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\warning.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\watch1.cur
# End Source File
# Begin Source File

SOURCE=.\icons\xform.ico
# End Source File
# End Group
# End Target
# End Project
