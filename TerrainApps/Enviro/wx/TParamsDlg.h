//
// Name: TParamsDlg.h
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __TParamsDlg_H__
#define __TParamsDlg_H__

#include "enviro_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"
#include "vtdata/TParams.h"
#include "vtdata/vtTime.h"

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
	~TParamsDlg();

	void OnInitDialog(wxInitDialogEvent& event);

	bool TransferDataToWindow();
	bool TransferDataFromWindow();

	void SetParams(const TParams &Params);
	void GetParams(TParams &Params);
	void SetDataPaths(const vtStringArray &paths) { m_datapaths = paths; }
	void UpdateTiledTextureFilename();
	void UpdateEnableState();
	void RefreshLocationFields();
	void UpdateTimeString();
	void UpdateColorControl();
	void UpdateColorMapChoice();
	void DeleteItem(wxListBox *pBox);
	int FindLayerByFilename(const wxString2 &fname);

	// overall name
	wxString2   m_strTerrainName;

	// elevation
	bool	m_bGrid;
	bool	m_bTin;
	bool	m_bTileset;
	wxString2   m_strFilename;
	wxString2   m_strFilenameTin;
	wxString2   m_strFilenameTileset;
	float   m_fVerticalExag;

	// navigation
	float	m_fMinHeight;
	int		m_iNavStyle;
	float   m_fNavSpeed;
	wxString2   m_strLocFile;
	int	 m_iInitLocation;
	wxString2   m_strInitLocation;
	float   m_fHither;
	bool	m_bAccel;

	// LOD
	int	 m_iLodMethod;
	int	 m_iTriCount;
	bool	m_bTriStrips;
	int	 m_iVertCount;
	int	 m_iTileCacheSize;

	// time
	bool	m_bTimeOn;
	vtTime  m_InitTime;
	float   m_fTimeSpeed;

	// texture
	int	 m_iTexture;
	int	 m_iTilesize;
	wxString2   m_strTextureSingle;
	wxString2   m_strTextureBase;
	wxString2   m_strTextureFilename;
	wxString2   m_strTextureTileset;
	bool	m_bJPEG;
	bool	m_bMipmap;
	bool	m_b16bit;
	bool	m_bPreLight;
	float   m_fPreLightFactor;
	bool	m_bCastShadows;
	wxString2   m_strColorMap;

	// detail texture
	bool	m_bDetailTexture;
	wxString2 m_strDetailName;
	float	m_fDetailScale;
	float	m_fDetailDistance;

	// culture
	bool	m_bRoads;
	wxString2   m_strRoadFile;
	bool	m_bHwy;
	bool	m_bPaved;
	bool	m_bDirt;
	float   m_fRoadHeight;
	float   m_fRoadDistance;
	bool	m_bTexRoads;
	bool	m_bRoadCulture;

	bool	m_bPlants;
	wxString2   m_strVegFile;
	int	 m_iVegDistance;

	bool	m_bFog;
	float   m_fFogDistance;

	std::vector<vtTagArray> m_Layers;
	vtStringArray m_AnimPaths;
	int		m_iStructDistance;
	bool	m_bStructureShadows;
	int		m_iStructureRez;
	float	m_fDarkness;
	wxString2 m_strContent;

	bool	m_bSky;
	wxString2   m_strSkyTexture;
	bool	m_bOceanPlane;
	float   m_fOceanPlaneLevel;
	bool	m_bDepressOcean;
	float   m_fDepressOceanLevel;
	bool	m_bHorizon;
	wxColor	m_BgColor;

	bool	m_bVehicles;
//  float   m_fVehicleSize;
//  float   m_fVehicleSpeed;

	bool	m_bRouteEnable;		// not yet used
	wxString2 m_strRouteFile;	// not yet used

	wxString2 m_strOverlayFile;
	int		m_iOverlayX;
	int		m_iOverlayY;

	wxString2 m_strInitTime;
//  wxString2   m_strMemRequired;

