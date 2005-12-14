//
// Name: TParamsDlg.cpp
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "TParamsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/colordlg.h>
#include "vtlib/vtlib.h"
#include "vtlib/core/Location.h"
#include "vtdata/FilePath.h"		// for FindFileOnPaths()
#include "vtdata/vtLog.h"
#include "vtui/ColorMapDlg.h"
#include "vtui/Helper.h"

#include "TParamsDlg.h"
#include "TimeDlg.h"
#include "StyleDlg.h"

#include "ScenarioParamsDialog.h"
#define NTILES 4

//---------------------------------------------------------------------------

/**
 * wxListBoxEventHandler is a roudabout way of catching events on our
 * listboxes, to implement the "Delete" key operation on them.
 */
class wxListBoxEventHandler: public wxEvtHandler
{
public:
	wxListBoxEventHandler(TParamsDlg *dlg, wxListBox *pBox)
	{
		m_pDlg = dlg;
		m_pBox = pBox;
	}
	void OnChar(wxKeyEvent& event)
	{
		if (event.GetKeyCode() == WXK_DELETE)
		{
			int sel = m_pBox->GetSelection();
			if (sel != -1 && sel < m_pBox->GetCount()-1)
			{
				m_pDlg->DeleteItem(m_pBox);
				m_pDlg->TransferDataToWindow();
			}
		}
		event.Skip();
	}

private:
	TParamsDlg *m_pDlg;
	wxListBox *m_pBox;
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxListBoxEventHandler, wxEvtHandler)
	EVT_CHAR(wxListBoxEventHandler::OnChar)
END_EVENT_TABLE()


// WDR: class implementations

//----------------------------------------------------------------------------
// TParamsDlg
//----------------------------------------------------------------------------

// WDR: event table for TParamsDlg

BEGIN_EVENT_TABLE(TParamsDlg,AutoDialog)
	EVT_INIT_DIALOG (TParamsDlg::OnInitDialog)

	EVT_RADIOBUTTON( ID_USE_GRID, TParamsDlg::OnCheckBox )
	EVT_RADIOBUTTON( ID_USE_TIN, TParamsDlg::OnCheckBox )
	EVT_RADIOBUTTON( ID_USE_TILESET, TParamsDlg::OnCheckBox )
	EVT_CHOICE( ID_LODMETHOD, TParamsDlg::OnCheckBox )
		
	EVT_TEXT( ID_TILESIZE, TParamsDlg::OnTileSize )
	EVT_TEXT( ID_TFILE_BASE, TParamsDlg::OnTextureFileBase )

	EVT_RADIOBUTTON( ID_NONE, TParamsDlg::OnTextureNone )
	EVT_RADIOBUTTON( ID_SINGLE, TParamsDlg::OnTextureSingle )
	EVT_RADIOBUTTON( ID_DERIVED, TParamsDlg::OnTextureDerived )
	EVT_RADIOBUTTON( ID_TILED_4BY4, TParamsDlg::OnTextureTiled )
	EVT_RADIOBUTTON( ID_TILESET, TParamsDlg::OnTextureTileset )

	EVT_BUTTON( ID_EDIT_COLORS, TParamsDlg::OnEditColors )
	EVT_CHECKBOX( ID_JPEG, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_DETAILTEXTURE, TParamsDlg::OnCheckBox )

	EVT_CHECKBOX( ID_PLANTS, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_ROADS, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_CHECK_STRUCTURE_SHADOWS, TParamsDlg::OnCheckBox )

	EVT_LISTBOX_DCLICK( ID_STRUCTFILES, TParamsDlg::OnListDblClickStructure )
	EVT_LISTBOX_DCLICK( ID_RAWFILES, TParamsDlg::OnListDblClickRaw )
	EVT_LISTBOX_DCLICK( ID_ANIM_PATHS, TParamsDlg::OnListDblClickAnimPaths )

	EVT_CHECKBOX( ID_OCEANPLANE, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_DEPRESSOCEAN, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_SKY, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_FOG, TParamsDlg::OnCheckBox )
	EVT_BUTTON( ID_BGCOLOR, TParamsDlg::OnBgColor )

	EVT_TEXT( ID_LOCFILE, TParamsDlg::OnChoiceLocFile )
	EVT_CHOICE( ID_INIT_LOCATION, TParamsDlg::OnChoiceInitLocation )

	EVT_BUTTON( ID_SET_INIT_TIME, TParamsDlg::OnSetInitTime )
	EVT_BUTTON( ID_STYLE, TParamsDlg::OnStyle )
	EVT_BUTTON( ID_OVERLAY_DOTDOTDOT, TParamsDlg::OnOverlay )
	EVT_BUTTON( ID_NEW_SCENARIO, TParamsDlg::OnNewScenario )
	EVT_BUTTON( ID_DELETE_SCENARIO, TParamsDlg::OnDeleteScenario )
	EVT_BUTTON( ID_EDIT_SCENARIO, TParamsDlg::OnEditScenario )
	EVT_BUTTON( ID_MOVEUP_SCENARIO, TParamsDlg::OnMoveUpScenario )
	EVT_BUTTON( ID_MOVEDOWN_SCENARIO, TParamsDlg::OnMoveDownSceanario )
	EVT_LISTBOX( ID_SCENARIO_LIST, TParamsDlg::OnScenarioListEvent )
END_EVENT_TABLE()

