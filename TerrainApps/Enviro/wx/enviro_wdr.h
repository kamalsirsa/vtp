//------------------------------------------------------------------------------
// Header generated by wxDesigner from file: enviro.wdr
// Do not modify this file, all changes will be lost!
//------------------------------------------------------------------------------

#ifndef __WDR_enviro_H__
#define __WDR_enviro_H__

#if defined(__GNUG__) && !defined(__APPLE__)
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
#include <wx/toolbar.h>

// Declare window functions

#define ID_TEXT 10000
#define ID_EARTHVIEW 10001
#define ID_IMAGETEXT 10002
#define ID_IMAGE 10003
#define ID_TERRAIN 10004
#define ID_TNAME 10005
#define ID_EDITPROP 10006
#define ID_TERRMAN 10007
#define ID_OPTIONS 10008
#define ID_OPENGL 10009
wxSizer *StartupDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SCENETREE 10010
#define ID_ENABLED 10011
#define ID_ZOOMTO 10012
#define ID_REFRESH 10013
wxSizer *SceneGraphFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_NOTEBOOK 10014
wxSizer *TParamsFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SPECIES 10015
#define ID_COMMON_NAMES 10016
#define ID_LANGUAGE 10017
#define ID_PLANT_HEIGHT_EDIT 10018
#define ID_HEIGHT_SLIDER 10019
#define ID_PLANT_INDIVIDUAL 10020
#define ID_PLANT_LINEAR 10021
#define ID_PLANT_CONTINUOUS 10022
#define ID_PLANT_VARIANCE_EDIT 10023
#define ID_PLANT_VARIANCE_SLIDER 10024
#define ID_PLANT_SPACING_EDIT 10025
wxSizer *PlantDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_LOCLIST 10026
#define ID_SAVE 10027
#define ID_LOAD 10028
#define ID_RECALL 10029
#define ID_STORE 10030
#define ID_STOREAS 10031
#define ID_REMOVE 10032
#define ID_LINE 10033
#define ID_ANIMS 10034
#define ID_NEW_ANIM 10035
#define ID_SAVE_ANIM 10036
#define ID_LOAD_ANIM 10037
#define ID_RESET 10038
#define ID_STOP 10039
#define ID_RECORD1 10040
#define ID_PLAY 10041
#define ID_ANIM_POS 10042
#define ID_ACTIVE 10043
#define ID_SPEEDSLIDER 10044
#define ID_SPEED 10045
#define ID_LOOP 10046
#define ID_CONTINUOUS 10047
#define ID_SMOOTH 10048
#define ID_POS_ONLY 10049
#define ID_RECORD_LINEAR 10050
#define ID_RECORD_INTERVAL 10051
#define ID_RECORD_SPACING 10052
wxSizer *LocationDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_CAMX 10053
#define ID_CAMY 10054
#define ID_CAMZ 10055
#define ID_FOV_TEXT 10056
#define ID_FOV 10057
#define ID_FOVSLIDER 10058
#define ID_NEAR 10059
#define ID_NEARSLIDER 10060
#define ID_FAR 10061
#define ID_FARSLIDER 10062
#define ID_SPEED_UNITS 10063
#define ID_ACCEL 10064
#define ID_LOD_VEG 10065
#define ID_SLIDER_VEG 10066
#define ID_LOD_STRUCT 10067
#define ID_SLIDER_STRUCT 10068
#define ID_LOD_ROAD 10069
#define ID_SLIDER_ROAD 10070
wxSizer *CameraDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

