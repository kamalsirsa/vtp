//
// Name: TParamsDlg.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "TParamsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/image.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Location.h"
#include "vtdata/Features.h"		// for RefreshLabelFields()
#include "vtdata/FilePath.h"		// for FindFileOnPaths()
#include "vtui/Helper.h"
#include "TParamsDlg.h"

#define NTILES 4

//---------------------------------------------------------------------------

BEGIN_EVENT_TABLE(wxListBoxEventHandler, wxEvtHandler)
	EVT_CHAR(wxListBoxEventHandler::OnChar)
END_EVENT_TABLE()

wxListBoxEventHandler::wxListBoxEventHandler(TParamsDlg *dlg, wxListBox *box) : wxEvtHandler()
{
	m_pDlg = dlg;
	m_pBox = box;
}

void wxListBoxEventHandler::OnChar(wxKeyEvent& event)
{
	if (event.KeyCode() == WXK_DELETE)
	{
		int sel = m_pBox->GetSelection();
		if (sel != -1 && sel < m_pBox->GetCount()-1)
		{
			m_pBox->Delete(sel);
			m_pDlg->TransferDataFromWindow();
		}
	}
	event.Skip();
}

// WDR: class implementations

//----------------------------------------------------------------------------
// TParamsDlg
//----------------------------------------------------------------------------

// WDR: event table for TParamsDlg

BEGIN_EVENT_TABLE(TParamsDlg,AutoDialog)
	EVT_RADIOBUTTON( ID_USE_GRID, TParamsDlg::OnCheckBox )
	EVT_RADIOBUTTON( ID_USE_TIN, TParamsDlg::OnCheckBox )

	EVT_TEXT( ID_TILESIZE, TParamsDlg::OnTileSize )
	EVT_TEXT( ID_TFILEBASE, TParamsDlg::OnTextureFileBase )

	EVT_RADIOBUTTON( ID_NONE, TParamsDlg::OnTextureNone )
	EVT_RADIOBUTTON( ID_SINGLE, TParamsDlg::OnTextureSingle )
	EVT_RADIOBUTTON( ID_DERIVED, TParamsDlg::OnTextureDerived )
	EVT_RADIOBUTTON( ID_TILED, TParamsDlg::OnTextureTiled )

	EVT_CHECKBOX( ID_JPEG, TParamsDlg::OnCheckBox )

	EVT_CHECKBOX( ID_PLANTS, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_ROADS, TParamsDlg::OnCheckBox )

	EVT_LISTBOX_DCLICK( ID_STRUCTFILES, TParamsDlg::OnListDblClick )

	EVT_CHECKBOX( ID_OCEANPLANE, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_DEPRESSOCEAN, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_SKY, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_LABELS, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_FOG, TParamsDlg::OnCheckBox )

	EVT_TEXT( ID_LABEL_FILE, TParamsDlg::OnChoiceLabelFile )
	EVT_TEXT( ID_LOCFILE, TParamsDlg::OnChoiceLocFile )
	EVT_CHOICE( ID_INIT_LOCATION, TParamsDlg::OnChoiceInitLocation )
END_EVENT_TABLE()

TParamsDlg::TParamsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	TParamsFunc( this, TRUE );

	// make sure that validation gets down to the child windows
	SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

	// including the children of the notebook
	wxNotebook *notebook = (wxNotebook*) FindWindow( ID_NOTEBOOK );
	notebook->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

	m_pBoxHandler = NULL;
}

TParamsDlg::~TParamsDlg()
{
	if (m_pBoxHandler)
		m_pStructFiles->PopEventHandler(true);
}

