//------------------------------------------------------------------------------
// Header generated by wxDesigner from file: vtui.wdr
// Do not modify this file, all changes will be lost!
//------------------------------------------------------------------------------

#ifndef __WDR_vtui_H__
#define __WDR_vtui_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "vtui_wdr.h"
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

const int ID_TEXT = 10000;
const int ID_LEVEL = 10001;
const int ID_LEVEL_COPY = 10002;
const int ID_LEVEL_DEL = 10003;
const int ID_LEVEL_UP = 10004;
const int ID_LEVEL_DOWN = 10005;
const int ID_LINE1 = 10006;
const int ID_EDITHEIGHTS = 10007;
const int ID_SET_ROOF_TYPE = 10008;
const int ID_STORIES = 10009;
const int ID_STORY_HEIGHT = 10010;
const int ID_MATERIAL1 = 10011;
const int ID_SET_MATERIAL = 10012;
const int ID_COLOR1 = 10013;
const int ID_SET_COLOR = 10014;
const int ID_EDGE_SLOPES = 10015;
const int ID_SET_EDGE_SLOPES = 10016;
const int ID_EDGES = 10017;
wxSizer *BuildingDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

extern wxSizer *g_pTopGroup;
const int ID_EDGE = 10018;
extern wxSizer *g_pEdgeGroup;
const int ID_MATERIAL2 = 10019;
const int ID_FACADE = 10020;
const int ID_COLOR2 = 10021;
const int ID_EDGE_SLOPE = 10022;
const int ID_FEATURES = 10023;
const int ID_FEAT_CLEAR = 10024;
const int ID_FEAT_WALL = 10025;
const int ID_FEAT_WINDOW = 10026;
const int ID_FEAT_DOOR = 10027;
wxSizer *BuildingEdgesDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_LINEAR_STRUCTURE_STYLE = 10028;
const int ID_POST_TYPE = 10029;
const int ID_POST_SPACING_EDIT = 10030;
const int ID_POST_SPACING_SLIDER = 10031;
const int ID_POST_HEIGHT_EDIT = 10032;
const int ID_POST_HEIGHT_SLIDER = 10033;
const int ID_POST_SIZE_EDIT = 10034;
const int ID_POST_SIZE_SLIDER = 10035;
const int ID_CHOICE_EXTENSION = 10036;
const int ID_CONN_TYPE = 10037;
const int ID_CONN_MATERIAL = 10038;
const int ID_CONN_TOP_EDIT = 10039;
const int ID_CONN_TOP_SLIDER = 10040;
const int ID_CONN_BOTTOM_EDIT = 10041;
const int ID_CONN_BOTTOM_SLIDER = 10042;
const int ID_CONN_WIDTH_EDIT = 10043;
const int ID_CONN_WIDTH_SLIDER = 10044;
const int ID_SLOPE = 10045;
const int ID_SLOPE_SLIDER = 10046;
const int ID_CHOICE_PROFILE = 10047;
const int ID_PROFILE_EDIT = 10048;
const int ID_CONSTANT_TOP = 10049;
wxSizer *LinearStructDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_RADIO_CONTENT = 10050;
const int ID_CHOICE_FILE = 10051;
const int ID_CHOICE_TYPE = 10052;
const int ID_CHOICE_ITEM = 10053;
const int ID_RADIO_MODEL = 10054;
const int ID_MODEL_FILE = 10055;
const int ID_BROWSE_MODEL_FILE = 10056;
const int ID_LOCATION = 10057;
wxSizer *InstanceDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_RADIO_LINE = 10058;
const int ID_RADIO_PATH = 10059;
const int ID_DIST_TOOL_CLEAR = 10060;
const int ID_MAP_OFFSET = 10061;
const int ID_UNITS1 = 10062;
const int ID_MAP_DIST = 10063;
const int ID_UNITS2 = 10064;
const int ID_GEOD_DIST = 10065;
const int ID_UNITS3 = 10066;
const int ID_GROUND_DIST = 10067;
const int ID_UNITS4 = 10068;
const int ID_VERTICAL = 10069;
const int ID_UNITS5 = 10070;
wxSizer *DistanceDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_CMAP_FILE = 10071;
const int ID_RELATIVE = 10072;
const int ID_BLEND = 10073;
const int ID_COLORLIST = 10074;
const int ID_CHANGE_COLOR = 10075;
const int ID_DELETE_ELEVATION = 10076;
const int ID_HEIGHT_TO_ADD = 10077;
const int ID_ADD = 10078;
const int ID_SAVE_CMAP = 10079;
const int ID_SAVE_AS_CMAP = 10080;
const int ID_LOAD_CMAP = 10081;
wxSizer *ColorMapDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_LINE_OF_SIGHT = 10082;
const int ID_VISIBILITY = 10083;
const int ID_FRESNEL = 10084;
const int ID_USE_EFFECTIVE = 10085;
const int ID_SHOW_CULTURE = 10086;
const int ID_LINE2 = 10087;
const int ID_HEIGHT1 = 10088;
const int ID_HEIGHT2 = 10089;
const int ID_RF = 10090;
const int ID_CURVATURE = 10091;
const int ID_STATUS_TEXT = 10092;
const int ID_EXPORT_DXF = 10093;
const int ID_EXPORT_TRACE = 10094;
wxSizer *ProfileDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_PROJ = 10095;
const int ID_SHOW_ALL_DATUMS = 10096;
const int ID_DATUM = 10097;
const int ID_ELLIPSOID = 10098;
const int ID_HORUNITS = 10099;
const int ID_ZONE = 10100;
const int ID_PROJPARAM = 10101;
const int ID_STATEPLANE = 10102;
const int ID_SET_EPSG = 10103;
const int ID_PROJ_LOAD = 10104;
const int ID_PROJ_SAVE = 10105;
wxSizer *ProjectionDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_STATEPLANES = 10106;
const int ID_NAD27 = 10107;
const int ID_NAD83 = 10108;
const int ID_RADIO_METERS = 10109;
const int ID_RADIO_FEET = 10110;
const int ID_RADIO_FEET_US = 10111;
wxSizer *StatePlaneDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_EDIT_PANEL = 10112;
const int ID_ADD_POINT = 10113;
const int ID_MOVE_POINT = 10114;
const int ID_REMOVE_POINT = 10115;
const int ID_SAVE_PROF = 10116;
const int ID_SAVE_AS_PROF = 10117;
const int ID_LOAD_PROF = 10118;
wxSizer *ProfileEditDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_TAGLIST = 10119;
wxSizer *TagDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_TEXT_SHOW = 10120;
const int ID_CHOICE_SHOW = 10121;
const int ID_TEXT_VERTICAL = 10122;
const int ID_CHOICE_VERTICAL = 10123;
const int ID_DEL_HIGH = 10124;
const int ID_LIST = 10125;
wxSizer *FeatureTableDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_TEXTCTRL = 10126;
const int ID_SLIDER_RATIO = 10127;
const int ID_TEXT_X = 10128;
const int ID_TEXT_Y = 10129;
wxSizer *SizeDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

// Declare menubar functions

// Declare toolbar functions

// Declare bitmap functions

wxBitmap vtuiBitmapsFunc( size_t index );

#endif

// End of generated file