wxSizer *TextDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_STRUCTTYPE 10071
wxSizer *UtilDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_USE_GRID 10072
#define ID_FILENAME 10073
#define ID_LODMETHOD 10074
#define ID_TRICOUNT 10075
#define ID_PIXELERROR 10076
#define ID_TRISTRIPS 10077
#define ID_USE_TIN 10078
#define ID_FILENAME_TIN 10079
#define ID_VERTEXAG 10080
wxSizer *TParams1Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_NONE 10081
#define ID_SINGLE 10082
#define ID_TFILESINGLE 10083
#define ID_DERIVED 10084
#define ID_CHOICE_COLORS 10085
#define ID_EDIT_COLORS 10086
#define ID_TILED 10087
#define ID_TILESIZE 10088
#define ID_TFILEBASE 10089
#define ID_JPEG 10090
#define ID_TFILENAME 10091
#define ID_MIPMAP 10092
#define ID_16BIT 10093
#define ID_PRELIGHT 10094
#define ID_LIGHT_FACTOR 10095
#define ID_CAST_SHADOWS 10096
#define ID_DETAILTEXTURE 10097
#define ID_DT_NAME 10098
#define ID_DT_SCALE 10099
#define ID_DT_DISTANCE 10100
wxSizer *TParams2Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_PLANTS 10101
#define ID_TREEFILE 10102
#define ID_VEGDISTANCE 10103
#define ID_ROADS 10104
#define ID_ROADFILE 10105
#define ID_ROADHEIGHT 10106
#define ID_ROADDISTANCE 10107
#define ID_TEXROADS 10108
#define ID_ROADCULTURE 10109
#define ID_HIGHWAYS 10110
#define ID_PAVED 10111
#define ID_DIRT 10112
#define ID_CONTENT_FILE 10113
#define ID_STRUCTFILES 10114
#define ID_STRUCT_DISTANCE 10115
#define ID_CHECK_STRUCTURE_SHADOWS 10116
#define ID_CHOICE_SHADOW_REZ 10117
#define ID_DARKNESS 10118
#define ID_VEHICLES 10119
wxSizer *TParams3Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_NAV_STYLE 10120
#define ID_MINHEIGHT 10121
#define ID_NAVSPEED 10122
#define ID_LOCFILE 10123
#define ID_INIT_LOCATION 10124
#define ID_HITHER 10125
#define ID_ANIM_PATHS 10126
#define ID_TEXT_INIT_TIME 10127
#define ID_SET_INIT_TIME 10128
#define ID_TIMEMOVES 10129
#define ID_TIMESPEED 10130
wxSizer *TParams4Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_TREECTRL 10131
#define ID_ADD_PATH 10132
#define ID_ADD_TERRAIN 10133
#define ID_COPY 10134
#define ID_DELETE 10135
#define ID_EDIT_PARAMS 10136
wxSizer *TerrManFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_OCEANPLANE 10137
#define ID_OCEANPLANEOFFSET 10138
#define ID_DEPRESSOCEAN 10139
#define ID_DEPRESSOCEANOFFSET 10140
#define ID_SKY 10141
#define ID_SKYTEXTURE 10142
#define ID_HORIZON 10143
#define ID_FOG 10144
#define ID_FOG_DISTANCE 10145
#define ID_COLOR3 10146
#define ID_BGCOLOR 10147
wxSizer *TParams5Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_RAWFILES 10148
#define ID_STYLE 10149
#define ID_OVERLAY_FILE 10150
#define ID_OVERLAY_DOTDOTDOT 10151
#define ID_OVERLAY_X 10152
#define ID_OVERLAY_Y 10153
wxSizer *TParams6Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

extern wxSizer *g_pLayerSizer1;
extern wxSizer *g_pLayerSizer2;
#define ID_LAYER_ACTIVE 10154
#define ID_LAYER_VISIBLE 10155
#define ID_SHADOW_VISIBLE  10156
#define ID_LAYER_ZOOM_TO 10157
#define ID_LAYER_SAVE 10158
#define ID_LAYER_CREATE 10159
#define ID_LAYER_REMOVE 10160
#define ID_SHOW_ALL 10161
#define ID_LAYER_TREE 10162
wxSizer *LayerDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SPIN_YEAR 10163
#define ID_SPIN_MONTH 10164
#define ID_SPIN_DAY 10165
#define ID_SPIN_HOUR 10166
#define ID_SPIN_MINUTE 10167
#define ID_SPIN_SECOND 10168
#define ID_TEXT_SPEED 10169
#define ID_SLIDER_SPEED 10170
wxSizer *TimeDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_FULLSCREEN 10171
#define ID_TEXTURE_COMPRESSION 10172
#define ID_DISABLE_MIPMAPS 10173
#define ID_SELECTION_CUTOFF 10174
#define ID_SELECTION_RADIUS 10175
#define ID_PLANTSIZE 10176
#define ID_ONLY_AVAILABLE_SPECIES 10177
#define ID_CHOICE_CONTENT 10178
wxSizer *OptionsDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_FEATURE_TYPE 10179
#define ID_GEOMETRY 10180
#define ID_GEOM_COLOR 10181
#define ID_GEOM_HEIGHT 10182
#define ID_TESSELLATE 10183
#define ID_TEXT_LABELS 10184
#define ID_LABEL_COLOR 10185
#define ID_TEXT_FIELD 10186
#define ID_COLOR_FIELD 10187
#define ID_LABEL_HEIGHT 10188
#define ID_LABEL_SIZE 10189
wxSizer *StyleDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

// Declare menubar functions

// Declare toolbar functions

// Declare bitmap functions

wxBitmap MyBitmapsFunc( size_t index );

#endif

// End of generated file