//
// set the values in the dialog from the supplied paramter structure
//
void TParamsDlg::SetParams(const TParams &Params)
{
	// overall name
	m_strTerrainName.from_utf8(Params.m_strName);

	// elevation
	m_strFilename = wxString::FromAscii((const char *)Params.m_strElevFile);
	m_strFilenameTin = wxString::FromAscii((const char *)Params.m_strElevFile);
	m_fVerticalExag = Params.m_fVerticalExag;
	m_bTin = Params.m_bTin;

	/// navigation
	m_iMinHeight = Params.m_iMinHeight;
	m_iNavStyle = Params.m_iNavStyle;
	m_fNavSpeed = Params.m_fNavSpeed;
	m_strLocFile = wxString::FromAscii((const char *)Params.m_strLocFile);
	m_strInitLocation.from_utf8(Params.m_strInitLocation);
	m_fHither = Params.m_fHither;
	m_bAccel = Params.m_bAccel;

	// LOD
	m_iLodMethod = Params.m_eLodMethod;
	m_fPixelError = Params.m_fPixelError;
	m_iTriCount = Params.m_iTriCount;
	m_bTriStrips = Params.m_bTriStrips;
	m_bDetailTexture = Params.m_bDetailTexture;

	// time
	m_bTimeOn = Params.m_bTimeOn;
	m_iInitTime = Params.m_iInitTime;
	m_fTimeSpeed = Params.m_fTimeSpeed;

	// texture
	m_iTexture = Params.m_eTexture;
	m_iTilesize = Params.m_iTilesize;
	m_strTextureSingle = wxString::FromAscii((const char *)Params.m_strTextureSingle);
	m_strTextureBase = wxString::FromAscii((const char *)Params.m_strTextureBase);
	m_bJPEG = Params.m_bJPEG;
	m_strTextureFilename = wxString::FromAscii((const char *)Params.m_strTextureFilename);
	m_bMipmap = Params.m_bMipmap;
	m_b16bit = Params.m_b16bit;

	// culture
	m_bRoads = Params.m_bRoads;
	m_strRoadFile = wxString::FromAscii((const char *)Params.m_strRoadFile);
	m_bHwy = Params.m_bHwy;
	m_bDirt = Params.m_bDirt;
	m_bPaved = Params.m_bPaved;
	m_fRoadHeight = Params.m_fRoadHeight;
	m_fRoadDistance = Params.m_fRoadDistance;
	m_bTexRoads = Params.m_bTexRoads;
	m_bRoadCulture = Params.m_bRoadCulture;

	m_bPlants = Params.m_bPlants;
	m_strVegFile = wxString::FromAscii((const char *)Params.m_strVegFile);
	m_iVegDistance = Params.m_iVegDistance;

	m_bFog = Params.m_bFog;
	m_fFogDistance = Params.m_fFogDistance;

	unsigned int i, num = Params.m_strStructFiles.size();
	for (i = 0; i < num; i++)
		m_strStructFiles.Append(new wxString2(Params.m_strStructFiles[i]));
	m_iStructDistance = Params.m_iStructDistance;

	m_bVehicles = Params.m_bVehicles;
//  m_fVehicleSize = Params.m_fVehicleSize;
//  m_fVehicleSpeed = Params.m_fVehicleSpeed;
//  m_iNumCars = Params.m_iNumCars;

	m_bSky = Params.m_bSky;
	m_strSkyTexture = wxString::FromAscii((const char *)Params.m_strSkyTexture);
	m_bHorizon = Params.m_bHorizon;
//  m_bOverlay = Params.m_bOverlay;
	m_bLabels = Params.m_bLabels;
	m_strLabelFile = Params.m_strLabelFile;
	m_Style = Params.m_Style;

	m_bOceanPlane = Params.m_bOceanPlane;
	m_fOceanPlaneLevel = Params.m_fOceanPlaneLevel;
	m_bDepressOcean = Params.m_bDepressOcean;
	m_fDepressOceanLevel = Params.m_fDepressOceanLevel;

	m_bPreLight = Params.m_bPreLight;
	m_bPreLit = Params.m_bPreLit;
	m_fPreLightFactor = Params.m_fPreLightFactor;

	m_strRouteFile = wxString::FromAscii((const char *)Params.m_strRouteFile);
	m_bRouteEnable = Params.m_bRouteEnable;

	// Safety check
	if (m_iTriCount < 500 || m_iTriCount > 100000)
		m_iTriCount = 10000;
}

