//
// Name: TParamsDlg.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
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
#include "TimeDlg.h"

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

	EVT_BUTTON( ID_SET_INIT_TIME, TParamsDlg::OnSetInitTime )
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
// Set the values in the dialog from the supplied paramter structure.
// Note that TParams uses UTF8 for all its strings, so we need to use
//  from_utf8() when copying to wxString.
//
void TParamsDlg::SetParams(const TParams &Params)
{
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// overall name
	m_strTerrainName.from_utf8(Params.GetValueString(STR_NAME));

	// elevation
	m_strFilename.from_utf8(Params.GetValueString(STR_ELEVFILE));
	m_strFilenameTin.from_utf8(Params.GetValueString(STR_ELEVFILE));
	m_fVerticalExag =	Params.GetValueFloat(STR_VERTICALEXAG);
	m_bTin =			Params.GetValueBool(STR_TIN);

	/// navigation
	m_iMinHeight =		Params.GetValueInt(STR_MINHEIGHT);
	m_iNavStyle =		Params.GetValueInt(STR_NAVSTYLE);
	m_fNavSpeed =		Params.GetValueFloat(STR_NAVSPEED);
	m_strLocFile.from_utf8(Params.GetValueString(STR_LOCFILE));
	m_strInitLocation.from_utf8(Params.GetValueString(STR_INITLOCATION));
	m_fHither =			Params.GetValueFloat(STR_HITHER);
	m_bAccel =			Params.GetValueBool(STR_ACCEL);

	// LOD
	m_iLodMethod =		Params.GetLodMethod();
	m_fPixelError =		Params.GetValueFloat(STR_PIXELERROR);
	m_iTriCount =		Params.GetValueInt(STR_TRICOUNT);
	m_bTriStrips =		Params.GetValueBool(STR_TRISTRIPS);
	m_bDetailTexture =	Params.GetValueBool(STR_DETAILTEXTURE);

	// time
	m_bTimeOn =			Params.GetValueBool(STR_TIMEON);
	m_InitTime.SetFromString(Params.GetValueString(STR_INITTIME));
	m_fTimeSpeed =		Params.GetValueFloat(STR_TIMESPEED);

	// texture
	m_iTexture =		Params.GetTextureEnum();
	m_iTilesize =		Params.GetValueInt(STR_TILESIZE);
	m_strTextureSingle.from_utf8(Params.GetValueString(STR_TEXTURESINGLE));
	m_strTextureBase.from_utf8(Params.GetValueString(STR_TEXTUREBASE));
	m_bJPEG =			(Params.GetValueBool(STR_TEXTUREFORMAT) == 1);
	m_strTextureFilename.from_utf8(Params.CookTextureFilename());
	m_bMipmap =			Params.GetValueBool(STR_MIPMAP);
	m_b16bit =			Params.GetValueBool(STR_REQUEST16BIT);
	m_bPreLight =		Params.GetValueBool(STR_PRELIGHT);
	m_fPreLightFactor = Params.GetValueFloat(STR_PRELIGHTFACTOR);
	m_bCastShadows =	Params.GetValueBool(STR_CAST_SHADOWS);

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

	unsigned int i, num = Params.m_strStructFiles.size();
	for (i = 0; i < num; i++)
	{
		wxString2 *ws = new wxString2();
		ws->from_utf8(Params.m_strStructFiles[i]);
		m_strStructFiles.Append(ws);
	}
	m_iStructDistance = Params.GetValueInt(STR_STRUCTDIST);

	m_bVehicles =		Params.GetValueBool(STR_VEHICLES);
//  m_fVehicleSize =	Params.GetValueFloat(STR_VEHICLESIZE);
//  m_fVehicleSpeed =	Params.GetValueFloat(STR_VEHICLESPEED);
//  m_iNumCars =		Params.GetValueInt(STR_NUMCARS);

	m_bSky =			Params.GetValueBool(STR_SKY);
	m_strSkyTexture.from_utf8(Params.GetValueString(STR_SKYTEXTURE));
	m_bOceanPlane =		Params.GetValueBool(STR_OCEANPLANE);
	m_fOceanPlaneLevel = Params.GetValueFloat(STR_OCEANPLANELEVEL);
	m_bDepressOcean =	Params.GetValueBool(STR_DEPRESSOCEAN);
	m_fDepressOceanLevel = Params.GetValueFloat(STR_DEPRESSOCEANLEVEL);
	m_bHorizon =		Params.GetValueBool(STR_HORIZON);
//  m_bOverlay =		Params.GetValueBool(STR_OVERLAY);
	m_bLabels =			Params.GetValueBool(STR_LABELS);
	m_strLabelFile.from_utf8(Params.GetValueString(STR_LABELFILE));
	m_Style =			Params.GetPointStyle();

	m_bRouteEnable =	Params.GetValueBool(STR_ROUTEENABLE);
	m_strRouteFile.from_utf8(Params.GetValueString(STR_ROUTEFILE));

	// Safety check
	if (m_iTriCount < 500 || m_iTriCount > 100000)
		m_iTriCount = 10000;
}

