//
// Name:		TParamsDlg.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __TParamsDlg_H__
#define __TParamsDlg_H__

#ifdef __GNUG__
	#pragma interface "TParamsDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "AutoDialog.h"
#include "vtlib/core/TParams.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// TParamsDlg
//----------------------------------------------------------------------------

class TParamsDlg: public AutoDialog
{
public:
	// constructors and destructors
	TParamsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void OnInitDialog(wxInitDialogEvent& event);

	bool TransferDataToWindow();
	bool TransferDataFromWindow();

	void SetParams(TParams &Params);
	void GetParams(TParams &Params);
	void UpdateTiledTextureFilename();
	void UpdateEnableState();

	wxString	m_strFilename;
	wxString	m_strFilenameTin;
	float   m_fVerticalExag;
	bool	m_bTin;
	bool	m_bRegular;
	int		m_iSubsample;
	bool	m_bDynamic;
	float   m_fPixelError;
	int		m_iTriCount;
	bool	m_bTimeOn;
	int		m_iInitTime;
	float   m_fTimeSpeed;
	bool	m_bSky;
	wxString	m_strSkyTexture;
	bool	m_bFog;
	int		m_iTexture;
	int		m_iTilesize;
	wxString	m_strTextureSingle;
	wxString	m_strTextureBase;
	wxString	m_strTextureFilename;
	bool	m_bMipmap;
	bool	m_b16bit;
	bool	m_bRoads;
	wxString	m_strRoadFile;
	float   m_fWidthExag;
	bool	m_bTexRoads;
	bool	m_bTrees;
	wxString	m_strTreeFile;
	int		m_iTreeDistance;
	int		m_iFogDistance;
	bool	m_bVertexColors;
//  bool	m_bOverlay;

	bool	m_bOceanPlane;
	float	m_fOceanPlaneLevel;
	bool	m_bDepressOcean;
	float	m_fDepressOceanLevel;

	bool	m_bHorizon;
	bool	m_bLabels;
	int		m_iMinHeight;
	bool	m_bBuildings;
	wxString	m_strBuildingFile;
//  bool	m_bVehicles;
//  float   m_fVehicleSize;
	bool	m_bTriStrips;
//  wxString	m_strMemRequired;
	bool	m_bDetailTexture;
	bool	m_bPreLight;
	bool	m_bDirt;
	bool	m_bHwy;
	bool	m_bPaved;
	float   m_fRoadDistance;
	float   m_fRoadHeight;
	float   m_fNavSpeed;
	wxString	m_strLocFile;
	float   m_fPreLightFactor;
//  int		m_iNumCars;
	bool	m_bRoadCulture;
	int		m_iLodMethod;
//  float   m_fVehicleSpeed;
	bool	m_bPreLit;
	bool	m_bAirports;
	bool	m_bRouteEnable;
	wxString m_strRouteFile;

	wxString m_strInitTime;

	// WDR: method declarations for TParamsDlg
	wxRadioButton* GetUseGrid()  { return (wxRadioButton*) FindWindow( ID_USE_GRID ); }
	wxRadioButton* GetUseTin()  { return (wxRadioButton*) FindWindow( ID_USE_TIN ); }
	wxComboBox* GetLocfile()  { return (wxComboBox*) FindWindow( ID_LOCFILE ); }
	wxRadioButton* GetTiled()  { return (wxRadioButton*) FindWindow( ID_TILED ); }
	wxRadioButton* GetDerived()  { return (wxRadioButton*) FindWindow( ID_DERIVED ); }
	wxRadioButton* GetSingle()  { return (wxRadioButton*) FindWindow( ID_SINGLE ); }
	wxRadioButton* GetNone()  { return (wxRadioButton*) FindWindow( ID_NONE ); }
	wxTextCtrl* GetLightFactor()  { return (wxTextCtrl*) FindWindow( ID_LIGHT_FACTOR ); }
	wxComboBox* GetBuildingfile()  { return (wxComboBox*) FindWindow( ID_BUILDINGFILE ); }
	wxComboBox* GetRoadfile()  { return (wxComboBox*) FindWindow( ID_ROADFILE ); }
	wxComboBox* GetTreefile()  { return (wxComboBox*) FindWindow( ID_TREEFILE ); }
	wxComboBox* GetTfilesingle()  { return (wxComboBox*) FindWindow( ID_TFILESINGLE ); }
	wxChoice* GetLodmethod()  { return (wxChoice*) FindWindow( ID_LODMETHOD ); }
	wxComboBox* GetFilename()  { return (wxComboBox*) FindWindow( ID_FILENAME ); }
	wxComboBox* GetFilenameTin()  { return (wxComboBox*) FindWindow( ID_FILENAME_TIN ); }
	wxComboBox* GetSkytexture()  { return (wxComboBox*) FindWindow( ID_SKYTEXTURE ); }
	wxTextCtrl* GetFogDistance()  { return (wxTextCtrl*) FindWindow( ID_FOG_DISTANCE ); }
	wxTextCtrl* GetDepressOceanOffset()  { return (wxTextCtrl*) FindWindow( ID_DEPRESSOCEANOFFSET ); }
	wxTextCtrl* GetOceanPlaneOffset()  { return (wxTextCtrl*) FindWindow( ID_OCEANPLANEOFFSET ); }

	//  CButton m_PreLight;
//  CButton m_TriStrips;
	bool	m_bReady;
	bool	m_bSetting;

private:
	// WDR: member variable declarations for TParamsDlg
	wxTextCtrl* m_pPreLightFactor;
	wxComboBox* m_pBuildingFile;
	wxComboBox* m_pRoadFile;
	wxComboBox* m_pTreeFile;
	wxComboBox* m_pTextureFileSingle;
	wxChoice* m_pLodMethod;
	wxComboBox* m_pFilename;
	wxComboBox* m_pFilenameTin;
	wxRadioButton* m_pNone;
	wxRadioButton* m_pSingle;
	wxRadioButton* m_pDerived;
	wxRadioButton* m_pTiled;
	wxComboBox* m_pLocFile;
	wxComboBox* m_pSkyTexture;

private:
	// WDR: handler declarations for TParamsDlg
	void OnTextureFileBase( wxCommandEvent &event );
	void OnTileSize( wxCommandEvent &event );
	void OnTextureNone( wxCommandEvent &event );
	void OnTextureSingle( wxCommandEvent &event );
	void OnTextureDerived( wxCommandEvent &event );
	void OnTextureTiled( wxCommandEvent &event );
    void OnCheckBox( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __TParamsDlg_H__