//
// get the values from the dialog into the supplied paramter structure
//
void TParamsDlg::GetParams(TParams &Params)
{
	// overall name
	Params.m_strName = m_strTerrainName.to_utf8();

	// elevation
	if (m_bTin)
		Params.m_strElevFile = m_strFilenameTin.mb_str();
	else
		Params.m_strElevFile = m_strFilename.mb_str();
	Params.m_fVerticalExag = m_fVerticalExag;
	Params.m_bTin = m_bTin;

	// navigation
	Params.m_iMinHeight = m_iMinHeight;
	Params.m_fNavSpeed = m_fNavSpeed;
	Params.m_iNavStyle = m_iNavStyle;
	Params.m_strLocFile = m_strLocFile.mb_str();
	Params.m_strInitLocation = m_strInitLocation.to_utf8();
	Params.m_fHither = m_fHither;
	Params.m_bAccel = m_bAccel;

	// LOD
	Params.m_eLodMethod = (enum LodMethodEnum) m_iLodMethod;
	Params.m_fPixelError = m_fPixelError;
	Params.m_iTriCount = m_iTriCount;
	Params.m_bTriStrips = m_bTriStrips;
	Params.m_bDetailTexture = m_bDetailTexture;

	// time
	Params.m_bTimeOn = m_bTimeOn;
	Params.m_iInitTime = m_iInitTime;
	Params.m_fTimeSpeed = m_fTimeSpeed;

	// texture
	Params.m_eTexture = (enum TextureEnum)m_iTexture;
	Params.m_iTilesize = m_iTilesize;
	Params.m_strTextureSingle = m_strTextureSingle.mb_str();
	Params.m_strTextureBase = m_strTextureBase.mb_str();
	Params.m_bJPEG = m_bJPEG;
	Params.m_strTextureFilename = m_strTextureFilename.mb_str();
	Params.m_bMipmap = m_bMipmap;
	Params.m_b16bit = m_b16bit;

	Params.m_bRoads = m_bRoads;
	Params.m_strRoadFile = m_strRoadFile.mb_str();
	Params.m_bHwy = m_bHwy;
	Params.m_bPaved = m_bPaved;
	Params.m_bDirt = m_bDirt;
	Params.m_fRoadHeight = m_fRoadHeight;
	Params.m_fRoadDistance = m_fRoadDistance;
	Params.m_bTexRoads = m_bTexRoads;
	Params.m_bRoadCulture = m_bRoadCulture;

	Params.m_bPlants = m_bPlants;
	Params.m_strVegFile = m_strVegFile.mb_str();
	Params.m_iVegDistance = m_iVegDistance;

	Params.m_bFog = m_bFog;
	Params.m_fFogDistance = m_fFogDistance;

	Params.m_strStructFiles.clear();
	int i, num = m_strStructFiles.GetSize();
	for (i = 0; i < num; i++)
		Params.m_strStructFiles.push_back(vtString(m_strStructFiles[i]->mb_str()));
	Params.m_iStructDistance = m_iStructDistance;

	Params.m_bVehicles = m_bVehicles;
//  Params.m_fVehicleSize = m_fVehicleSize;
//  Params.m_fVehicleSpeed = m_fVehicleSpeed;
//  Params.m_iNumCars = m_iNumCars;
//  Params.m_bOverlay = m_bOverlay;

	Params.m_bSky = m_bSky;
	Params.m_strSkyTexture = m_strSkyTexture.mb_str();
	Params.m_bOceanPlane = m_bOceanPlane;
	Params.m_bHorizon = m_bHorizon;
	Params.m_bLabels = m_bLabels;
	Params.m_strLabelFile = m_strLabelFile.mb_str();
	Params.m_Style = m_Style;

	Params.m_bOceanPlane = m_bOceanPlane;
	Params.m_fOceanPlaneLevel = m_fOceanPlaneLevel;
	Params.m_bDepressOcean = m_bDepressOcean;
	Params.m_fDepressOceanLevel = m_fDepressOceanLevel;

	Params.m_bPreLight = m_bPreLight;
	Params.m_bPreLit = m_bPreLit;
	Params.m_fPreLightFactor = m_fPreLightFactor;

	Params.m_strRouteFile = m_strRouteFile.mb_str();
	Params.m_bRouteEnable = m_bRouteEnable;
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
	GetFilename()->Enable(!m_bTin);
	GetFilenameTin()->Enable(m_bTin);

	FindWindow(ID_LODMETHOD)->Enable(!m_bTin);
	FindWindow(ID_PIXELERROR)->Enable(!m_bTin);
	FindWindow(ID_TRICOUNT)->Enable(!m_bTin);
	FindWindow(ID_TRISTRIPS)->Enable(!m_bTin);
	FindWindow(ID_DETAILTEXTURE)->Enable(!m_bTin);

	FindWindow(ID_TFILESINGLE)->Enable(m_iTexture == TE_SINGLE);
	FindWindow(ID_TILESIZE)->Enable(m_iTexture == TE_TILED);
	FindWindow(ID_TFILEBASE)->Enable(m_iTexture == TE_TILED);
	FindWindow(ID_JPEG)->Enable(m_iTexture == TE_TILED);
	FindWindow(ID_TFILENAME)->Enable(m_iTexture == TE_TILED);

	FindWindow(ID_MIPMAP)->Enable(m_iTexture != TE_NONE);
	FindWindow(ID_16BIT)->Enable(m_iTexture != TE_NONE);
	FindWindow(ID_PRELIGHT)->Enable(m_iTexture != TE_NONE);
	FindWindow(ID_LIGHT_FACTOR)->Enable(m_iTexture != TE_NONE);
	FindWindow(ID_PRELIT)->Enable(m_iTexture != TE_NONE);

	FindWindow(ID_TREEFILE)->Enable(m_bPlants);
//	FindWindow(ID_VEGDISTANCE)->Enable(m_bPlants); // user might want to adjust

	FindWindow(ID_ROADFILE)->Enable(m_bRoads);
	FindWindow(ID_ROADHEIGHT)->Enable(m_bRoads);
	FindWindow(ID_ROADDISTANCE)->Enable(m_bRoads);
	FindWindow(ID_TEXROADS)->Enable(m_bRoads);
	FindWindow(ID_ROADCULTURE)->Enable(m_bRoads);
	FindWindow(ID_HIGHWAYS)->Enable(m_bRoads);
	FindWindow(ID_PAVED)->Enable(m_bRoads);
	FindWindow(ID_DIRT)->Enable(m_bRoads);

	GetOceanPlaneOffset()->Enable(m_bOceanPlane);
	GetDepressOceanOffset()->Enable(m_bDepressOcean);
	GetSkytexture()->Enable(m_bSky);
	GetSkytexture()->Enable(m_bSky);
	GetFogDistance()->Enable(m_bFog);

	GetLabelFile()->Enable(m_bLabels);
	FindWindow(ID_LABEL_FIELD)->Enable(m_bLabels);
	FindWindow(ID_LABEL_HEIGHT)->Enable(m_bLabels);
	FindWindow(ID_LABEL_SIZE)->Enable(m_bLabels);
}

