//------------------------------------------------------------------------------
// Header generated by wxDesigner from file: enviro.wdr
// Do not modify this file, all changes will be lost!
//------------------------------------------------------------------------------

#ifndef __WDR_enviro_H__
#define __WDR_enviro_H__

#ifdef __GNUG__
    #pragma interface "enviro_wdr.h"
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
#define ID_FLOATING 10009
#define ID_SOUND 10010
#define ID_HTML_PANE 10011
#define ID_PLANTSIZE 10012
#define ID_SHADOWS 10013
#define ID_OPENGL 10014
wxSizer *StartupDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SCENETREE 10015
#define ID_ENABLED 10016
#define ID_ZOOMTO 10017
#define ID_REFRESH 10018
wxSizer *SceneGraphFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_NOTEBOOK 10019
wxSizer *TParamsFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SPECIES 10020
#define ID_PLANT_HEIGHT_EDIT 10021
#define ID_HEIGHT_SLIDER 10022
#define ID_PLANT_INDIVIDUAL 10023
#define ID_PLANT_LINEAR 10024
#define ID_PLANT_CONTINUOUS 10025
#define ID_PLANT_VARIANCE_EDIT 10026
#define ID_PLANT_VARIANCE_SLIDER 10027
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
#define ID_LOD_VEG 10045
#define ID_SLIDER_VEG 10046
#define ID_LOD_STRUCT 10047
#define ID_SLIDER_STRUCT 10048
#define ID_LOD_ROAD 10049
#define ID_SLIDER_ROAD 10050
wxSizer *CameraDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

wxSizer *TextDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_STRUCTTYPE 10051
wxSizer *UtilDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_USE_GRID 10052
#define ID_FILENAME 10053
#define ID_LODMETHOD 10054
#define ID_PIXELERROR 10055
#define ID_TRICOUNT 10056
#define ID_TRISTRIPS 10057
#define ID_DETAILTEXTURE 10058
#define ID_USE_TIN 10059
#define ID_FILENAME_TIN 10060
#define ID_VERTEXAG 10061
wxSizer *TParams1Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_NONE 10062
#define ID_SINGLE 10063
#define ID_TFILESINGLE 10064
#define ID_DERIVED 10065
#define ID_TILED 10066
#define ID_TILESIZE 10067
#define ID_TFILEBASE 10068
#define ID_JPEG 10069
#define ID_TFILENAME 10070
#define ID_MIPMAP 10071
#define ID_16BIT 10072
#define ID_PRELIGHT 10073
#define ID_LIGHT_FACTOR 10074
#define ID_PRELIT 10075
wxSizer *TParams2Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_TREES 10076
#define ID_TREEFILE 10077
#define ID_VEGDISTANCE 10078
#define ID_ROADS 10079
#define ID_ROADFILE 10080
#define ID_ROADHEIGHT 10081
#define ID_ROADDISTANCE 10082
#define ID_TEXROADS 10083
#define ID_ROADCULTURE 10084
#define ID_HIGHWAYS 10085
#define ID_PAVED 10086
#define ID_DIRT 10087
#define ID_STRUCTFILES 10088
#define ID_STRUCT_DISTANCE 10089
#define ID_OCEANPLANE 10090
#define ID_OCEANPLANEOFFSET 10091
#define ID_DEPRESSOCEAN 10092
#define ID_DEPRESSOCEANOFFSET 10093
#define ID_SKY 10094
#define ID_SKYTEXTURE 10095
#define ID_HORIZON 10096
#define ID_FOG 10097
#define ID_FOG_DISTANCE 10098
wxSizer *TParams3Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_NAV_STYLE 10099
#define ID_MINHEIGHT 10100
#define ID_NAVSPEED 10101
#define ID_LOCFILE 10102
#define ID_INITTIME 10103
#define ID_TIMEMOVES 10104
#define ID_TIMESPEED 10105
#define ID_LABELS 10106
#define ID_LABEL_FILE 10107
#define ID_LABEL_FIELD 10108
#define ID_LABEL_HEIGHT 10109
#define ID_LABEL_SIZE 10110
wxSizer *TParams4Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

// Declare menubar functions

// Declare toolbar functions

// Declare bitmap functions

wxBitmap MyBitmapsFunc( size_t index );

#endif

// End of generated file