TParamsDlg::TParamsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style | wxRESIZE_BORDER )
{
	VTLOG("TParamsDlg: Constructing.\n");

	TParamsFunc( this, TRUE );

	m_bSetting = false;

	// make sure that validation gets down to the child windows
	SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

	// including the children of the notebook
	wxNotebook *notebook = (wxNotebook*) FindWindow( ID_NOTEBOOK );
	notebook->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

	m_pPreLightFactor = GetLightFactor();
	m_pStructFiles = GetStructFiles();
	m_pRawFiles = GetRawFiles();
	m_pAnimFiles = GetAnimPaths();
	m_pRoadFile = GetRoadfile();
	m_pTreeFile = GetTreefile();
	m_pTextureFileSingle = GetTfileSingle();
	m_pTextureFileTileset = GetTfileTileset();
	m_pDTName = GetDTName();
	m_pLodMethod = GetLodmethod();
	m_pFilename = GetFilename();
	m_pFilenameTin = GetFilenameTin();
	m_pFilenameTileset = GetFilenameTileset();
	m_pLocFile = GetLocfile();
	m_pShadowRez = GetChoiceShadowRez();
	m_pSkyTexture = GetSkytexture();
	m_pLocField = GetLocField();
	m_pNavStyle = GetNavStyle();

	m_pNone = GetNone();
	m_pSingle = GetSingle();
	m_pDerived = GetDerived();
	m_pTiled = GetTiled();
	m_pTileset = GetTileset();
	m_pColorMap = GetColorMap();

	m_pScenarioList = GetScenarioList();
	m_iOverlayX = 0;
	m_iOverlayY = 0;

	// Create Validators To Attach C++ Members To WX Controls

	// overall name
	AddValidator(ID_TNAME, &m_strTerrainName);

	// elevation
	AddValidator(ID_USE_GRID, &m_bGrid);
	AddValidator(ID_USE_TIN, &m_bTin);
	AddValidator(ID_USE_TILESET, &m_bTileset);
	AddValidator(ID_FILENAME, &m_strFilename);
	AddValidator(ID_FILENAME_TIN, &m_strFilenameTin);
	AddValidator(ID_FILENAME_TILES, &m_strFilenameTileset);
	AddNumValidator(ID_VERTEXAG, &m_fVerticalExag, 2);

	// nav
	AddNumValidator(ID_MINHEIGHT, &m_fMinHeight);
	AddValidator(ID_NAV_STYLE, &m_iNavStyle);
	AddNumValidator(ID_NAVSPEED, &m_fNavSpeed, 2);
	AddValidator(ID_LOCFILE, &m_strLocFile);
	AddValidator(ID_INIT_LOCATION, &m_iInitLocation);
	AddNumValidator(ID_HITHER, &m_fHither);
	AddValidator(ID_ACCEL, &m_bAccel);

	// LOD
	AddValidator(ID_LODMETHOD, &m_iLodMethod);
	AddNumValidator(ID_TRI_COUNT, &m_iTriCount);
	AddValidator(ID_TRISTRIPS, &m_bTriStrips);
	AddNumValidator(ID_VTX_COUNT, &m_iVertCount);
	AddNumValidator(ID_TILE_CACHE_SIZE, &m_iTileCacheSize);

	// time
	AddValidator(ID_TIMEMOVES, &m_bTimeOn);
	AddValidator(ID_TEXT_INIT_TIME, &m_strInitTime);
	AddNumValidator(ID_TIMESPEED, &m_fTimeSpeed, 2);

	// texture
	AddValidator(ID_TFILE_SINGLE, &m_strTextureSingle);
	AddNumValidator(ID_TILESIZE, &m_iTilesize);
	AddValidator(ID_TFILE_BASE, &m_strTextureBase);
	AddValidator(ID_JPEG, &m_bJPEG);
	AddValidator(ID_TFILENAME, &m_strTextureFilename);
	AddValidator(ID_TFILE_TILESET, &m_strTextureTileset);
	AddValidator(ID_MIPMAP, &m_bMipmap);
	AddValidator(ID_16BIT, &m_b16bit);
	AddValidator(ID_PRELIGHT, &m_bPreLight);
	AddValidator(ID_CAST_SHADOWS, &m_bCastShadows);
	AddNumValidator(ID_LIGHT_FACTOR, &m_fPreLightFactor, 2);
	AddValidator(ID_CHOICE_COLORS, &m_strColorMap);
	AddValidator(ID_RETAIN, &m_bTextureRetain);

	// detail texture
	AddValidator(ID_DETAILTEXTURE, &m_bDetailTexture);
	AddValidator(ID_DT_NAME, &m_strDetailName);
	AddNumValidator(ID_DT_SCALE, &m_fDetailScale);
	AddNumValidator(ID_DT_DISTANCE, &m_fDetailDistance);

	// culture page
	AddValidator(ID_PLANTS, &m_bPlants);
	AddValidator(ID_TREEFILE, &m_strVegFile);
	AddNumValidator(ID_VEGDISTANCE, &m_iVegDistance);

	AddValidator(ID_ROADS, &m_bRoads);
	AddValidator(ID_ROADFILE, &m_strRoadFile);
	AddValidator(ID_HIGHWAYS, &m_bHwy);
	AddValidator(ID_PAVED, &m_bPaved);
	AddValidator(ID_DIRT, &m_bDirt);
	AddNumValidator(ID_ROADHEIGHT, &m_fRoadHeight);
	AddNumValidator(ID_ROADDISTANCE, &m_fRoadDistance);
	AddValidator(ID_TEXROADS, &m_bTexRoads);
	AddValidator(ID_ROADCULTURE, &m_bRoadCulture);

	AddValidator(ID_CONTENT_FILE, &m_strContent);
	AddNumValidator(ID_STRUCT_DISTANCE, &m_iStructDistance);
	AddValidator(ID_CHECK_STRUCTURE_SHADOWS, &m_bStructureShadows);
	AddValidator(ID_CHOICE_SHADOW_REZ, &m_iStructureRez);
	AddNumValidator(ID_DARKNESS, &m_fDarkness);
	AddValidator(ID_VEHICLES, &m_bVehicles);

	// Atmosphere and water page
	AddValidator(ID_SKY, &m_bSky);
	AddValidator(ID_SKYTEXTURE, &m_strSkyTexture);
	AddValidator(ID_OCEANPLANE, &m_bOceanPlane);
	AddNumValidator(ID_OCEANPLANEOFFSET, &m_fOceanPlaneLevel);
	AddValidator(ID_DEPRESSOCEAN, &m_bDepressOcean);
	AddNumValidator(ID_DEPRESSOCEANOFFSET, &m_fDepressOceanLevel);
	AddValidator(ID_HORIZON, &m_bHorizon);
	AddValidator(ID_FOG, &m_bFog);
	AddNumValidator(ID_FOG_DISTANCE, &m_fFogDistance);

	// Abstract Layers page, overlay stuff
	AddValidator(ID_OVERLAY_FILE, &m_strOverlayFile);
	AddNumValidator(ID_OVERLAY_X, &m_iOverlayX);
	AddNumValidator(ID_OVERLAY_Y, &m_iOverlayY);

	// It's somewhat roundabout, but this lets us capture events on the
	// listbox controls without having to subclass.
	m_pStructFiles->PushEventHandler(new wxListBoxEventHandler(this, m_pStructFiles));
	m_pRawFiles->PushEventHandler(new wxListBoxEventHandler(this, m_pRawFiles));
	m_pAnimFiles->PushEventHandler(new wxListBoxEventHandler(this, m_pAnimFiles));
}

TParamsDlg::~TParamsDlg()
{
	m_pStructFiles->PopEventHandler(true);
	m_pRawFiles->PopEventHandler(true);
	m_pAnimFiles->PopEventHandler(true);
}

