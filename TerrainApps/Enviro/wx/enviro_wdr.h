//------------------------------------------------------------------------------
// Header generated by wxDesigner from file: enviro.wdr
// Do not modify this file, all changes will be lost!
//------------------------------------------------------------------------------

#ifndef __WDR_enviro_H__
#define __WDR_enviro_H__

#ifdef __GNUG__
    #pragma interface "enviro_wdr.cpp"
#endif

// Include wxWindows' headers

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/image.h>
#include <wx/statline.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/grid.h>

// Declare window functions

#define ID_TEXT 10000
#define ID_EARTHVIEW 10001
#define ID_IMAGETEXT 10002
#define ID_IMAGE 10003
#define ID_TERRAIN 10004
#define ID_TNAME 10005
#define ID_TSELECT 10006
#define ID_EDITPROP 10007
#define ID_FULLSCREEN 10008
#define ID_GRAVITY 10009
#define ID_FLOATING 10010
#define ID_SOUND 10011
#define ID_HTML_PANE 10012
#define ID_VCURSOR 10013
#define ID_PLANTSIZE 10014
#define ID_SHADOWS 10015
#define ID_OPENGL 10016
wxSizer *StartupDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_TLIST 10017
#define ID_NEW 10018
#define ID_DELETE 10019
wxSizer *TerrainDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SCENETREE 10020
#define ID_ENABLED 10021
#define ID_ZOOMTO 10022
#define ID_REFRESH 10023
wxSizer *SceneGraphFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_NOTEBOOK 10024
wxSizer *TParamsFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SPECIES 10025
#define ID_SIZEEDIT 10026
#define ID_SIZESLIDER 10027
#define ID_PLANT_SPACING_EDIT 10028
wxSizer *PlantDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_LOCLIST 10029
#define ID_SAVE 10030
#define ID_LOAD 10031
#define ID_RECALL 10032
#define ID_STORE 10033
#define ID_STOREAS 10034
#define ID_REMOVE 10035
wxSizer *LocationDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_FOV 10036
#define ID_FOVSLIDER 10037
#define ID_NEAR 10038
#define ID_NEARSLIDER 10039
#define ID_FAR 10040
#define ID_FARSLIDER 10041
#define ID_LINE 10042
#define ID_SPEED 10043
#define ID_SPEEDSLIDER 10044
wxSizer *CameraDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

wxSizer *TextDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_STRUCTTYPE 10045
wxSizer *UtilDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_USE_GRID 10046
#define ID_FILENAME 10047
#define ID_REGULAR 10048
#define ID_SUBSAMPLE 10049
#define ID_VERTEXCOLORS 10050
#define ID_DYNAMIC 10051
#define ID_LODMETHOD 10052
#define ID_PIXELERROR 10053
#define ID_TRICOUNT 10054
#define ID_TRISTRIPS 10055
#define ID_DETAILTEXTURE 10056
#define ID_USE_TIN 10057
#define ID_FILENAME_TIN 10058
#define ID_VERTEXAG 10059
wxSizer *TParams1Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_NONE 10060
#define ID_SINGLE 10061
#define ID_TFILESINGLE 10062
#define ID_DERIVED 10063
#define ID_TILED 10064
#define ID_TILESIZE 10065
#define ID_TFILEBASE 10066
#define ID_JPEG 10067
#define ID_TFILENAME 10068
#define ID_MIPMAP 10069
#define ID_16BIT 10070
#define ID_PRELIGHT 10071
#define ID_LIGHT_FACTOR 10072
#define ID_PRELIT 10073
wxSizer *TParams2Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_TREES 10074
#define ID_TREEFILE 10075
#define ID_TREEDISTANCE 10076
#define ID_ROADS 10077
#define ID_ROADFILE 10078
#define ID_ROADHEIGHT 10079
#define ID_ROADDISTANCE 10080
#define ID_TEXROADS 10081
#define ID_ROADCULTURE 10082
#define ID_HIGHWAYS 10083
#define ID_PAVED 10084
#define ID_DIRT 10085
#define ID_BUILDINGS 10086
#define ID_BUILDINGFILE 10087
#define ID_OCEANPLANE 10088
#define ID_OCEANPLANEOFFSET 10089
#define ID_DEPRESSOCEAN 10090
#define ID_DEPRESSOCEANOFFSET 10091
#define ID_LABELS 10092
#define ID_SKY 10093
#define ID_SKYTEXTURE 10094
#define ID_HORIZON 10095
#define ID_AIRPORTS 10096
#define ID_FOG 10097
#define ID_FOG_DISTANCE 10098
wxSizer *TParams3Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_MINHEIGHT 10099
#define ID_NAVSPEED 10100
#define ID_LOCFILE 10101
#define ID_INITTIME 10102
#define ID_TIMEMOVES 10103
#define ID_TIMESPEED 10104
wxSizer *TParams4Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

// Declare menubar functions

// Declare toolbar functions

// Declare bitmap functions

wxBitmap MyBitmapsFunc( size_t index );

#endif

// End of generated file
