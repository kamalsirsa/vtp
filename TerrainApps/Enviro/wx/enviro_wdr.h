//------------------------------------------------------------------------------
// Header generated by wxDesigner from file: enviro.wdr
// Do not modify this file, all changes will be lost!
//------------------------------------------------------------------------------

#ifndef __WDR_enviro_H__
#define __WDR_enviro_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "enviro_wdr.h"
#endif

// Include wxWidgets' headers

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
#include <wx/tglbtn.h>

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
#define ID_LOG 10014
wxSizer *SceneGraphFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_NOTEBOOK 10015
wxSizer *TParamsFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SPECIES 10016
#define ID_COMMON_NAMES 10017
#define ID_LANGUAGE 10018
#define ID_PLANT_HEIGHT_EDIT 10019
#define ID_HEIGHT_SLIDER 10020
#define ID_PLANT_INDIVIDUAL 10021
#define ID_PLANT_LINEAR 10022
#define ID_PLANT_CONTINUOUS 10023
#define ID_PLANT_VARIANCE_EDIT 10024
#define ID_PLANT_VARIANCE_SLIDER 10025
#define ID_PLANT_SPACING_EDIT 10026
wxSizer *PlantDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_LOCLIST 10027
#define ID_SAVE 10028
#define ID_LOAD 10029
#define ID_RECALL 10030
#define ID_STORE 10031
#define ID_STOREAS 10032
#define ID_REMOVE 10033
#define ID_LINE 10034
#define ID_ANIMTREE 10035
#define ID_NEW_ANIM 10036
#define ID_SAVE_ANIM 10037
#define ID_LOAD_ANIM 10038
#define ID_RESET 10039
#define ID_STOP 10040
#define ID_RECORD1 10041
#define ID_PLAY 10042
#define ID_ANIM_POS 10043
#define ID_ACTIVE 10044
#define ID_SPEEDSLIDER 10045
#define ID_SPEED 10046
#define ID_LOOP 10047
#define ID_CONTINUOUS 10048
#define ID_SMOOTH 10049
#define ID_POS_ONLY 10050
#define ID_PLAY_TO_DISK 10051
#define ID_RECORD_LINEAR 10052
#define ID_RECORD_INTERVAL 10053
#define ID_RECORD_SPACING 10054
wxSizer *LocationDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

extern wxSizer *CamSizerTop;
#define ID_CAMX 10055
#define ID_CAMY 10056
#define ID_CAMZ 10057
#define ID_FOV_TEXT 10058
#define ID_FOV 10059
#define ID_FOVSLIDER 10060
extern wxSizer *CamSizerNear;
#define ID_NEAR 10061
#define ID_NEARSLIDER 10062
extern wxSizer *CamSizerFar;
#define ID_FAR 10063
#define ID_FARSLIDER 10064
#define ID_EYE_SEP 10065
#define ID_EYE_SEPSLIDER 10066
#define ID_SPEED_UNITS 10067
#define ID_ACCEL 10068
extern wxSizer *CamSizerLOD;
#define ID_LOD_VEG 10069
#define ID_SLIDER_VEG 10070
#define ID_LOD_STRUCT 10071
#define ID_SLIDER_STRUCT 10072
#define ID_LOD_ROAD 10073
#define ID_SLIDER_ROAD 10074
wxSizer *CameraDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