//
// Set the values in the dialog from the supplied paramter structure.
// Note that TParams uses UTF8 for all its strings, so we need to use
//  from_utf8() when copying to wxString.
//
void TParamsDlg::SetParams(const TParams &Params)
{
	VTLOG("TParamsDlg::SetParams\n");
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// overall name
	m_strTerrainName.from_utf8(Params.GetValueString(STR_NAME));

	// elevation
	m_bGrid =			Params.GetValueInt(STR_SURFACE_TYPE) == 0;
	m_bTin =			Params.GetValueInt(STR_SURFACE_TYPE) == 1;
	m_bTileset =		Params.GetValueInt(STR_SURFACE_TYPE) == 2;
	if (m_bGrid)
		m_strFilename.from_utf8(Params.GetValueString(STR_ELEVFILE));
	if (m_bTin)
		m_strFilenameTin.from_utf8(Params.GetValueString(STR_ELEVFILE));
	if (m_bTileset)
		m_strFilenameTileset.from_utf8(Params.GetValueString(STR_ELEVFILE));
	m_fVerticalExag =   Params.GetValueFloat(STR_VERTICALEXAG);

	/// navigation
	m_fMinHeight =		Params.GetValueFloat(STR_MINHEIGHT);
	m_iNavStyle =		Params.GetValueInt(STR_NAVSTYLE);
	m_fNavSpeed =		Params.GetValueFloat(STR_NAVSPEED);
	m_strLocFile.from_utf8(Params.GetValueString(STR_LOCFILE));
	m_strInitLocation.from_utf8(Params.GetValueString(STR_INITLOCATION));
	m_fHither =			Params.GetValueFloat(STR_HITHER);
	m_bAccel =			Params.GetValueBool(STR_ACCEL);
	m_AnimPaths =		Params.m_AnimPaths;

	// LOD
	m_iLodMethod =		Params.GetLodMethod();
	m_iTriCount =		Params.GetValueInt(STR_TRICOUNT);
	m_bTriStrips =		Params.GetValueBool(STR_TRISTRIPS);
	m_iVertCount =		Params.GetValueInt(STR_VERTCOUNT);
	m_iTileCacheSize =	Params.GetValueInt(STR_TILE_CACHE_SIZE);

	// time
	m_bTimeOn =		 Params.GetValueBool(STR_TIMEON);
	m_InitTime.SetFromString(Params.GetValueString(STR_INITTIME));
	m_fTimeSpeed =	  Params.GetValueFloat(STR_TIMESPEED);

	// texture
	m_iTexture =		Params.GetTextureEnum();
	if (m_iTexture == 1)	// single
	{
		m_strTextureSingle.from_utf8(Params.GetValueString(STR_TEXTUREFILE));
	}
	else if (m_iTexture == 2)	// tile4x4
	{
		m_strTextureBase.from_utf8(Params.GetValueString(STR_TEXTUREBASE));
		m_iTilesize =		Params.GetValueInt(STR_TILESIZE);
		m_bJPEG =		   (Params.GetValueBool(STR_TEXTUREFORMAT) == 1);
		m_strTextureFilename.from_utf8(Params.CookTextureFilename());
	}
	else if (m_iTexture == 3)	// derived
	{
		m_strColorMap.from_utf8(Params.GetValueString(STR_COLOR_MAP));
	}
	else if (m_iTexture == 4)	// tileset
	{
		m_strTextureTileset.from_utf8(Params.GetValueString(STR_TEXTUREFILE));
	}
	m_bMipmap =			Params.GetValueBool(STR_MIPMAP);
	m_b16bit =			Params.GetValueBool(STR_REQUEST16BIT);
	m_bPreLight =		Params.GetValueBool(STR_PRELIGHT);
	m_fPreLightFactor = Params.GetValueFloat(STR_PRELIGHTFACTOR);
	m_bCastShadows =	Params.GetValueBool(STR_CAST_SHADOWS);
	m_bTextureRetain =	Params.GetValueBool(STR_TEXTURE_RETAIN);

	// detail texture
	m_bDetailTexture =  Params.GetValueBool(STR_DETAILTEXTURE);
	m_strDetailName.from_utf8(Params.GetValueString(STR_DTEXTURE_NAME));
	m_fDetailScale = Params.GetValueFloat(STR_DTEXTURE_SCALE);
	m_fDetailDistance = Params.GetValueFloat(STR_DTEXTURE_DISTANCE);

	// culture
	m_bRoads =			Params.GetValueBool(STR_ROADS);
	m_strRoadFile.from_utf8(Params.GetValueString(STR_ROADFILE));
	m_bHwy =			Params.GetValueBool(STR_HWY);
	m_bPaved =			Params.GetValueBool(STR_PAVED);
	m_bDirt =			Params.GetValueBool(STR_DIRT);
	m_fRoadHeight =		Params.GetValueFloat(STR_ROADHEIGHT);
	m_fRoadDistance =	Params.GetValueFloat(STR_ROADDISTANCE);
	m_bTexRoads =		Params.GetValueBool(STR_TEXROADS);
	m_bRoadCulture =	Params.GetValueBool(STR_ROADCULTURE);

	m_bPlants =			Params.GetValueBool(STR_TREES);
	m_strVegFile.from_utf8(Params.GetValueString(STR_TREEFILE));
	m_iVegDistance =	Params.GetValueInt(STR_VEGDISTANCE);

	m_bFog =			Params.GetValueBool(STR_FOG);
	m_fFogDistance =	Params.GetValueFloat(STR_FOGDISTANCE);

	// Layers and structure stuff
	m_Layers = Params.m_Layers;
	m_iStructDistance = Params.GetValueInt(STR_STRUCTDIST);
	m_bStructureShadows = Params.GetValueBool(STR_STRUCT_SHADOWS);
	m_iStructureRez = vt_log2(Params.GetValueInt(STR_SHADOW_REZ))-8;
	m_fDarkness =		Params.GetValueFloat(STR_SHADOW_DARKNESS);
	m_strContent =	  Params.GetValueString(STR_CONTENT_FILE);

	m_bVehicles =	   Params.GetValueBool(STR_VEHICLES);
//  m_fVehicleSize =	Params.GetValueFloat(STR_VEHICLESIZE);
//  m_fVehicleSpeed =   Params.GetValueFloat(STR_VEHICLESPEED);

	m_bSky =			Params.GetValueBool(STR_SKY);
	m_strSkyTexture.from_utf8(Params.GetValueString(STR_SKYTEXTURE));
	m_bOceanPlane =	 Params.GetValueBool(STR_OCEANPLANE);
	m_fOceanPlaneLevel = Params.GetValueFloat(STR_OCEANPLANELEVEL);
	m_bDepressOcean =   Params.GetValueBool(STR_DEPRESSOCEAN);
	m_fDepressOceanLevel = Params.GetValueFloat(STR_DEPRESSOCEANLEVEL);
	m_bHorizon =		Params.GetValueBool(STR_HORIZON);
	RGBi col = 			Params.GetValueRGBi(STR_BGCOLOR);
	m_BgColor.Set(col.r, col.g, col.b);

	m_bRouteEnable =	Params.GetValueBool(STR_ROUTEENABLE);
	const char *routefile = Params.GetValueString(STR_ROUTEFILE);
	if (routefile)
		m_strRouteFile.from_utf8(routefile);

	vtString fname;
	if (Params.GetOverlay(fname, m_iOverlayX, m_iOverlayY))
		m_strOverlayFile = fname;

	// Safety check
	if (m_iTriCount < 500 || m_iTriCount > 100000)
		m_iTriCount = 10000;
	m_Scenarios = Params.m_Scenarios;

	VTLOG("   Finished SetParams\n");
}