void TParamsDlg::RefreshLabelFields()
{
	m_pLabelField->Clear();
	vtFeatures feat;

	vtString fname = "PointData/";
	fname += m_strLabelFile.mb_str();
	vtString fpath = FindFileOnPaths(m_datapaths, fname);
	if (!feat.LoadHeaderFromSHP(fpath))
		return;

	int i, num = feat.GetNumFields();
	for (i = 0; i < num; i++)
	{
		Field *field = feat.GetField(i);
		wxString2 field_name = field->m_name;
		m_pLabelField->Append(field_name);
	}
	if (num)
	{
		if (m_Style.m_field_index < 0)
			m_Style.m_field_index = 0;
		if (m_Style.m_field_index > num-1)
			m_Style.m_field_index = num-1;
	}
}

void TParamsDlg::RefreshLocationFields()
{
	m_pLocField->Clear();
	m_pLocField->Append(_T("(default)"));

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

// WDR: handler implementations for TParamsDlg

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
	m_bReady = false;
	m_bSetting = true;

	m_pPreLightFactor = GetLightFactor();
	m_pStructFiles = GetStructFiles();
	m_pRoadFile = GetRoadfile();
	m_pTreeFile = GetTreefile();
	m_pTextureFileSingle = GetTfilesingle();
	m_pLodMethod = GetLodmethod();
	m_pFilename = GetFilename();
	m_pFilenameTin = GetFilenameTin();
	m_pLocFile = GetLocfile();
	m_pSkyTexture = GetSkytexture();
	m_pLabelFile = GetLabelFile();
	m_pLabelField = GetLabelField();
	m_pLocField = GetLocField();
	m_pNavStyle = GetNavStyle();

	m_pNone = GetNone();
	m_pSingle = GetSingle();
	m_pDerived = GetDerived();
	m_pTiled = GetTiled();

	unsigned int i;
	int sel;

	vtStringArray &paths = m_datapaths;

	for (i = 0; i < paths.size(); i++)
	{
		// fill the "Grid filename" control with available files
		AddFilenamesToComboBox(m_pFilename, paths[i] + "Elevation", "*.bt*");
		sel = m_pFilename->FindString(m_strFilename);
		if (sel != -1)
			m_pFilename->SetSelection(sel);

		// fill the "TIN filename" control with available files
		AddFilenamesToComboBox(m_pFilenameTin, paths[i] + "Elevation", "*.tin");
		sel = m_pFilenameTin->FindString(m_strFilenameTin);
		if (sel != -1)
			m_pFilename->SetSelection(sel);

		// fill the "single texture filename" control with available bitmap files
		AddFilenamesToComboBox(m_pTextureFileSingle, paths[i] + "GeoSpecific", "*.bmp");
		AddFilenamesToComboBox(m_pTextureFileSingle, paths[i] + "GeoSpecific", "*.jpg");
		AddFilenamesToComboBox(m_pTextureFileSingle, paths[i] + "GeoSpecific", "*.jpeg");
		sel = m_pTextureFileSingle->FindString(m_strTextureSingle);
		if (sel != -1)
			m_pTextureFileSingle->SetSelection(sel);

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

		// fill in Sky files
		AddFilenamesToComboBox(m_pSkyTexture, paths[i] + "Sky", "*.bmp");
		AddFilenamesToComboBox(m_pSkyTexture, paths[i] + "Sky", "*.png");
		AddFilenamesToComboBox(m_pSkyTexture, paths[i] + "Sky", "*.jpg");
		sel = m_pSkyTexture->FindString(m_strSkyTexture);
		if (sel != -1)
			m_pSkyTexture->SetSelection(sel);

		// fill in PointData files
		AddFilenamesToComboBox(m_pLabelFile, paths[i] + "PointData", "*.shp");
		sel = m_pLabelFile->FindString(m_strLabelFile);
		if (sel != -1)
			m_pLabelFile->SetSelection(sel);
	}

	m_pLodMethod->Clear();
	m_pLodMethod->Append(_T("Roettger"));
	m_pLodMethod->Append(_T("TopoVista"));
	m_pLodMethod->Append(_T("McNally"));
	m_pLodMethod->Append(_T("Demeter"));
	m_pLodMethod->Append(_T("Custom"));
	m_pLodMethod->Append(_T("BryanQuad"));
	// add your own LOD method here!

	m_pLodMethod->SetSelection(m_iLodMethod);

	m_pNavStyle->Clear();
	m_pNavStyle->Append(_T("Normal Terrain Flyer"));
	m_pNavStyle->Append(_T("Terrain Flyer with Velocity"));
	m_pNavStyle->Append(_T("Grab-Pivot"));
	m_pNavStyle->Append(_T("Quake-Style Walk"));
	m_pNavStyle->Append(_T("Panoramic Flyer"));

	RefreshLabelFields();
	RefreshLocationFields();

//  DetermineTerrainSizeFromBT();
//  DetermineSizeFromBMP();

//  OnChangeMem();
	UpdateTiledTextureFilename();

	GetUseGrid()->SetValue(!m_bTin);
	GetUseTin()->SetValue(m_bTin);

	// overall name
	AddValidator(ID_TNAME, &m_strTerrainName);

	// elevation
	AddValidator(ID_FILENAME, &m_strFilename);
	AddValidator(ID_FILENAME_TIN, &m_strFilenameTin);
	AddNumValidator(ID_VERTEXAG, &m_fVerticalExag, 2);
	AddValidator(ID_USE_TIN, &m_bTin);

	// nav
	AddNumValidator(ID_MINHEIGHT, &m_iMinHeight);
	AddValidator(ID_NAV_STYLE, &m_iNavStyle);
	AddNumValidator(ID_NAVSPEED, &m_fNavSpeed, 2);
	AddValidator(ID_LOCFILE, &m_strLocFile);
	AddValidator(ID_INIT_LOCATION, &m_iInitLocation);
	AddNumValidator(ID_HITHER, &m_fHither);
	AddValidator(ID_ACCEL, &m_bAccel);

	// LOD
	AddValidator(ID_LODMETHOD, &m_iLodMethod);
	AddNumValidator(ID_PIXELERROR, &m_fPixelError, 2);
	AddNumValidator(ID_TRICOUNT, &m_iTriCount);
	AddValidator(ID_TRISTRIPS, &m_bTriStrips);
	AddValidator(ID_DETAILTEXTURE, &m_bDetailTexture);

	// time
	AddValidator(ID_TIMEMOVES, &m_bTimeOn);
	AddNumValidator(ID_INITTIME, &m_iInitTime);
	AddNumValidator(ID_TIMESPEED, &m_fTimeSpeed, 2);

	// texture
	AddValidator(ID_TFILESINGLE, &m_strTextureSingle);
	AddNumValidator(ID_TILESIZE, &m_iTilesize);
	AddValidator(ID_TFILEBASE, &m_strTextureBase);
	AddValidator(ID_JPEG, &m_bJPEG);
	AddValidator(ID_TFILENAME, &m_strTextureFilename);
	AddValidator(ID_MIPMAP, &m_bMipmap);
	AddValidator(ID_16BIT, &m_b16bit);
	AddValidator(ID_PRELIGHT, &m_bPreLight);
	AddValidator(ID_PRELIT, &m_bPreLit);
	AddNumValidator(ID_LIGHT_FACTOR, &m_fPreLightFactor, 2);

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

	AddNumValidator(ID_STRUCT_DISTANCE, &m_iStructDistance);
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

	// Feature labels page
	AddValidator(ID_LABELS, &m_bLabels);
	AddValidator(ID_LABEL_FILE, &m_strLabelFile);
	AddValidator(ID_LABEL_FIELD, &m_Style.m_field_index);
	AddNumValidator(ID_LABEL_HEIGHT, &m_Style.m_label_elevation);
	AddNumValidator(ID_LABEL_SIZE, &m_Style.m_label_size);

	m_iInitLocation = m_pLocField->FindString(m_strInitLocation);
	if (m_iInitLocation == -1)
		m_iInitLocation = 0;

	wxWindow::OnInitDialog(event);

	UpdateEnableState();

	m_bReady = true;
	m_bSetting = false;

	// It's somewhat roundabout, but this lets us capture events on the
	// listbox control without having to subclass it.
	m_pBoxHandler = new wxListBoxEventHandler(this, m_pStructFiles);
	m_pStructFiles->PushEventHandler(m_pBoxHandler);
}