//
// get the values from the dialog into the supplied paramter structure
//
void TParamsDlg::GetParams(TParams &Params)
{
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// overall name
	Params.SetValueString(STR_NAME, m_strTerrainName.to_utf8());

	// elevation
	if (m_bTin)
		Params.SetValueString(STR_ELEVFILE, m_strFilenameTin.to_utf8());
	else
		Params.SetValueString(STR_ELEVFILE, m_strFilename.to_utf8());
	Params.SetValueFloat(STR_VERTICALEXAG, m_fVerticalExag);
	Params.SetValueBool(STR_TIN, m_bTin);

	// navigation
	Params.SetValueInt(STR_MINHEIGHT, m_iMinHeight);
	Params.SetValueInt(STR_NAVSTYLE, m_iNavStyle);
	Params.SetValueFloat(STR_NAVSPEED, m_fNavSpeed);
	Params.SetValueString(STR_LOCFILE, m_strLocFile.to_utf8());
	Params.SetValueString(STR_INITLOCATION, m_strInitLocation.to_utf8());
	Params.SetValueFloat(STR_HITHER, m_fHither);
	Params.SetValueBool(STR_ACCEL, m_bAccel);

	// LOD
	Params.SetLodMethod((enum LodMethodEnum) m_iLodMethod);
	Params.SetValueFloat(STR_PIXELERROR, m_fPixelError);
	Params.SetValueInt(STR_TRICOUNT, m_iTriCount);
	Params.SetValueBool(STR_TRISTRIPS, m_bTriStrips);
	Params.SetValueBool(STR_DETAILTEXTURE, m_bDetailTexture);

	// time
	Params.SetValueBool(STR_TIMEON, m_bTimeOn);
	Params.SetValueString(STR_INITTIME, m_InitTime.GetAsString());
	Params.SetValueFloat(STR_TIMESPEED, m_fTimeSpeed);

	// texture
	Params.SetTextureEnum((enum TextureEnum)m_iTexture);
	Params.SetValueInt(STR_TILESIZE, m_iTilesize);
	Params.SetValueString(STR_TEXTURESINGLE, m_strTextureSingle.to_utf8());
	Params.SetValueString(STR_TEXTUREBASE, m_strTextureBase.to_utf8());
	Params.SetValueInt(STR_TEXTUREFORMAT, (int) m_bJPEG);
//	Params.SetValueString(TextureFilename, m_strTextureFilename.to_utf8());
	Params.SetValueBool(STR_MIPMAP, m_bMipmap);
	Params.SetValueBool(STR_REQUEST16BIT, m_b16bit);
	Params.SetValueBool(STR_PRELIGHT, m_bPreLight);
	Params.SetValueFloat(STR_PRELIGHTFACTOR, m_fPreLightFactor);
	Params.SetValueBool(STR_CAST_SHADOWS, m_bCastShadows);

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

	Params.m_strStructFiles.clear();
	int i, num = m_strStructFiles.GetSize();
	for (i = 0; i < num; i++)
		Params.m_strStructFiles.push_back(vtString(m_strStructFiles[i]->mb_str()));
	Params.SetValueInt(STR_STRUCTDIST, m_iStructDistance);

	Params.SetValueBool(STR_VEHICLES, m_bVehicles);
//  Params.SetValueFloat(STR_VEHICLESIZE, m_fVehicleSize);
//  Params.SetValueFloat(STR_VEHICLESPEED, m_fVehicleSpeed);
//  Params.SetValueInt(STR_NUMCARS, m_iNumCars);

	Params.SetValueBool(STR_SKY, m_bSky);
	Params.SetValueString(STR_SKYTEXTURE, m_strSkyTexture.to_utf8());

	Params.SetValueBool(STR_OCEANPLANE, m_bOceanPlane);
	Params.SetValueFloat(STR_OCEANPLANELEVEL, m_fOceanPlaneLevel);
	Params.SetValueBool(STR_DEPRESSOCEAN, m_bDepressOcean);
	Params.SetValueFloat(STR_DEPRESSOCEANLEVEL, m_fDepressOceanLevel);
	Params.SetValueBool(STR_HORIZON, m_bHorizon);
//  Params.SetValueBool(STR_OVERLAY, m_bOverlay);

	Params.SetValueBool(STR_LABELS, m_bLabels);
	Params.SetValueString(STR_LABELFILE, m_strLabelFile.to_utf8());
	Params.SetPointStyle(m_Style);

	Params.SetValueBool(STR_ROUTEENABLE, m_bRouteEnable);
	Params.SetValueString(STR_ROUTEFILE, m_strRouteFile.to_utf8());
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
	FindWindow(ID_CAST_SHADOWS)->Enable(m_iTexture != TE_NONE);

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

	vtString fname = "PointData/";
	fname += m_strLabelFile.mb_str();
	vtString fpath = FindFileOnPaths(m_datapaths, fname);

	vtFeatureLoader loader;
	vtFeatureSet *feat = loader.LoadHeaderFromSHP(fpath);
	if (!feat)
		return;

	int i, num = feat->GetNumFields();
	for (i = 0; i < num; i++)
	{
		Field *field = feat->GetField(i);
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
	delete feat;
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
		AddFilenamesToComboBox(m_pFilenameTin, paths[i] + "Elevation", "*.itf");
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
	m_pNavStyle->Append(_("Normal Terrain Flyer"));
	m_pNavStyle->Append(_("Terrain Flyer with Velocity"));
	m_pNavStyle->Append(_("Grab-Pivot"));
	m_pNavStyle->Append(_("Quake-Style Walk"));
	m_pNavStyle->Append(_("Panoramic Flyer"));

	RefreshLabelFields();
	RefreshLocationFields();

//  DetermineTerrainSizeFromBT();
//  DetermineSizeFromBMP();

//  OnChangeMem();
	UpdateTiledTextureFilename();

	GetUseGrid()->SetValue(!m_bTin);
	GetUseTin()->SetValue(m_bTin);

	m_strInitTime = asctime(&m_InitTime.GetTM());

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
	AddValidator(ID_TEXT_INIT_TIME, &m_strInitTime);
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
	AddValidator(ID_CAST_SHADOWS, &m_bCastShadows);
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
	m_pStructFiles->Append(_("(double-click to add files)"));

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
		AddFilenamesToArray(strings, m_datapaths[i] + "BuildingData", "*.vtst*");

	int num = strings.Count();

	// int num = m_pLocList->GetSelection();  // no care
	wxString result = wxGetSingleChoice(_("One of the following to add:"), _("Choose a structure file"),
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

void TParamsDlg::OnSetInitTime( wxCommandEvent &event )
{
	TimeDlg dlg(this, -1, _("Set Initial Time"));
	dlg.AddOkCancel();
	dlg.SetTime(m_InitTime);
	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetTime(m_InitTime);
		m_strInitTime = asctime(&m_InitTime.GetTM());
		m_bSetting = true;
		TransferDataToWindow();
		m_bSetting = false;
	}
}

