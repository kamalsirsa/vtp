# Microsoft Developer Studio Project File - Name="VTBuilder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=VTBuilder - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "VTBuilder.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "VTBuilder.mak" CFG="VTBuilder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "VTBuilder - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "VTBuilder - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/TerrainApps/VTBuilder", PUPBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "VTBuilder - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "VTBuilder___Win32_Release"
# PROP BASE Intermediate_Dir "VTBuilder___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O1 /Ob2 /I "\wx2\include" /I "\TerrainSDK\vtdata" /I "\TerrainSDK\ProjectionLib" /I "\TerrainSDK\shapelib" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /Yu"wx/wxprec.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /Ob2 /I "..\..\TerrainSDK" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /D SUPPORT_TRANSIT=0 /Yu"wx/wxprec.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /i "\APIs\wx2\include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 gdi32.lib wxdll.lib netcdfs.lib vtdata.lib ProjectionLib.lib gctpc.lib Shapelib.lib io.lib container.lib builder.lib ws2_32.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libci.lib" /libpath:"\TerrainSDK\vtdata\Release" /libpath:"\wx2\lib" /libpath:"\TerrainSDK\NetCDF" /libpath:"\TerrainSDK\ProjectionLib\Release" /libpath:"\TerrainSDK\gctpc\Release" /libpath:"\TerrainSDK\shapelib\Release" /libpath:"\TerrainSDK\sdtsxx\builder\Release" /libpath:"\TerrainSDK\sdtsxx\container\Release" /libpath:"\TerrainSDK\sdtsxx\io\Release"
# ADD LINK32 wx22_9.lib gdi32.lib ws2_32.lib netcdf.lib gdal_i.lib libpng.lib zlib.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /libpath:"\APIs\wx2\lib" /libpath:"\APIs\NetCDF" /libpath:"\APIs\netcdf-3.5.0.win32bin\lib"

!ELSEIF  "$(CFG)" == "VTBuilder - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "VTBuilder___Win32_Debug"
# PROP BASE Intermediate_Dir "VTBuilder___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "\APIs\wx2\include" /I "\TerrainSDK" /I "\TerrainSDK\shapelib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D DEBUG=1 /D "__WXDEBUG__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /FR /Yu"wx/wxprec.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\TerrainSDK" /D "_DEBUG" /D DEBUG=1 /D "__WXDEBUG__" /D "WIN32" /D "_WINDOWS" /D "__WINDOWS__" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /D SUPPORT_TRANSIT=0 /FR /Yu"wx/wxprec.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /i "\APIs\wx2\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wx22_1d.lib netcdfs.lib vtdatad.lib ProjectionLib.lib gctpc.lib gdi32.lib Shapelib.lib io.lib container.lib builder.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcmt.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcpd.lib" /pdbtype:sept /libpath:"\TerrainSDK\vtdata\Debug" /libpath:"\APIs\wx2\lib" /libpath:"\TerrainSDK\NetCDF" /libpath:"\TerrainSDK\ProjectionLib\Debug" /libpath:"\TerrainSDK\gctpc\Debug" /libpath:"\TerrainSDK\shapelib\Debug" /libpath:"\TerrainSDK\sdtsxx\builder\Debug" /libpath:"\TerrainSDK\sdtsxx\container\Debug" /libpath:"\TerrainSDK\sdtsxx\io\Debug"
# ADD LINK32 wx22_9d.lib gdi32.lib ws2_32.lib netcdf.lib gdal_i.lib libpng.lib zlib.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"\APIs\wx2\lib" /libpath:"\APIs\NetCDF" /libpath:"\APIs\netcdf-3.5.0.win32bin\lib"

!ENDIF 

# Begin Target

# Name "VTBuilder - Win32 Release"
# Name "VTBuilder - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\App.cpp

!IF  "$(CFG)" == "VTBuilder - Win32 Release"

!ELSEIF  "$(CFG)" == "VTBuilder - Win32 Debug"

# ADD BASE CPP /Yu"wx/wxprec.h"
# ADD CPP /Yu"wx/wxprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AutoDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\BuilderView.cpp
# End Source File
# Begin Source File

SOURCE=.\BuildingDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DistribVegDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ElevLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\ExtentDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Frame.cpp

!IF  "$(CFG)" == "VTBuilder - Win32 Release"

!ELSEIF  "$(CFG)" == "VTBuilder - Win32 Debug"

# ADD BASE CPP /Yu"wx/wxprec.h"
# ADD CPP /Yu"wx/wxprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FrameMenus.cpp
# End Source File
# Begin Source File

SOURCE=.\Helper.cpp
# End Source File
# Begin Source File

SOURCE=.\Import.cpp
# End Source File
# Begin Source File

SOURCE=.\ImportVegDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Layer.cpp
# End Source File
# Begin Source File

SOURCE=.\LayerPropDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\pre.cpp
# ADD BASE CPP /Yc"wx/wxprec.h"
# ADD CPP /Yc"wx/wxprec.h"
# End Source File
# Begin Source File

SOURCE=.\Projection2Dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RawDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RawLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\ResampleDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RoadDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RoadFix.cpp
# End Source File
# Begin Source File

SOURCE=.\RoadLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\RoadMapEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\RoadMapIO.cpp
# End Source File
# Begin Source File

