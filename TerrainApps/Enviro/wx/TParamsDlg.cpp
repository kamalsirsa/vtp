//
// Name: TParamsDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
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
#include "TParamsDlg.h"
#include "../Options.h"

#define NTILES 4

extern void AddFilenamesToComboBox(wxComboBox *box, const char *directory,
							const char *wildcard, int omit_chars = 0);

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

	EVT_CHECKBOX( ID_REGULAR, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_DYNAMIC, TParamsDlg::OnCheckBox )

	EVT_CHECKBOX( ID_BUILDINGS, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_TREES, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_ROADS, TParamsDlg::OnCheckBox )

	EVT_CHECKBOX( ID_BUILDINGS, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_OCEANPLANE, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_DEPRESSOCEAN, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_SKY, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_FOG, TParamsDlg::OnCheckBox )
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
}


//
// set the values in the dialog from the supplied paramter structure
//
void TParamsDlg::SetParams(TParams &Params)
{
	m_strFilename = Params.m_strElevFile;
	m_strFilenameTin = Params.m_strElevFile;
	m_fVerticalExag = Params.m_fVerticalExag;
	m_iMinHeight = Params.m_iMinHeight;
	m_fNavSpeed = Params.m_fNavSpeed;
	m_strLocFile = Params.m_strLocFile;

	m_bRegular = Params.m_bRegular;
	m_iSubsample = Params.m_iSubsample;

	m_bDynamic = Params.m_bDynamic;
	m_iLodMethod = Params.m_eLodMethod;
	m_fPixelError = Params.m_fPixelError;
	m_iTriCount = Params.m_iTriCount;
	m_bTriStrips = Params.m_bTriStrips;
	m_bDetailTexture = Params.m_bDetailTexture;

	m_bTin = Params.m_bTin;

	m_bTimeOn = Params.m_bTimeOn;
	m_iInitTime = Params.m_iInitTime;
	m_fTimeSpeed = Params.m_fTimeSpeed;

	m_iTexture = Params.m_eTexture;
	m_iTilesize = Params.m_iTilesize;
	m_strTextureSingle = Params.m_strTextureSingle;
	m_strTextureBase = Params.m_strTextureBase;
	m_bJPEG = Params.m_bJPEG;
	m_strTextureFilename = Params.m_strTextureFilename;
	m_bMipmap = Params.m_bMipmap;
	m_b16bit = Params.m_b16bit;

	m_bRoads = Params.m_bRoads;
	m_strRoadFile = Params.m_strRoadFile;
	m_bHwy = Params.m_bHwy;
	m_bDirt = Params.m_bDirt;
	m_bPaved = Params.m_bPaved;
	m_fRoadHeight = Params.m_fRoadHeight;
	m_fRoadDistance = Params.m_fRoadDistance;
	m_bTexRoads = Params.m_bTexRoads;
	m_bRoadCulture = Params.m_bRoadCulture;

	m_bTrees = Params.m_bTrees;
	m_strTreeFile = Params.m_strTreeFile;
	m_iTreeDistance = Params.m_iTreeDistance;

	m_bFog = Params.m_bFog;
	m_iFogDistance = Params.m_iFogDistance;

	m_bBuildings = Params.m_bBuildings;
	m_strBuildingFile = Params.m_strBuildingFile;

//  m_bVehicles = Params.m_bVehicles;
//  m_fVehicleSize = Params.m_fVehicleSize;
//  m_fVehicleSpeed = Params.m_fVehicleSpeed;
//  m_iNumCars = Params.m_iNumCars;

	m_bSky = Params.m_bSky;
	m_strSkyTexture = Params.m_strSkyTexture;
	m_bHorizon = Params.m_bHorizon;
	m_bVertexColors = Params.m_bVertexColors;
//  m_bOverlay = Params.m_bOverlay;
	m_bLabels = Params.m_bLabels;

	m_bOceanPlane = Params.m_bOceanPlane;
	m_fOceanPlaneLevel = Params.m_fOceanPlaneLevel;
	m_bDepressOcean = Params.m_bDepressOcean;
	m_fDepressOceanLevel = Params.m_fDepressOceanLevel;

	m_bPreLight = Params.m_bPreLight;
	m_bPreLit = Params.m_bPreLit;
	m_fPreLightFactor = Params.m_fPreLightFactor;

	m_bAirports = Params.m_bAirports;

	m_strRouteFile = Params.m_strRouteFile;
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
	if (m_bTin)
		Params.m_strElevFile = m_strFilenameTin;
	else
		Params.m_strElevFile = m_strFilename;

	// LocationsFilename
	Params.m_fVerticalExag = m_fVerticalExag;
	Params.m_iMinHeight = m_iMinHeight;
	Params.m_fNavSpeed = m_fNavSpeed;
	Params.m_strLocFile = m_strLocFile;

	Params.m_bRegular = m_bRegular;
	Params.m_iSubsample = m_iSubsample;

	Params.m_bDynamic = m_bDynamic;
	Params.m_eLodMethod = (enum LodMethodEnum) m_iLodMethod;
	Params.m_fPixelError = m_fPixelError;
	Params.m_iTriCount = m_iTriCount;
	Params.m_bTriStrips = m_bTriStrips;
	Params.m_bDetailTexture = m_bDetailTexture;

	Params.m_bTin = m_bTin;

	Params.m_bTimeOn = m_bTimeOn;
	Params.m_iInitTime = m_iInitTime;
	Params.m_fTimeSpeed = m_fTimeSpeed;

	Params.m_eTexture = (enum TextureEnum)m_iTexture;
	Params.m_iTilesize = m_iTilesize;
	Params.m_strTextureSingle = m_strTextureSingle;
	Params.m_strTextureBase = m_strTextureBase;
	Params.m_bJPEG = m_bJPEG;
	Params.m_strTextureFilename = m_strTextureFilename;
	Params.m_bMipmap = m_bMipmap;
	Params.m_b16bit = m_b16bit;

	Params.m_bRoads = m_bRoads;
	Params.m_strRoadFile = m_strRoadFile;
	Params.m_bHwy = m_bHwy;
	Params.m_bPaved = m_bPaved;
	Params.m_bDirt = m_bDirt;
	Params.m_fRoadHeight = m_fRoadHeight;
	Params.m_fRoadDistance = m_fRoadDistance;
	Params.m_bTexRoads = m_bTexRoads;
	Params.m_bRoadCulture = m_bRoadCulture;

	Params.m_bTrees = m_bTrees;
	Params.m_strTreeFile = m_strTreeFile;
	Params.m_iTreeDistance = m_iTreeDistance;

	Params.m_bFog = m_bFog;
	Params.m_iFogDistance = m_iFogDistance;

	Params.m_bBuildings = m_bBuildings;
	Params.m_strBuildingFile = m_strBuildingFile;

//  Params.m_bVehicles = m_bVehicles;
//  Params.m_fVehicleSize = m_fVehicleSize;
//  Params.m_fVehicleSpeed = m_fVehicleSpeed;
//  Params.m_iNumCars = m_iNumCars;

	Params.m_bSky = m_bSky;
	Params.m_strSkyTexture = m_strSkyTexture;
	Params.m_bOceanPlane = m_bOceanPlane;
	Params.m_bHorizon = m_bHorizon;
	Params.m_bVertexColors = m_bVertexColors;
//  Params.m_bOverlay = m_bOverlay;
	Params.m_bLabels = m_bLabels;

	Params.m_bOceanPlane = m_bOceanPlane;
	Params.m_fOceanPlaneLevel = m_fOceanPlaneLevel;
	Params.m_bDepressOcean = m_bDepressOcean;
	Params.m_fDepressOceanLevel = m_fDepressOceanLevel;

	Params.m_bPreLight = m_bPreLight;
	Params.m_bPreLit = m_bPreLit;
	Params.m_fPreLightFactor = m_fPreLightFactor;

	Params.m_bAirports = m_bAirports;

	Params.m_strRouteFile = m_strRouteFile;
	Params.m_bRouteEnable = m_bRouteEnable;
}