//
// get the values from the dialog into the supplied paramter structure
//
void TParamsDlg::GetParams(TParams &Params)
{
	VTLOG("TParamsDlg::GetParams\n");
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// overall name
	Params.SetValueString(STR_NAME, m_strTerrainName.to_utf8());

	// elevation
	if (m_bGrid)
	{
		Params.SetValueInt(STR_SURFACE_TYPE, 0);
		Params.SetValueString(STR_ELEVFILE, m_strFilename.to_utf8());
	}
	if (m_bTin)
	{
		Params.SetValueInt(STR_SURFACE_TYPE, 1);
		Params.SetValueString(STR_ELEVFILE, m_strFilenameTin.to_utf8());
	}
	if (m_bTileset)
	{
		Params.SetValueInt(STR_SURFACE_TYPE, 2);
		Params.SetValueString(STR_ELEVFILE, m_strFilenameTileset.to_utf8());
	}
	Params.SetValueFloat(STR_VERTICALEXAG, m_fVerticalExag);

	// navigation
	Params.SetValueFloat(STR_MINHEIGHT, m_fMinHeight);
	Params.SetValueInt(STR_NAVSTYLE, m_iNavStyle);
	Params.SetValueFloat(STR_NAVSPEED, m_fNavSpeed);
	Params.SetValueString(STR_LOCFILE, m_strLocFile.to_utf8());
	Params.SetValueString(STR_INITLOCATION, m_strInitLocation.to_utf8());
	Params.SetValueFloat(STR_HITHER, m_fHither);
	Params.SetValueBool(STR_ACCEL, m_bAccel);
	Params.m_AnimPaths = m_AnimPaths;

	// LOD
	Params.SetLodMethod((enum LodMethodEnum) m_iLodMethod);
	Params.SetValueInt(STR_TRICOUNT, m_iTriCount);
	Params.SetValueBool(STR_TRISTRIPS, m_bTriStrips);
	Params.SetValueInt(STR_VERTCOUNT, m_iVertCount);
	Params.SetValueInt(STR_TILE_CACHE_SIZE, m_iTileCacheSize);

	// time
	Params.SetValueBool(STR_TIMEON, m_bTimeOn);
	Params.SetValueString(STR_INITTIME, m_InitTime.GetAsString());
	Params.SetValueFloat(STR_TIMESPEED, m_fTimeSpeed);

	// texture
	Params.SetTextureEnum((enum TextureEnum)m_iTexture);
	if (m_iTexture == 1)	// single
	{
		Params.SetValueString(STR_TEXTUREFILE, m_strTextureSingle.to_utf8());
	}
	else if (m_iTexture == 2)	// tile4x4
	{
		Params.SetValueInt(STR_TILESIZE, m_iTilesize);
		Params.SetValueString(STR_TEXTUREBASE, m_strTextureBase.to_utf8());
		Params.SetValueInt(STR_TEXTUREFORMAT, (int) m_bJPEG);
	}
	else if (m_iTexture == 3)	// derived
	{
		Params.SetValueString(STR_COLOR_MAP, m_strColorMap.to_utf8());
	}
	else if (m_iTexture == 4)	// tileset
	{
		Params.SetValueString(STR_TEXTUREFILE, m_strTextureTileset.to_utf8());
	}
	Params.SetValueBool(STR_MIPMAP, m_bMipmap);
	Params.SetValueBool(STR_REQUEST16BIT, m_b16bit);
	Params.SetValueBool(STR_PRELIGHT, m_bPreLight);
	Params.SetValueFloat(STR_PRELIGHTFACTOR, m_fPreLightFactor);
	Params.SetValueBool(STR_CAST_SHADOWS, m_bCastShadows);
	Params.SetValueBool(STR_TEXTURE_RETAIN, m_bTextureRetain);

	// detail texture
	Params.SetValueBool(STR_DETAILTEXTURE, m_bDetailTexture);
	Params.SetValueString(STR_DTEXTURE_NAME, m_strDetailName.to_utf8());
	Params.SetValueFloat(STR_DTEXTURE_SCALE, m_fDetailScale);
	Params.SetValueFloat(STR_DTEXTURE_DISTANCE, m_fDetailDistance);

	// culture
	Params.SetValueBool(STR_ROADS, m_bRoads);
	Params.SetValueString(STR_ROADFILE, m_strRoadFile.to_utf8());
	Params.SetValueBool(STR_HWY, m_bHwy);
	Params.SetValueBool(STR_PAVED, m_bPaved);
	Params.SetValueBool(STR_DIRT, m_bDirt);
	Params.SetValueFloat(STR_ROADHEIGHT, m_fRoadHeight);
	Params.SetValueFloat(STR_ROADDISTANCE, m_fRoadDistance);
	Params.SetValueBool(STR_TEXROADS, m_bTexRoads);
	Params.SetValueBool(STR_ROADCULTURE, m_bRoadCulture);

	Params.SetValueBool(STR_TREES, m_bPlants);
	Params.SetValueString(STR_TREEFILE, m_strVegFile.to_utf8());
	Params.SetValueInt(STR_VEGDISTANCE, m_iVegDistance);

	Params.SetValueBool(STR_FOG, m_bFog);
	Params.SetValueFloat(STR_FOGDISTANCE, m_fFogDistance);
	// (fog color not exposed in UI)

	// Layers and structure stuff
	Params.m_Layers = m_Layers;

	Params.SetValueInt(STR_STRUCTDIST, m_iStructDistance);
	Params.SetValueBool(STR_STRUCT_SHADOWS, m_bStructureShadows);
	Params.SetValueInt(STR_SHADOW_REZ, 1 << (m_iStructureRez+8));
	Params.SetValueFloat(STR_SHADOW_DARKNESS, m_fDarkness);
	Params.SetValueString(STR_CONTENT_FILE, m_strContent.to_utf8());

	Params.SetValueBool(STR_VEHICLES, m_bVehicles);
//  Params.SetValueFloat(STR_VEHICLESIZE, m_fVehicleSize);
//  Params.SetValueFloat(STR_VEHICLESPEED, m_fVehicleSpeed);

	Params.SetValueBool(STR_SKY, m_bSky);
	Params.SetValueString(STR_SKYTEXTURE, m_strSkyTexture.to_utf8());

	Params.SetValueBool(STR_OCEANPLANE, m_bOceanPlane);
	Params.SetValueFloat(STR_OCEANPLANELEVEL, m_fOceanPlaneLevel);
	Params.SetValueBool(STR_DEPRESSOCEAN, m_bDepressOcean);
	Params.SetValueFloat(STR_DEPRESSOCEANLEVEL, m_fDepressOceanLevel);
	Params.SetValueBool(STR_HORIZON, m_bHorizon);
	RGBi col(m_BgColor.Red(), m_BgColor.Green(), m_BgColor.Blue());
	Params.SetValueRGBi(STR_BGCOLOR, col);

	Params.SetValueBool(STR_ROUTEENABLE, m_bRouteEnable);
	Params.SetValueString(STR_ROUTEFILE, m_strRouteFile.to_utf8());

	Params.SetOverlay(m_strOverlayFile.vt_str(), m_iOverlayX, m_iOverlayY);

	Params.m_Scenarios = m_Scenarios;

}

void TParamsDlg::UpdateTiledTextureFilename()
{
	m_strTextureFilename.Printf(_T("%s%d"), m_strTextureBase.c_str(),
		NTILES * (m_iTilesize-1) + 1);
	if (m_bJPEG)
		m_strTextureFilename += _T(".jpg");
	else
		m_strTextureFilename += _T(".bmp");
	TransferDataToWindow();
}