// Scenarios
	std::vector<ScenarioParams> m_Scenarios;

	// WDR: method declarations for TParamsDlg
	wxButton* GetMovedownScenario()  { return (wxButton*) FindWindow( ID_MOVEDOWN_SCENARIO ); }
	wxButton* GetMoveupScenario()  { return (wxButton*) FindWindow( ID_MOVEUP_SCENARIO ); }
	wxButton* GetEditScenario()  { return (wxButton*) FindWindow( ID_EDIT_SCENARIO ); }
	wxButton* GetDeleteScenario()  { return (wxButton*) FindWindow( ID_DELETE_SCENARIO ); }
	wxListBox* GetScenarioList()  { return (wxListBox*) FindWindow( ID_SCENARIO_LIST ); }
	wxListBox* GetAnimPaths()  { return (wxListBox*) FindWindow( ID_ANIM_PATHS ); }
	wxComboBox* GetDTName()  { return (wxComboBox*) FindWindow( ID_DT_NAME ); }
	wxChoice* GetColorMap()  { return (wxChoice*) FindWindow( ID_CHOICE_COLORS ); }
	wxStaticBitmap* GetColorBitmap()  { return (wxStaticBitmap*) FindWindow( ID_COLOR3 ); }
	wxChoice* GetChoiceShadowRez()  { return (wxChoice*) FindWindow( ID_CHOICE_SHADOW_REZ ); }
	wxCheckBox* GetCheckStructureShadows()  { return (wxCheckBox*) FindWindow( ID_CHECK_STRUCTURE_SHADOWS ); }
	wxComboBox* GetContentFile()  { return (wxComboBox*) FindWindow( ID_CONTENT_FILE ); }
	wxChoice* GetNavStyle()  { return (wxChoice*) FindWindow( ID_NAV_STYLE ); }
	wxRadioButton* GetUseGrid()  { return (wxRadioButton*) FindWindow( ID_USE_GRID ); }
	wxRadioButton* GetUseTin()  { return (wxRadioButton*) FindWindow( ID_USE_TIN ); }
	wxRadioButton* GetUseTileset()  { return (wxRadioButton*) FindWindow( ID_USE_TILESET ); }
	wxComboBox* GetLocfile()  { return (wxComboBox*) FindWindow( ID_LOCFILE ); }
	wxRadioButton* GetTileset()  { return (wxRadioButton*) FindWindow( ID_TILESET ); }
	wxRadioButton* GetTiled()  { return (wxRadioButton*) FindWindow( ID_TILED_4BY4 ); }
	wxRadioButton* GetDerived()  { return (wxRadioButton*) FindWindow( ID_DERIVED ); }
	wxRadioButton* GetSingle()  { return (wxRadioButton*) FindWindow( ID_SINGLE ); }
	wxRadioButton* GetNone()  { return (wxRadioButton*) FindWindow( ID_NONE ); }
	wxTextCtrl* GetLightFactor()  { return (wxTextCtrl*) FindWindow( ID_LIGHT_FACTOR ); }
	wxComboBox* GetRoadfile()  { return (wxComboBox*) FindWindow( ID_ROADFILE ); }
	wxComboBox* GetTreefile()  { return (wxComboBox*) FindWindow( ID_TREEFILE ); }
	wxComboBox* GetTfileSingle()  { return (wxComboBox*) FindWindow( ID_TFILE_SINGLE ); }
	wxComboBox* GetTfileTileset()  { return (wxComboBox*) FindWindow( ID_TFILE_TILESET ); }
	wxChoice* GetLodmethod()  { return (wxChoice*) FindWindow( ID_LODMETHOD ); }
	wxComboBox* GetFilename()  { return (wxComboBox*) FindWindow( ID_FILENAME ); }
	wxComboBox* GetFilenameTin()  { return (wxComboBox*) FindWindow( ID_FILENAME_TIN ); }
	wxComboBox* GetFilenameTileset()  { return (wxComboBox*) FindWindow( ID_FILENAME_TILES ); }
	wxComboBox* GetSkytexture()  { return (wxComboBox*) FindWindow( ID_SKYTEXTURE ); }
	wxTextCtrl* GetFogDistance()  { return (wxTextCtrl*) FindWindow( ID_FOG_DISTANCE ); }
	wxTextCtrl* GetDepressOceanOffset()  { return (wxTextCtrl*) FindWindow( ID_DEPRESSOCEANOFFSET ); }
	wxTextCtrl* GetOceanPlaneOffset()  { return (wxTextCtrl*) FindWindow( ID_OCEANPLANEOFFSET ); }
	wxChoice* GetLocField()  { return (wxChoice*) FindWindow( ID_INIT_LOCATION ); }
	wxListBox* GetStructFiles()  { return (wxListBox*) FindWindow( ID_STRUCTFILES ); }
	wxListBox* GetRawFiles()  { return (wxListBox*) FindWindow( ID_RAWFILES ); }

	bool	m_bReady;
	bool	m_bSetting;
	vtStringArray m_datapaths;

private:
	// WDR: member variable declarations for TParamsDlg
	wxComboBox* m_pFilename;
	wxComboBox* m_pFilenameTin;
	wxComboBox* m_pFilenameTileset;
	wxListBox* m_pAnimFiles;
	wxListBox* m_pRawFiles;
	wxComboBox* m_pDTName;
	wxChoice* m_pNavStyle;
	wxTextCtrl* m_pPreLightFactor;
	wxListBox* m_pStructFiles;
	wxComboBox* m_pRoadFile;
	wxComboBox* m_pTreeFile;
	wxComboBox* m_pTextureFileSingle;
	wxComboBox* m_pTextureFileTileset;
	wxChoice* m_pLodMethod;
	wxRadioButton* m_pNone;
	wxRadioButton* m_pSingle;
	wxRadioButton* m_pDerived;
	wxRadioButton* m_pTiled;
	wxRadioButton* m_pTileset;
	wxComboBox* m_pLocFile;
	wxComboBox* m_pSkyTexture;
	wxChoice* m_pLocField;
	wxChoice* m_pShadowRez;
	wxChoice* m_pColorMap;
	wxListBox* m_pScenarioList;

private:
	// WDR: handler declarations for TParamsDlg
	void OnOverlay( wxCommandEvent &event );
	void OnBgColor( wxCommandEvent &event );
	void OnTextureFileBase( wxCommandEvent &event );
	void OnTileSize( wxCommandEvent &event );
	void OnTextureNone( wxCommandEvent &event );
	void OnTextureSingle( wxCommandEvent &event );
	void OnTextureDerived( wxCommandEvent &event );
	void OnTextureTiled( wxCommandEvent &event );
	void OnTextureTileset( wxCommandEvent &event );
	void OnEditColors( wxCommandEvent &event );
	void OnCheckBox( wxCommandEvent &event );
	void OnListDblClickStructure( wxCommandEvent &event );
	void OnListDblClickRaw( wxCommandEvent &event );
	void OnListDblClickAnimPaths( wxCommandEvent &event );
	void OnChoiceLocFile( wxCommandEvent &event );
	void OnChoiceInitLocation( wxCommandEvent &event );
	void OnSetInitTime( wxCommandEvent &event );
	void OnStyle( wxCommandEvent &event );
	void OnScenarioListEvent( wxCommandEvent &event );
	void OnNewScenario( wxCommandEvent &event );
	void OnDeleteScenario( wxCommandEvent &event );
	void OnEditScenario( wxCommandEvent &event );
	void OnMoveUpScenario( wxCommandEvent &event );
	void OnMoveDownSceanario( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __TParamsDlg_H__