wxSizer *TextDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_STRUCTTYPE 10075
wxSizer *UtilDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_USE_GRID 10076
#define ID_FILENAME 10077
#define ID_LODMETHOD 10078
#define ID_TRI_COUNT 10079
#define ID_TRISTRIPS 10080
#define ID_USE_TIN 10081
#define ID_FILENAME_TIN 10082
#define ID_USE_TILESET 10083
#define ID_FILENAME_TILES 10084
#define ID_VTX_COUNT 10085
#define ID_TILE_CACHE_SIZE 10086
#define ID_TILE_THREADING 10087
#define ID_VERTEXAG 10088
wxSizer *TParams1Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_NONE 10089
#define ID_SINGLE 10090
#define ID_TFILE_SINGLE 10091
#define ID_DERIVED 10092
#define ID_CHOICE_COLORS 10093
#define ID_EDIT_COLORS 10094
#define ID_TILED_4BY4 10095
#define ID_CHOICE_TILESIZE 10096
#define ID_TFILE_BASE 10097
#define ID_TFILENAME 10098
#define ID_TILESET 10099
#define ID_TFILE_TILESET 10100
#define ID_TEXTURE_GRADUAL 10101
#define ID_MIPMAP 10102
#define ID_16BIT 10103
#define ID_PRELIGHT 10104
#define ID_LIGHT_FACTOR 10105
#define ID_CAST_SHADOWS 10106
#define ID_RETAIN 10107
#define ID_DETAILTEXTURE 10108
#define ID_DT_NAME 10109
#define ID_DT_SCALE 10110
#define ID_DT_DISTANCE 10111
wxSizer *TParams2Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_PLANTS 10112
#define ID_TREEFILE 10113
#define ID_VEGDISTANCE 10114
#define ID_ROADS 10115
#define ID_ROADFILE 10116
#define ID_ROADHEIGHT 10117
#define ID_ROADDISTANCE 10118
#define ID_TEXROADS 10119
#define ID_ROADCULTURE 10120
#define ID_HIGHWAYS 10121
#define ID_PAVED 10122
#define ID_DIRT 10123
#define ID_CONTENT_FILE 10124
#define ID_STRUCTFILES 10125
#define ID_STRUCT_DISTANCE 10126
#define ID_CHECK_STRUCTURE_SHADOWS 10127
#define ID_CHOICE_SHADOW_REZ 10128
#define ID_DARKNESS 10129
#define ID_CHECK_STRUCTURE_PAGING 10130
#define ID_PAGING_MAX_STRUCTURES 10131
#define ID_PAGE_OUT_DISTANCE 10132
wxSizer *TParams3Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_NAV_STYLE 10133
#define ID_MINHEIGHT 10134
#define ID_NAVSPEED 10135
#define ID_ALLOW_ROLL 10136
#define ID_LOCFILE 10137
#define ID_INIT_LOCATION 10138
#define ID_HITHER 10139
#define ID_ANIM_PATHS 10140
wxSizer *TParams4Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_TREECTRL 10141
#define ID_ADD_PATH 10142
#define ID_ADD_TERRAIN 10143
#define ID_COPY 10144
#define ID_DELETE 10145
#define ID_EDIT_PARAMS 10146
wxSizer *TerrManFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_OCEANPLANE 10147
#define ID_OCEANPLANEOFFSET 10148
#define ID_DEPRESSOCEAN 10149
#define ID_DEPRESSOCEANOFFSET 10150
#define ID_SKY 10151
#define ID_SKYTEXTURE 10152
#define ID_HORIZON 10153
#define ID_FOG 10154
#define ID_FOG_DISTANCE 10155
#define ID_COLOR3 10156
#define ID_BGCOLOR 10157
#define ID_TEXT_INIT_TIME 10158
#define ID_SET_INIT_TIME 10159
#define ID_TIMEMOVES 10160
#define ID_TIMESPEED 10161
wxSizer *TParams5Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_RAWFILES 10162
#define ID_STYLE 10163
#define ID_IMAGEFILES 10164
wxSizer *TParams6Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SPIN_YEAR 10165
#define ID_SPIN_MONTH 10166
#define ID_SPIN_DAY 10167
#define ID_SPIN_HOUR 10168
#define ID_SPIN_MINUTE 10169
#define ID_SPIN_SECOND 10170
#define ID_TEXT_SPEED 10171
#define ID_SLIDER_SPEED 10172
wxSizer *TimeDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_FULLSCREEN 10173
#define ID_STEREO 10174
#define ID_STEREO1 10175
#define ID_STEREO2 10176
#define ID_WINX 10177
#define ID_WINY 10178
#define ID_WIN_XSIZE 10179
#define ID_WIN_YSIZE 10180
#define ID_SIZE_INSIDE 10181
#define ID_TEXTURE_COMPRESSION 10182
#define ID_DISABLE_MIPMAPS 10183
#define ID_DIRECT_PICKING 10184
#define ID_SELECTION_CUTOFF 10185
#define ID_SELECTION_RADIUS 10186
#define ID_PLANTSIZE 10187
#define ID_ONLY_AVAILABLE_SPECIES 10188
#define ID_CHOICE_CONTENT 10189
#define ID_TERRAIN_PROGRESS 10190
#define ID_FLY_IN 10191
wxSizer *OptionsDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_FEATURE_TYPE 10192
wxSizer *StyleDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SCENARIO_LIST 10193
#define ID_NEW_SCENARIO 10194
#define ID_DELETE_SCENARIO 10195
#define ID_EDIT_SCENARIO 10196
#define ID_MOVEUP_SCENARIO 10197
#define ID_MOVEDOWN_SCENARIO 10198
#define ID_CHOICE_SCENARIO 10199
wxSizer *ScenariosPaneFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SCENARIO_VISIBLE_LAYERS 10200
#define ID_SCENARIO_ADD_VISIBLE_LAYER 10201
#define ID_SCENARIO_REMOVE_VISIBLE_LAYER 10202
#define ID_SCENARIO_AVAILABLE_LAYERS 10203
wxSizer *ScenarioVisibleLayersPane( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SCENARIO_NAME 10204
wxSizer *ScenarioParamsDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

extern wxSizer *ScenarioSizerTop;
extern wxSizer *ScenarioSizerMid;
extern wxSizer *ScenarioSizerSide;
#define ID_SCENARIO_PREVIOUS 10205
#define ID_SCENARIO_NEXT 10206
wxSizer *ScenarioSelectDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_PANEL1 10207
#define ID_TARGET 10208
#define ID_CURRENT 10209
#define ID_SLIDER_PRANGE 10210
#define ID_TEXT_PRANGE 10211
#define ID_PANEL2 10212
#define ID_STATICBITMAP 10213
#define ID_TILE_STATUS 10214
wxSizer *LODDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

wxSizer *TextureDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

wxSizer *LayerAnimDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_CHOICE_VEHICLES 10215
#define ID_SET_VEHICLE_COLOR 10216
wxSizer *VehicleDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_ENABLE_OBJECT_GEOM 10217
#define ID_RADIO1 10218
#define ID_OBJECT_GEOM_COLOR 10219
#define ID_RADIO_USE_OBJECT_COLOR_FIELD 10220
#define ID_OBJECT_COLOR_FIELD 10221
#define ID_OBJECT_GEOM_HEIGHT 10222
#define ID_OBJECT_GEOM_SIZE 10223
wxSizer *Style1Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_ENABLE_LINE_GEOM 10224
#define ID_RADIO2 10225
#define ID_LINE_GEOM_COLOR 10226
#define ID_RADIO_USE_LINE_COLOR_FIELD 10227
#define ID_LINE_COLOR_FIELD 10228
#define ID_LINE_GEOM_HEIGHT 10229
#define ID_LINE_WIDTH 10230
#define ID_TESSELLATE 10231
wxSizer *Style2Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_ENABLE_TEXT_LABELS 10232
#define ID_RADIO3 10233
#define ID_TEXT_COLOR 10234
#define ID_RADIO_USE_TEXT_COLOR_FIELD 10235
#define ID_TEXT_COLOR_FIELD 10236
#define ID_TEXT_FIELD 10237
#define ID_LABEL_HEIGHT 10238
#define ID_LABEL_SIZE 10239
#define ID_FONT 10240
wxSizer *Style3Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_ENABLE_TEXTURE_OVERLAY 10241
#define ID_TEXTURE_MODE 10242
wxSizer *Style4Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_TEXT_PAGEOUT 10243
#define ID_SLIDER_PAGEOUT 10244
#define ID_COUNT_CURRENT 10245
#define ID_COUNT_MAXIMUM 10246
#define ID_PANEL3 10247
wxSizer *StructLODDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

wxSizer *PagingDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_OVERLAY_FILE 10248
#define ID_OVERLAY_DOTDOTDOT 10249
#define ID_OVERLAY_X 10250
#define ID_OVERLAY_Y 10251
#define ID_CHECK_OVERVIEW 10252
#define ID_CHECK_COMPASS 10253
wxSizer *TParams7Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SLIDER_FOG_DISTANCE 10254
#define ID_TEXT_WIND_DIRECTION 10255
#define ID_SLIDER_WIND_DIRECTION 10256
#define ID_TEXT_WIND_SPEED 10257
#define ID_SLIDER_WIND_SPEED 10258
wxSizer *EphemDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

// Declare menubar functions

// Declare toolbar functions

#define ID_LAYER_CREATE 10259
#define ID_LAYER_LOAD 10260
#define ID_LAYER_SAVE 10261
#define ID_LAYER_SAVE_AS 10262
#define ID_LAYER_DELETE 10263
#define ID_LAYER_ZOOM_TO 10264
#define ID_LAYER_VISIBLE 10265
#define ID_LAYER_TABLE 10266
#define ID_LAYER_SHADOW 10267
#define ID_SHOW_ALL 10268
void LayerToolBarFunc( wxToolBar *parent );

// Declare bitmap functions

wxBitmap MyBitmapsFunc( size_t index );

wxBitmap LayerToolBitmapsFunc( size_t index );

wxBitmap ColorBitmapsFunc( size_t index );

wxBitmap ToolsFunc( size_t index );

wxBitmap IconsFunc( size_t index );

#endif

// End of generated file