bool TParamsDlg::TransferDataToWindow()
{
	m_bSetting = true;

	m_pNone->SetValue(m_iTexture == TE_NONE);
	m_pSingle->SetValue(m_iTexture == TE_SINGLE);
	m_pDerived->SetValue(m_iTexture == TE_DERIVED);
	m_pTiled->SetValue(m_iTexture == TE_TILED);

	m_pStructFiles->Clear();
	int i, num = m_strStructFiles.GetSize();
	for (i = 0; i < num; i++)
		m_pStructFiles->Append(*m_strStructFiles[i]);
	m_pStructFiles->Append(_T("(double-click to add files)"));

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

	m_strStructFiles.Empty();
	int i, num = m_pStructFiles->GetCount();
	for (i = 0; i < num-1; i++)		// skip last
		m_strStructFiles.Append(new wxString2(m_pStructFiles->GetString(i)));

	return wxDialog::TransferDataFromWindow();
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
		m_bPreLit = false;
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
		m_bPreLit = true;
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

void TParamsDlg::OnCheckBox( wxCommandEvent &event )
{
	TransferDataFromWindow();
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
		//		VTLOG("   entry: '%s'", name1.c_str());
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

void TParamsDlg::OnListDblClick( wxCommandEvent &event )
{
	unsigned int i;
	wxArrayString strings;

	for (i = 0; i < m_datapaths.size(); i++)
		AddFilenamesToArray(strings, m_datapaths[i] + "BuildingData", "*.vtst");

	int num = strings.Count();

	// int num = m_pLocList->GetSelection();  // no care
	wxString result = wxGetSingleChoice(_T("one of the following to add:"), _T("Choose a structure file"),
		strings, this);

	if (result.Cmp(_T("")))	// user selected something
	{
		m_strStructFiles.Append(new wxString2(result));
		TransferDataToWindow();
	}
}

void TParamsDlg::OnChoiceLabelFile( wxCommandEvent &event )
{
	if (m_bSetting || !m_bReady) return;

	wxString2 prev = m_strLabelFile;
	TransferDataFromWindow();
	if (m_strLabelFile != prev)
	{
		RefreshLabelFields();
		m_bSetting = true;
		TransferDataToWindow();
		m_bSetting = false;
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