void TParamsDlg::UpdateEnableState()
{
	GetFilename()->Enable(m_bGrid);
	GetFilenameTin()->Enable(m_bTin);
	GetFilenameTileset()->Enable(m_bTileset);

	FindWindow(ID_LODMETHOD)->Enable(m_bGrid);
	FindWindow(ID_TRI_COUNT)->Enable(m_bGrid && m_iLodMethod != LM_TOPOVISTA);
	FindWindow(ID_TRISTRIPS)->Enable(m_bGrid && m_iLodMethod == LM_MCNALLY);
	FindWindow(ID_DETAILTEXTURE)->Enable(m_bGrid);
	FindWindow(ID_VTX_COUNT)->Enable(m_bTileset);
	FindWindow(ID_TILE_CACHE_SIZE)->Enable(m_bTileset);

	FindWindow(ID_NONE)->Enable(!m_bTileset);
	FindWindow(ID_SINGLE)->Enable(!m_bTileset);
	FindWindow(ID_DERIVED)->Enable(!m_bTileset);
	FindWindow(ID_TILED_4BY4)->Enable(!m_bTileset);
	FindWindow(ID_TILESET)->Enable(m_bTileset);

	FindWindow(ID_TFILE_SINGLE)->Enable(m_iTexture == TE_SINGLE);
	FindWindow(ID_CHOICE_COLORS)->Enable(m_iTexture == TE_DERIVED);
	FindWindow(ID_EDIT_COLORS)->Enable(m_iTexture == TE_DERIVED);
	FindWindow(ID_TILESIZE)->Enable(m_iTexture == TE_TILED);
	FindWindow(ID_TFILE_BASE)->Enable(m_iTexture == TE_TILED);
	FindWindow(ID_JPEG)->Enable(m_iTexture == TE_TILED);
	FindWindow(ID_TFILENAME)->Enable(m_iTexture == TE_TILED);
	FindWindow(ID_TFILE_TILESET)->Enable(m_iTexture == TE_TILESET);

	FindWindow(ID_MIPMAP)->Enable(m_iTexture != TE_NONE && !m_bTileset);
	FindWindow(ID_16BIT)->Enable(m_iTexture != TE_NONE && !m_bTileset);
	FindWindow(ID_PRELIGHT)->Enable(m_iTexture != TE_NONE && !m_bTileset);
	FindWindow(ID_LIGHT_FACTOR)->Enable(m_iTexture != TE_NONE && !m_bTileset);
	FindWindow(ID_CAST_SHADOWS)->Enable(m_iTexture != TE_NONE && !m_bTileset);
	FindWindow(ID_RETAIN)->Enable(m_iTexture != TE_NONE && !m_bTileset);

	FindWindow(ID_DETAILTEXTURE)->Enable(m_iLodMethod == LM_MCNALLY);
	FindWindow(ID_DT_NAME)->Enable(m_iLodMethod == LM_MCNALLY && m_bDetailTexture);
	FindWindow(ID_DT_SCALE)->Enable(m_iLodMethod == LM_MCNALLY && m_bDetailTexture);
	FindWindow(ID_DT_DISTANCE)->Enable(m_iLodMethod == LM_MCNALLY && m_bDetailTexture);

	FindWindow(ID_TREEFILE)->Enable(m_bPlants);
//  FindWindow(ID_VEGDISTANCE)->Enable(m_bPlants); // user might want to adjust

	FindWindow(ID_ROADFILE)->Enable(m_bRoads);
	FindWindow(ID_ROADHEIGHT)->Enable(m_bRoads);
	FindWindow(ID_ROADDISTANCE)->Enable(m_bRoads);
	FindWindow(ID_TEXROADS)->Enable(m_bRoads);
	FindWindow(ID_ROADCULTURE)->Enable(m_bRoads);
	FindWindow(ID_HIGHWAYS)->Enable(m_bRoads);
	FindWindow(ID_PAVED)->Enable(m_bRoads);
	FindWindow(ID_DIRT)->Enable(m_bRoads);

	FindWindow(ID_CHOICE_SHADOW_REZ)->Enable(m_bStructureShadows);
	FindWindow(ID_DARKNESS)->Enable(m_bStructureShadows);

	GetOceanPlaneOffset()->Enable(m_bOceanPlane);
	GetDepressOceanOffset()->Enable(m_bDepressOcean);
	GetSkytexture()->Enable(m_bSky);
	GetSkytexture()->Enable(m_bSky);
	GetFogDistance()->Enable(m_bFog);
	int iSelected = m_pScenarioList->GetSelection();
	if (iSelected != wxNOT_FOUND)
	{
		GetEditScenario()->Enable(true);
		GetDeleteScenario()->Enable(true);
		if (iSelected != (m_pScenarioList->GetCount() - 1))
			GetMovedownScenario()->Enable(true);
		else
			GetMovedownScenario()->Enable(false);
	   if (iSelected != 0)
			GetMoveupScenario()->Enable(true);
		else
 			GetMoveupScenario()->Enable(false);
	}
	else
	{
		GetEditScenario()->Enable(false);
		GetDeleteScenario()->Enable(false);
		GetMoveupScenario()->Enable(false);
		GetMovedownScenario()->Enable(false);
	}
}

void TParamsDlg::RefreshLocationFields()
{
	m_pLocField->Clear();
	m_pLocField->Append(_("(default)"));

	vtString fname = "Locations/";
	fname += m_strLocFile.mb_str();
	vtString path = FindFileOnPaths(m_datapaths, fname);
	if (path == "")
		return;
	vtLocationSaver saver;
	if (!saver.Read(path))
		return;

	int i, num = saver.GetNumLocations();
	for (i = 0; i < num; i++)
	{
		vtLocation *loc = saver.GetLocation(i);
		wxString2 str;
		str = loc->m_strName;
		m_pLocField->Append(str);
	}
	if (num)
	{
		if (m_iInitLocation < 0)
			m_iInitLocation = 0;
		if (m_iInitLocation > num-1)
			m_iInitLocation = num-1;
	}
}

void TParamsDlg::UpdateColorMapChoice()
{
	m_pColorMap->Clear();
	vtStringArray &paths = m_datapaths;
	for (unsigned int i = 0; i < paths.size(); i++)
	{
		// fill the "colormap" control with available colormap files
		AddFilenamesToChoice(m_pColorMap, paths[i] + "GeoTypical", "*.cmt");
		int sel = m_pColorMap->FindString(m_strColorMap);
		if (sel != -1)
			m_pColorMap->SetSelection(sel);
	}
}

void TParamsDlg::DeleteItem(wxListBox *pBox)
{
	wxString2 fname1 = pBox->GetStringSelection();

	// might be a layer
	int idx = FindLayerByFilename(fname1);
	if (idx != -1)
		m_Layers.erase(m_Layers.begin()+idx);

	// or an animpath
	for (unsigned int i = 0; i < m_AnimPaths.size(); i++)
		if (!fname1.vt_str().Compare(m_AnimPaths[i]))
		{
			m_AnimPaths.erase(m_AnimPaths.begin()+i);
			break;
		}
}