void TParamsDlg::UpdateTiledTextureFilename()
{
	m_strTextureFilename.Printf("%s%d", (const char *)m_strTextureBase,
		NTILES * (m_iTilesize-1) + 1);
	if (m_bJPEG)
		m_strTextureFilename += ".jpg";
	else
		m_strTextureFilename += ".bmp";
	TransferDataToWindow();
}

void TParamsDlg::UpdateEnableState()
{
	GetFilename()->Enable(!m_bTin);
	GetFilenameTin()->Enable(m_bTin);

	FindWindow(ID_SUBSAMPLE)->Enable(m_bRegular);
	FindWindow(ID_VERTEXCOLORS)->Enable(m_bRegular);

	FindWindow(ID_LODMETHOD)->Enable(m_bDynamic);
	FindWindow(ID_PIXELERROR)->Enable(m_bDynamic);
	FindWindow(ID_TRICOUNT)->Enable(m_bDynamic);
	FindWindow(ID_TRISTRIPS)->Enable(m_bDynamic);
	FindWindow(ID_DETAILTEXTURE)->Enable(m_bDynamic);

	FindWindow(ID_TILESIZE)->Enable(m_iTexture == TE_TILED);
	FindWindow(ID_TFILEBASE)->Enable(m_iTexture == TE_TILED);
	FindWindow(ID_JPEG)->Enable(m_iTexture == TE_TILED);
	FindWindow(ID_TFILENAME)->Enable(m_iTexture == TE_TILED);

	FindWindow(ID_MIPMAP)->Enable(m_iTexture != TE_NONE);
	FindWindow(ID_16BIT)->Enable(m_iTexture != TE_NONE);
	FindWindow(ID_PRELIGHT)->Enable(m_iTexture != TE_NONE);
	FindWindow(ID_LIGHT_FACTOR)->Enable(m_iTexture != TE_NONE);
	FindWindow(ID_PRELIT)->Enable(m_iTexture != TE_NONE);

	FindWindow(ID_TREEFILE)->Enable(m_bTrees);
	FindWindow(ID_TREEDISTANCE)->Enable(m_bTrees);

	FindWindow(ID_ROADFILE)->Enable(m_bRoads);
	FindWindow(ID_ROADHEIGHT)->Enable(m_bRoads);
	FindWindow(ID_ROADDISTANCE)->Enable(m_bRoads);
	FindWindow(ID_TEXROADS)->Enable(m_bRoads);
	FindWindow(ID_ROADCULTURE)->Enable(m_bRoads);
	FindWindow(ID_HIGHWAYS)->Enable(m_bRoads);
	FindWindow(ID_PAVED)->Enable(m_bRoads);
	FindWindow(ID_DIRT)->Enable(m_bRoads);

	GetBuildingfile()->Enable(m_bBuildings);
	GetOceanPlaneOffset()->Enable(m_bOceanPlane);
	GetDepressOceanOffset()->Enable(m_bDepressOcean);
	GetSkytexture()->Enable(m_bSky);
	GetSkytexture()->Enable(m_bSky);
	GetFogDistance()->Enable(m_bFog);
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
	m_pBuildingFile = GetBuildingfile();
	m_pRoadFile = GetRoadfile();
	m_pTreeFile = GetTreefile();
	m_pTextureFileSingle = GetTfilesingle();
	m_pLodMethod = GetLodmethod();
	m_pFilename = GetFilename();
	m_pFilenameTin = GetFilenameTin();
	m_pLocFile = GetLocfile();
	m_pSkyTexture = GetSkytexture();

	m_pNone = GetNone();
	m_pSingle = GetSingle();
	m_pDerived = GetDerived();
	m_pTiled = GetTiled();

	int i, sel;

	StringArray &paths = g_Options.m_DataPaths;

	for (i = 0; i < paths.GetSize(); i++)
	{
		// fill the "Grid filename" control with available files
		AddFilenamesToComboBox(m_pFilename, *paths[i] + "Elevation", "*.bt");
		sel = m_pFilename->FindString(m_strFilename);
		if (sel != -1)
			m_pFilename->SetSelection(sel);

		// fill the "TIN filename" control with available files
		AddFilenamesToComboBox(m_pFilenameTin, *paths[i] + "Elevation", "*.tin");
		sel = m_pFilenameTin->FindString(m_strFilenameTin);
		if (sel != -1)
			m_pFilename->SetSelection(sel);

		// fill the "single texture filename" control with available bitmap files
		AddFilenamesToComboBox(m_pTextureFileSingle, *paths[i] + "GeoSpecific", "*.bmp");
		AddFilenamesToComboBox(m_pTextureFileSingle, *paths[i] + "GeoSpecific", "*.jpg");
		AddFilenamesToComboBox(m_pTextureFileSingle, *paths[i] + "GeoSpecific", "*.jpeg");
		sel = m_pTextureFileSingle->FindString(m_strTextureSingle);
		if (sel != -1)
			m_pTextureFileSingle->SetSelection(sel);

		// fill the Location files
		AddFilenamesToComboBox(m_pLocFile, *paths[i] + "Locations", "*.loc");
		sel = m_pLocFile->FindString(m_strLocFile);
		if (sel != -1)
			m_pLocFile->SetSelection(sel);

		// fill in Road files
		AddFilenamesToComboBox(m_pRoadFile, *paths[i] + "RoadData", "*.rmf");
		sel = m_pRoadFile->FindString(m_strRoadFile);
		if (sel != -1)
			m_pRoadFile->SetSelection(sel);

		// fill in Building files
		AddFilenamesToComboBox(m_pBuildingFile, *paths[i] + "BuildingData", "*.vtst");
		sel = m_pBuildingFile->FindString(m_strBuildingFile);
		if (sel != -1)
			m_pBuildingFile->SetSelection(sel);

		// fill in Vegetation files
		AddFilenamesToComboBox(m_pTreeFile, *paths[i] + "PlantData", "*.vf");
		sel = m_pTreeFile->FindString(m_strTreeFile);
		if (sel != -1)
			m_pTreeFile->SetSelection(sel);

		// fill in Sky files
		AddFilenamesToComboBox(m_pSkyTexture, *paths[i] + "Sky", "*.bmp");
		AddFilenamesToComboBox(m_pSkyTexture, *paths[i] + "Sky", "*.png");
		sel = m_pSkyTexture->FindString(m_strSkyTexture);
		if (sel != -1)
			m_pSkyTexture->SetSelection(sel);
	}

	m_pLodMethod->Clear();
	m_pLodMethod->Append("Roettger");
//	m_pLodMethod->Append("Lindstrom-Koller");
	m_pLodMethod->Append("TopoVista");
	m_pLodMethod->Append("McNally");
	m_pLodMethod->Append("Demeter");
	m_pLodMethod->Append("Custom");
	m_pLodMethod->Append("BryanQuad");
	// add your own LOD method here!

	m_pLodMethod->SetSelection(m_iLodMethod);

//  DetermineTerrainSizeFromBT();
//  DetermineSizeFromBMP();

//  OnChangeMem();
	UpdateTiledTextureFilename();

	GetUseGrid()->SetValue(!m_bTin);
	GetUseTin()->SetValue(m_bTin);

	AddValidator(ID_FILENAME, &m_strFilename);
	AddValidator(ID_FILENAME_TIN, &m_strFilenameTin);
	AddNumValidator(ID_VERTEXAG, &m_fVerticalExag);

	AddValidator(ID_USE_TIN, &m_bTin);

	AddValidator(ID_TIMEMOVES, &m_bTimeOn);
	AddNumValidator(ID_INITTIME, &m_iInitTime);
	AddNumValidator(ID_TIMESPEED, &m_fTimeSpeed);

	AddValidator(ID_REGULAR, &m_bRegular);
	AddNumValidator(ID_SUBSAMPLE, &m_iSubsample);
	AddValidator(ID_DYNAMIC, &m_bDynamic);
	AddValidator(ID_LODMETHOD, &m_iLodMethod);
	AddNumValidator(ID_PIXELERROR, &m_fPixelError);
	AddNumValidator(ID_TRICOUNT, &m_iTriCount);
	AddValidator(ID_SKY, &m_bSky);
	AddValidator(ID_SKYTEXTURE, &m_strSkyTexture);
	AddValidator(ID_FOG, &m_bFog);
	AddNumValidator(ID_FOG_DISTANCE, &m_iFogDistance);

	// texture
	AddValidator(ID_TFILESINGLE, &m_strTextureSingle);
	AddNumValidator(ID_TILESIZE, &m_iTilesize);
	AddValidator(ID_TFILEBASE, &m_strTextureBase);
	AddValidator(ID_JPEG, &m_bJPEG);
	AddValidator(ID_TFILENAME, &m_strTextureFilename);
	AddValidator(ID_MIPMAP, &m_bMipmap);
	AddValidator(ID_16BIT, &m_b16bit);
	AddValidator(ID_PRELIGHT, &m_bPreLight);
	AddNumValidator(ID_LIGHT_FACTOR, &m_fPreLightFactor);
	AddValidator(ID_PRELIT, &m_bPreLit);

	AddValidator(ID_ROADS, &m_bRoads);
	AddValidator(ID_ROADFILE, &m_strRoadFile);
	AddValidator(ID_TEXROADS, &m_bTexRoads);
	AddValidator(ID_TREES, &m_bTrees);
	AddValidator(ID_TREEFILE, &m_strTreeFile);
	AddNumValidator(ID_TREEDISTANCE, &m_iTreeDistance);
	AddValidator(ID_VERTEXCOLORS, &m_bVertexColors);
	AddValidator(ID_HORIZON, &m_bHorizon);
	AddValidator(ID_LABELS, &m_bLabels);
	AddNumValidator(ID_MINHEIGHT, &m_iMinHeight);
	AddValidator(ID_BUILDINGS, &m_bBuildings);
	AddValidator(ID_BUILDINGFILE, &m_strBuildingFile);
	AddValidator(ID_TRISTRIPS, &m_bTriStrips);
	AddValidator(ID_DETAILTEXTURE, &m_bDetailTexture);
	AddValidator(ID_DIRT, &m_bDirt);
	AddValidator(ID_PAVED, &m_bPaved);
	AddValidator(ID_HIGHWAYS, &m_bHwy);
	AddNumValidator(ID_ROADDISTANCE, &m_fRoadDistance);
	AddNumValidator(ID_ROADHEIGHT, &m_fRoadHeight);
	AddNumValidator(ID_NAVSPEED, &m_fNavSpeed);
	AddValidator(ID_LOCFILE, &m_strLocFile);
	AddValidator(ID_ROADCULTURE, &m_bRoadCulture);
	AddValidator(ID_AIRPORTS, &m_bAirports);
	AddValidator(ID_OCEANPLANE, &m_bOceanPlane);
	AddNumValidator(ID_OCEANPLANEOFFSET, &m_fOceanPlaneLevel);
	AddValidator(ID_DEPRESSOCEAN, &m_bDepressOcean);
	AddNumValidator(ID_DEPRESSOCEANOFFSET, &m_fDepressOceanLevel);

	wxWindow::OnInitDialog(event);

	UpdateEnableState();

	m_bReady = true;
	m_bSetting = false;
}

bool TParamsDlg::TransferDataToWindow()
{
	m_bSetting = true;

	m_pNone->SetValue(m_iTexture == TE_NONE);
	m_pSingle->SetValue(m_iTexture == TE_SINGLE);
	m_pDerived->SetValue(m_iTexture == TE_DERIVED);
	m_pTiled->SetValue(m_iTexture == TE_TILED);

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

