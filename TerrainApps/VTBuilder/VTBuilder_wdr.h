//------------------------------------------------------------------------------
// Header generated by wxDesigner from file: VTBuilder.wdr
// Do not modify this file, all changes will be lost!
//------------------------------------------------------------------------------

#ifndef __WDR_VTBuilder_H__
#define __WDR_VTBuilder_H__

#ifdef __GNUG__
    #pragma interface "VTBuilder_wdr.cpp"
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
#define ID_EXTENT_N 10001
#define ID_EXTENT_W 10002
#define ID_EXTENT_E 10003
#define ID_EXTENT_S 10004
#define ID_EXTENT_ALL 10005
wxSizer *ExtentDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_MAP_OFFSET 10006
#define ID_UNITS1 10007
#define ID_MAP_DIST 10008
#define ID_UNITS2 10009
#define ID_GEOD_DIST 10010
#define ID_UNITS3 10011
wxSizer *DistanceDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_BYTES 10012
#define ID_WIDTH 10013
#define ID_HEIGHT 10014
#define ID_LITTLE_ENDIAN 10015
#define ID_UTM 10016
#define ID_BIG_ENDIAN 10017
#define ID_FLOATING 10018
#define ID_VUNITS 10019
#define ID_SPACING 10020
wxSizer *RawDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_PROPS 10021
#define ID_LEFT 10022
#define ID_TOP 10023
#define ID_RIGHT 10024
#define ID_BOTTOM 10025
wxSizer *LayerPropDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_FIELD 10026
#define ID_DENSITY 10027
#define ID_BIOTYPE1 10028
#define ID_BIOTYPE2 10029
wxSizer *ImportVegFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define IDC_SAMPLING 10030
#define IDC_SCARCITY 10031
wxSizer *DistribVegFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_TOWER_XY 10032
#define ID_TOWER_TYPE 10033
#define ID_TOWER_MATERIAL 10034
#define ID_TOWER_ELEV 10035
wxSizer *TowerDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_PROJ 10036
#define ID_DATUM 10037
#define ID_HORUNITS 10038
#define ID_ZONE 10039
#define ID_PROJPARAM 10040
#define ID_STATEPLANE 10041
#define ID_LINE 10042
wxSizer *ProjectionDialog2Func( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_STATEPLANES 10043
#define ID_NAD27 10044
#define ID_NAD83 10045
wxSizer *StatePlaneDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_NUMLANES 10046
#define ID_HWYNAME 10047
#define ID_SIDEWALK 10048
#define ID_PARKING 10049
#define ID_MARGIN 10050
#define ID_SURFTYPE 10051
wxSizer *RoadPropDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_INTTYPE 10052
#define ID_ROADNUM 10053
#define ID_BEHAVIOR 10054
wxSizer *NodePropDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_SPACINGX 10055
#define ID_SPACINGY 10056
#define ID_SIZEX 10057
#define ID_SIZEY 10058
#define ID_CONSTRAIN 10059
#define ID_SMALLER 10060
#define ID_BIGGER 10061
#define ID_FLOATS 10062
#define ID_SHORTS 10063
#define ID_AREAX 10064
#define ID_AREAY 10065
#define ID_ESTX 10066
#define ID_ESTY 10067
wxSizer *ResampleDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_CONDITION 10068
#define ID_COMBO_VALUE 10069
wxSizer *SelectDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_TEXT_SHOW 10070
#define ID_CHOICE_SHOW 10071
#define ID_TEXT_VERTICAL 10072
#define ID_CHOICE_VERTICAL 10073
#define ID_DEL_HIGH 10074
#define ID_LIST 10075
wxSizer *FeatInfoDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_TOOLBAR 10076
#define ID_MINUTES 10077
#define ID_ELEVUNIT 10078
#define ID_PATHNAMES 10079
wxSizer *OptionsDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_USE_SPECIES 10080
#define ID_SPECIES_CHOICE 10081
#define ID_SPECIES_USE_FIELD 10082
#define ID_SPECIES_FIELD 10083
#define ID_SPECIES_ID 10084
#define ID_SPECIES_NAME 10085
#define ID_COMMON_NAME 10086
#define ID_BIOTYPE_INT 10087
#define ID_BIOTYPE_STRING 10088
#define ID_HEIGHT_RANDOM 10089
#define ID_HEIGHT_USE_FIELD 10090
#define ID_HEIGHT_FIELD 10091
wxSizer *VegFieldsDialogFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

#define ID_TYPE_CENTER 10092
#define ID_TYPE_FOOTPRINT 10093
#define ID_FLIP 10094
#define ID_TYPE_LINEAR 10095
#define ID_TYPE_INSTANCE 10096
#define ID_CHOICE_FILE_FIELD 10097
#define ID_INSIDE_AREA 10098
wxSizer *ImportStructFunc( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

// Declare menubar functions

// Declare toolbar functions

// Declare bitmap functions

wxBitmap MyBitmapsFunc( size_t index );

#endif

// End of generated file