int TParamsDlg::FindLayerByFilename(const wxString2 &fname)
{
	vtString fname2 = fname.vt_str();
	for (unsigned int i = 0; i < m_Layers.size(); i++)
		if (fname2 == m_Layers[i].GetValueString("Filename"))
			return (int) i;
	return -1;
}


// WDR: handler implementations for TParamsDlg

void TParamsDlg::OnBgColor( wxCommandEvent &event )
{
	wxColourData data;
	data.SetChooseFull(true);
	data.SetColour(m_BgColor);

	wxColourDialog dlg(this, &data);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxColourData data2 = dlg.GetColourData();
		m_BgColor = data2.GetColour();
		UpdateColorControl();
	}
}

void TParamsDlg::OnTextureFileBase( wxCommandEvent &event )
{
	if (m_bSetting || !m_bReady) return;
	TransferDataFromWindow();
	UpdateTiledTextureFilename();
}

void TParamsDlg::OnTileSize( wxCommandEvent &event )
{
	if (m_bSetting || !m_bReady) return;
	TransferDataFromWindow();
	UpdateTiledTextureFilename();
}

void TParamsDlg::OnInitDialog(wxInitDialogEvent& event)
{
	VTLOG("TParamsDlg::OnInitDialog\n");

	bool bShowProgress = (m_datapaths.size() > 1);
	if (bShowProgress)
		OpenProgressDialog(_T("Looking for files on data paths"), false, this);

	m_bReady = false;
	m_bSetting = true;

	unsigned int i;
	int sel;

	vtStringArray &paths = m_datapaths;

	for (i = 0; i < paths.size(); i++)
	{
		if (bShowProgress)
			UpdateProgressDialog(i * 100 / paths.size(), wxString2(paths[i]));

		// fill the "Grid filename" control with available files
		AddFilenamesToComboBox(m_pFilename, paths[i] + "Elevation", "*.bt*");
		sel = m_pFilename->FindString(m_strFilename);
		if (sel != -1)
			m_pFilename->SetSelection(sel);

		// fill the "TIN filename" control with available files
		AddFilenamesToComboBox(m_pFilenameTin, paths[i] + "Elevation", "*.tin");
		AddFilenamesToComboBox(m_pFilenameTin, paths[i] + "Elevation", "*.itf");
		sel = m_pFilenameTin->FindString(m_strFilenameTin);
		if (sel != -1)
			m_pFilenameTin->SetSelection(sel);

		// fill the "Tileset filename" control with available files
		AddFilenamesToComboBox(m_pFilenameTileset, paths[i] + "Elevation", "*.ini");
		sel = m_pFilenameTileset->FindString(m_strFilenameTileset);
		if (sel != -1)
			m_pFilenameTileset->SetSelection(sel);

		// fill the "single texture filename" control with available bitmap files
		AddFilenamesToComboBox(m_pTextureFileSingle, paths[i] + "GeoSpecific", "*.bmp");
		AddFilenamesToComboBox(m_pTextureFileSingle, paths[i] + "GeoSpecific", "*.jpg");
		AddFilenamesToComboBox(m_pTextureFileSingle, paths[i] + "GeoSpecific", "*.jpeg");
		AddFilenamesToComboBox(m_pTextureFileSingle, paths[i] + "GeoSpecific", "*.png");
		sel = m_pTextureFileSingle->FindString(m_strTextureTileset);
		if (sel != -1)
			m_pTextureFileSingle->SetSelection(sel);

		// fill the "texture Tileset filename" control with available files
		AddFilenamesToComboBox(m_pTextureFileTileset, paths[i] + "GeoSpecific", "*.ini");
		sel = m_pTextureFileTileset->FindString(m_strFilenameTileset);
		if (sel != -1)
			m_pTextureFileTileset->SetSelection(sel);

		// fill the "detail texture" control with available bitmap files
		AddFilenamesToComboBox(m_pDTName, paths[i] + "GeoTypical", "*.bmp");
		AddFilenamesToComboBox(m_pDTName, paths[i] + "GeoTypical", "*.jpg");
		AddFilenamesToComboBox(m_pDTName, paths[i] + "GeoTypical", "*.png");
		sel = m_pDTName->FindString(m_strDetailName);
		if (sel != -1)
			m_pDTName->SetSelection(sel);

		// fill the Location files
		AddFilenamesToComboBox(m_pLocFile, paths[i] + "Locations", "*.loc");
		sel = m_pLocFile->FindString(m_strLocFile);
		if (sel != -1)
			m_pLocFile->SetSelection(sel);

		// fill in Road files
		AddFilenamesToComboBox(m_pRoadFile, paths[i] + "RoadData", "*.rmf");
		sel = m_pRoadFile->FindString(m_strRoadFile);
		if (sel != -1)
			m_pRoadFile->SetSelection(sel);

		// fill in Vegetation files
		AddFilenamesToComboBox(m_pTreeFile, paths[i] + "PlantData", "*.vf");
		AddFilenamesToComboBox(m_pTreeFile, paths[i] + "PlantData", "*.shp");
		sel = m_pTreeFile->FindString(m_strVegFile);
		if (sel != -1)
			m_pTreeFile->SetSelection(sel);

		// fill in Content file
		AddFilenamesToComboBox(GetContentFile(), m_datapaths[i], "*.vtco");
		sel = GetContentFile()->FindString(m_strContent);
		if (sel != -1)
			GetContentFile()->SetSelection(sel);

		// fill in Sky files
		AddFilenamesToComboBox(m_pSkyTexture, paths[i] + "Sky", "*.bmp");
		AddFilenamesToComboBox(m_pSkyTexture, paths[i] + "Sky", "*.png");
		AddFilenamesToComboBox(m_pSkyTexture, paths[i] + "Sky", "*.jpg");
		sel = m_pSkyTexture->FindString(m_strSkyTexture);
		if (sel != -1)
			m_pSkyTexture->SetSelection(sel);
	}
	UpdateColorMapChoice();

	m_pLodMethod->Clear();
	m_pLodMethod->Append(_T("Roettger"));
	m_pLodMethod->Append(_T("TopoVista"));
	m_pLodMethod->Append(_T("McNally"));
	m_pLodMethod->Append(_T("Demeter"));
	m_pLodMethod->Append(_T("Custom"));
	m_pLodMethod->Append(_T("BryanQuad"));
	// add your own LOD method here!

	m_pLodMethod->SetSelection(m_iLodMethod);

	m_pShadowRez->Clear();
	m_pShadowRez->Append(_T("256"));
	m_pShadowRez->Append(_T("512"));
	m_pShadowRez->Append(_T("1024"));
	m_pShadowRez->Append(_T("2048"));
	m_pShadowRez->Append(_T("4096"));

	m_pNavStyle->Clear();
	m_pNavStyle->Append(_("Normal Terrain Flyer"));
	m_pNavStyle->Append(_("Terrain Flyer with Velocity"));
	m_pNavStyle->Append(_("Grab-Pivot"));
//  m_pNavStyle->Append(_("Quake-Style Walk"));
	m_pNavStyle->Append(_("Panoramic Flyer"));

	RefreshLocationFields();

//  DetermineTerrainSizeFromBT();
//  DetermineSizeFromBMP();

//  OnChangeMem();
	UpdateTiledTextureFilename();

	GetUseGrid()->SetValue(m_bGrid);
	GetUseTin()->SetValue(m_bTin);
	GetUseTileset()->SetValue(m_bTileset);

	UpdateTimeString();

	m_iInitLocation = m_pLocField->FindString(m_strInitLocation);
	if (m_iInitLocation == -1)
		m_iInitLocation = 0;

	UpdateColorControl();

	wxWindow::OnInitDialog(event);

	UpdateEnableState();

	if (bShowProgress)
		CloseProgressDialog();

	m_bReady = true;
}