SOURCE=.\ScaledView.cpp
# End Source File
# Begin Source File

SOURCE=.\StatePlaneDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StatusBar.cpp
# End Source File
# Begin Source File

SOURCE=.\StructLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\TransitLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\TreeView.cpp
# End Source File
# Begin Source File

SOURCE=.\UtilityLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\VegDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\VegLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\VTBuilder_wdr.cpp
# End Source File
# Begin Source File

SOURCE=.\WaterLayer.cpp
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\app.h
# End Source File
# Begin Source File

SOURCE=.\AutoDialog.h
# End Source File
# Begin Source File

SOURCE=.\BuilderView.h
# End Source File
# Begin Source File

SOURCE=.\BuildingDlg.h
# End Source File
# Begin Source File

SOURCE=.\DistribVegDlg.h
# End Source File
# Begin Source File

SOURCE=.\ElevLayer.h
# End Source File
# Begin Source File

SOURCE=.\ExtentDlg.h
# End Source File
# Begin Source File

SOURCE=.\Frame.h
# End Source File
# Begin Source File

SOURCE=.\Helper.h
# End Source File
# Begin Source File

SOURCE=.\ImportVegDlg.h
# End Source File
# Begin Source File

SOURCE=.\Layer.h
# End Source File
# Begin Source File

SOURCE=.\LayerPropDlg.h
# End Source File
# Begin Source File

SOURCE=.\MenuEnum.h
# End Source File
# Begin Source File

SOURCE=.\NodeDlg.h
# End Source File
# Begin Source File

SOURCE=.\Projection2Dlg.h
# End Source File
# Begin Source File

SOURCE=.\ProjectionDlg.h
# End Source File
# Begin Source File

SOURCE=.\RawDlg.h
# End Source File
# Begin Source File

SOURCE=.\RawLayer.h
# End Source File
# Begin Source File

SOURCE=.\ResampleDlg.h
# End Source File
# Begin Source File

SOURCE=.\RoadDlg.h
# End Source File
# Begin Source File

SOURCE=.\RoadLayer.h
# End Source File
# Begin Source File

SOURCE=.\RoadMapEdit.h
# End Source File
# Begin Source File

SOURCE=.\ScaledView.h
# End Source File
# Begin Source File

SOURCE=.\splitterwindow.h
# End Source File
# Begin Source File

SOURCE=.\StatePlaneDlg.h
# End Source File
# Begin Source File

SOURCE=.\statusbar.h
# End Source File
# Begin Source File

SOURCE=.\StructLayer.h
# End Source File
# Begin Source File

SOURCE=.\TransitLayer.h
# End Source File
# Begin Source File

SOURCE=.\TreeView.h
# End Source File
# Begin Source File

SOURCE=.\UtilityLayer.h
# End Source File
# Begin Source File

SOURCE=.\VegDlg.h
# End Source File
# Begin Source File

SOURCE=.\VegLayer.h
# End Source File
# Begin Source File

SOURCE=.\VTBuilder_wdr.h
# End Source File
# Begin Source File

SOURCE=.\WaterLayer.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "rc;cur;ico"
# Begin Source File

SOURCE=.\cursors\panhand.cur
# End Source File
# Begin Source File

SOURCE=.\icons\VTBuilder.ico
# End Source File
# Begin Source File

SOURCE=.\VTBuilder.rc
# ADD BASE RSC /l 0x409 /i "\wx2\include"
# ADD RSC /l 0x409 /i "\wx2\include"
# End Source File
# End Group
# Begin Group "Toolbar Bitmap"

# PROP Default_Filter "bmp"
# Begin Source File

SOURCE=.\bitmaps\bld_edit.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\distance.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\edit_crossing.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\edit_delete.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\edit_offset.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\elev_box.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\hand.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\layer_export.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\layer_import.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\layer_new.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\layer_open.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\layer_save.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\layer_show.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\loadimage.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\mergeelev.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\minus.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\newproj.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\openproj.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\plus.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\rd_direction.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\rd_edit.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\rd_select_node.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\rd_select_road.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\rd_select_whole.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\rd_shownodes.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\saveproj.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\select.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\view_mag.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\zoomall.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\zoomexact.bmp
# End Source File
# End Group
# Begin Group "TreeView Icons"

# PROP Default_Filter "ico"
# Begin Source File

SOURCE=.\icons\building.ico
# End Source File
# Begin Source File

SOURCE=.\icons\file1.ico
# End Source File
# Begin Source File

SOURCE=.\icons\file2.ico
# End Source File
# Begin Source File

SOURCE=.\icons\folder1.ico
# End Source File
# Begin Source File

SOURCE=.\icons\folder2.ico
# End Source File
# Begin Source File

SOURCE=.\icons\folder3.ico
# End Source File
# Begin Source File

SOURCE=.\icons\grid.ico
# End Source File
# Begin Source File

SOURCE=.\icons\image.ico
# End Source File
# Begin Source File

SOURCE=.\icons\raw.ico
# End Source File
# Begin Source File

SOURCE=.\icons\road.ico
# End Source File
# Begin Source File

SOURCE=.\icons\veg1.ico
# End Source File
# Begin Source File

SOURCE=.\icons\water.ico
# End Source File
# End Group
# End Target
# End Project