bool TParamsDlg::TransferDataToWindow()
{
	m_bSetting = true;

	m_pNone->SetValue(m_iTexture == TE_NONE);
	m_pSingle->SetValue(m_iTexture == TE_SINGLE);
	m_pDerived->SetValue(m_iTexture == TE_DERIVED);
	m_pTiled->SetValue(m_iTexture == TE_TILED);
	m_pTileset->SetValue(m_iTexture == TE_TILESET);

	unsigned int i;
	m_pStructFiles->Clear();
	m_pRawFiles->Clear();
	for (i = 0; i < m_Layers.size(); i++)
	{
		vtString ltype = m_Layers[i].GetValueString("Type");
		vtString fname = m_Layers[i].GetValueString("Filename");
		wxString2 fname2;
		fname2.from_utf8(fname);

		if (ltype == TERR_LTYPE_STRUCTURE)
			m_pStructFiles->Append(fname2);
		if (ltype == TERR_LTYPE_ABSTRACT)
			m_pRawFiles->Append(fname2);
	}
	m_pStructFiles->Append(_("(double-click to add files)"));
	m_pRawFiles->Append(_("(double-click to add files)"));

	m_pAnimFiles->Clear();
	for (i = 0; i < m_AnimPaths.size(); i++)
		m_pAnimFiles->Append(wxString2(m_AnimPaths[i]));
	m_pAnimFiles->Append(_("(double-click to add files)"));

	wxString2 str;
	m_pScenarioList->Clear();
	for (i = 0; i < m_Scenarios.size(); i++)
	{
		str.from_utf8(m_Scenarios[i].GetValueString(STR_SCENARIO_NAME));
		m_pScenarioList->Append(str);
	}

	bool result = wxDialog::TransferDataToWindow();
	m_bSetting = false;

	return result;
}

bool TParamsDlg::TransferDataFromWindow()
{
	if (m_pNone->GetValue()) m_iTexture = TE_NONE;
	if (m_pSingle->GetValue()) m_iTexture = TE_SINGLE;
	if (m_pDerived->GetValue()) m_iTexture = TE_DERIVED;
	if (m_pTiled->GetValue()) m_iTexture = TE_TILED;
	if (m_pTileset->GetValue()) m_iTexture = TE_TILESET;

	return wxDialog::TransferDataFromWindow();
}

void TParamsDlg::UpdateColorControl()
{
	FillWithColor(GetColorBitmap(), m_BgColor);
}

void TParamsDlg::OnTextureNone( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	if (event.IsChecked())
	{
		// turn off "Prelighting" if there is no texture
		TransferDataFromWindow();
		m_bPreLight = false;
		TransferDataToWindow();
	}
	UpdateEnableState();
}

void TParamsDlg::OnTextureSingle( wxCommandEvent &event )
{
	if (m_bSetting || !event.IsChecked())
		return;
	TransferDataFromWindow();
	UpdateEnableState();
}

void TParamsDlg::OnTextureDerived( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	if (event.IsChecked())
	{
		// turn on "Prelighting" if the user wants a derived texture
		TransferDataFromWindow();
		m_bPreLight = true;
		TransferDataToWindow();
	}
	UpdateEnableState();
}

void TParamsDlg::OnTextureTiled( wxCommandEvent &event )
{
	if (m_bSetting || !event.IsChecked())
		return;
	TransferDataFromWindow();
	UpdateEnableState();
}

void TParamsDlg::OnTextureTileset( wxCommandEvent &event )
{
	if (m_bSetting || !event.IsChecked())
		return;
	TransferDataFromWindow();
	UpdateEnableState();
}

void TParamsDlg::OnEditColors( wxCommandEvent &event )
{
	TransferDataFromWindow();

	// Look on data paths, to give a complete path to the dialog
	vtString name = "GeoTypical/";
	name += m_strColorMap.mb_str();
	name = FindFileOnPaths(m_datapaths, name);
	if (name == "")
	{
		wxMessageBox(_("Couldn't locate file."));
		return;
	}

	ColorMapDlg dlg(this, -1, _("ColorMap"));
	dlg.SetFile(name);
	dlg.ShowModal();

	// They may have added or removed some color map files on the data path
	UpdateColorMapChoice();
}

void TParamsDlg::OnCheckBox( wxCommandEvent &event )
{
	TransferDataFromWindow();

	// the tileset elevation and tileset texture only work with each other
	if (event.GetId() == ID_USE_TILESET && event.IsChecked())
		m_iTexture = TE_TILESET;
	if (event.GetId() != ID_USE_TILESET  && event.IsChecked() && m_iTexture == TE_TILESET)
		m_iTexture = TE_NONE;

	UpdateEnableState();
	UpdateTiledTextureFilename();
}

//
// This function is used to find all files in a given directory,
// and if they match a wildcard, add them to a combo box.
//
void AddFilenamesToArray(wxArrayString &array, const char *directory,
	const char *wildcard)
{
	int entries = 0, matches = 0;

	wxString2 wildstr = wildcard;
	for (dir_iter it((const char *)directory); it != dir_iter(); ++it)
	{
		entries++;
		std::string name1 = it.filename();
		//	  VTLOG("   entry: '%s'", name1.c_str());
		if (it.is_hidden() || it.is_directory())
			continue;

		wxString2 name = name1.c_str();
		if (name.Matches(wildstr))
		{
			array.Add(name);
			matches++;
		}
	}
}

void TParamsDlg::OnListDblClickStructure( wxCommandEvent &event )
{
	unsigned int i;
	wxArrayString strings;

	for (i = 0; i < m_datapaths.size(); i++)
		AddFilenamesToArray(strings, m_datapaths[i] + "BuildingData", "*.vtst*");

	wxString2 result = wxGetSingleChoice(_("One of the following to add:"), _("Choose a structure file"),
		strings, this);

	if (result.Cmp(_T(""))) // user selected something
	{
		TransferDataFromWindow();
		vtTagArray lay;
		lay.SetValueString("Type", TERR_LTYPE_STRUCTURE, true);
		lay.SetValueString("Filename", result.vt_str(), true);
		m_Layers.push_back(lay);
		TransferDataToWindow();
	}
}

void TParamsDlg::OnListDblClickRaw( wxCommandEvent &event )
{
	unsigned int i;
	wxArrayString strings;

	for (i = 0; i < m_datapaths.size(); i++)
	{
		AddFilenamesToArray(strings, m_datapaths[i] + "PointData", "*.shp");
		AddFilenamesToArray(strings, m_datapaths[i] + "PointData", "*.igc");
		AddFilenamesToArray(strings, m_datapaths[i] + "PointData", "*.dxf");
	}

	wxString2 result = wxGetSingleChoice(_("One of the following to add:"), _("Choose a structure file"),
		strings, this);

	if (result.Cmp(_T(""))) // user selected something
	{
		vtTagArray lay;
		lay.SetValueString("Type", TERR_LTYPE_ABSTRACT, true);
		lay.SetValueString("Filename", result.vt_str(), true);
		m_Layers.push_back(lay);
		TransferDataToWindow();
	}
}

void TParamsDlg::OnListDblClickAnimPaths( wxCommandEvent &event )
{
	unsigned int i;
	wxArrayString strings;

	for (i = 0; i < m_datapaths.size(); i++)
		AddFilenamesToArray(strings, m_datapaths[i] + "Locations", "*.vtap");

	wxString2 result = wxGetSingleChoice(_("One of the following to add:"), _("Choose an animpath file"),
		strings, this);

	if (result.Cmp(_T(""))) // user selected something
	{
		m_AnimPaths.push_back(result.vt_str());
		TransferDataToWindow();
	}
}

void TParamsDlg::OnChoiceLocFile( wxCommandEvent &event )
{
	if (m_bSetting || !m_bReady) return;

	wxString2 prev = m_strLocFile;
	TransferDataFromWindow();
	if (m_strLocFile != prev)
	{
		RefreshLocationFields();
		m_bSetting = true;
		TransferDataToWindow();
		m_bSetting = false;
	}
}

void TParamsDlg::OnChoiceInitLocation( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_strInitLocation = m_pLocField->GetString(m_iInitLocation);
}

void TParamsDlg::OnSetInitTime( wxCommandEvent &event )
{
	TransferDataFromWindow();

	TimeDlg dlg(this, -1, _("Set Initial Time"));
	dlg.AddOkCancel();
	dlg.SetTime(m_InitTime);
	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetTime(m_InitTime);
		UpdateTimeString();
		m_bSetting = true;
		TransferDataToWindow();
		m_bSetting = false;
	}
}

void TParamsDlg::OnStyle( wxCommandEvent &event )
{
	wxString2 str = GetRawFiles()->GetStringSelection();
	int idx = FindLayerByFilename(str);
	if (idx == -1)
		return;

	StyleDlg dlg(this, -1, _("Feature Style"));
	dlg.SetOptions(m_datapaths, m_Layers[idx]);
	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetOptions(m_Layers[idx]);
	}
}

void TParamsDlg::OnOverlay( wxCommandEvent &event )
{
	TransferDataFromWindow();

	wxFileDialog loadFile(NULL, _("Overlay Image File"), _T(""), _T(""),
		_("Image Files (*.png,*.jpg,*.bmp)|*.png;*.jpg;*.bmp"), wxOPEN);
	if (m_strOverlayFile != _T(""))
		loadFile.SetPath(m_strOverlayFile);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	m_strOverlayFile = loadFile.GetPath();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void TParamsDlg::UpdateTimeString()
{
	m_strInitTime = asctime(&m_InitTime.GetTM());

	// asctime has a weird habit of putting a LF at the end
	m_strInitTime.Trim();
}

void TParamsDlg::OnNewScenario( wxCommandEvent &event )
{
	wxString2 ScenarioName = wxGetTextFromUser(_("Enter Scenario Name"), _("New Scenario"));

	if (!ScenarioName.IsEmpty())
	{
		ScenarioParams Scenario;

		Scenario.SetValueString(STR_SCENARIO_NAME, ScenarioName.to_utf8(), true);
		m_Scenarios.push_back(Scenario);
		m_pScenarioList->SetSelection(m_pScenarioList->Append(ScenarioName));
		UpdateEnableState();
	}
}

void TParamsDlg::OnDeleteScenario( wxCommandEvent &event )
{
	int iSelected = m_pScenarioList->GetSelection();

	if (iSelected != wxNOT_FOUND)
	{
		m_pScenarioList->Delete(iSelected);
		m_Scenarios.erase(m_Scenarios.begin() + iSelected);
		UpdateEnableState();
	}
}

void TParamsDlg::OnEditScenario( wxCommandEvent &event )
{
	CScenarioParamsDialog ScenarioParamsDialog(this, -1, _("Scenario Parameters"));
	int iSelected = m_pScenarioList->GetSelection();
	
	if (iSelected != wxNOT_FOUND)
	{
		ScenarioParamsDialog.SetAvailableLayers(m_Layers);
		ScenarioParamsDialog.SetParams(m_Scenarios[iSelected]);

		if (wxID_OK == ScenarioParamsDialog.ShowModal())
		{
			if (ScenarioParamsDialog.IsModified())
			{
				wxString2 str;
				str.from_utf8(m_Scenarios[iSelected].GetValueString(STR_SCENARIO_NAME));
				m_Scenarios[iSelected] = ScenarioParamsDialog.GetParams();
				m_pScenarioList->SetString(iSelected, str);
			}
		}
	}
}

void TParamsDlg::OnMoveUpScenario( wxCommandEvent &event )
{
	int iSelected = m_pScenarioList->GetSelection();

	if ((iSelected != wxNOT_FOUND) && (iSelected != 0))
	{
		ScenarioParams TempParams = m_Scenarios[iSelected];
		wxString TempString = m_pScenarioList->GetString(iSelected);
		m_pScenarioList->Delete(iSelected);
// Bug in wxWindows
//		m_pScenarioList->SetSelection(m_pScenarioList->Insert(TempString, iSelected - 1));
		m_pScenarioList->Insert(TempString, iSelected - 1);
		m_pScenarioList->SetSelection(iSelected - 1);
		m_Scenarios.erase(m_Scenarios.begin() + iSelected);
		m_Scenarios.insert(m_Scenarios.begin() + iSelected - 1,TempParams);
		UpdateEnableState();
	}
}

void TParamsDlg::OnMoveDownSceanario( wxCommandEvent &event )
{
	int iSelected = m_pScenarioList->GetSelection();

	if ((iSelected != wxNOT_FOUND) && (iSelected != (m_pScenarioList->GetCount() - 1)))
	{
		ScenarioParams TempParams = m_Scenarios[iSelected];
		wxString TempString = m_pScenarioList->GetString(iSelected);
		m_pScenarioList->Delete(iSelected);
// Bug in wxWindows
//		m_pScenarioList->SetSelection(m_pScenarioList->Insert(TempString, iSelected + 1));
		m_pScenarioList->Insert(TempString, iSelected + 1);
		m_pScenarioList->SetSelection(iSelected + 1);
		m_Scenarios.erase(m_Scenarios.begin() + iSelected);
		m_Scenarios.insert(m_Scenarios.begin() + iSelected + 1,TempParams);
		UpdateEnableState();
	}
}

void TParamsDlg::OnScenarioListEvent( wxCommandEvent &event )
{
	UpdateEnableState();
}
