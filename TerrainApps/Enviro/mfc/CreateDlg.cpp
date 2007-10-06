// CreateDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "Charset.h"

#include "vtlib/vtlib.h"
#include "stdio.h"
#include "CreateDlg.h"
#include "../Options.h"

#include "vtlib/core/SRTerrain.h"
#include "vtlib/core/TVTerrain.h"
#include "vtlib/core/SMTerrain.h"
#include "vtdata/vtLog.h"
#include ".\createdlg.h"

#ifdef _MSC_VER
#pragma warning( disable : 4800 )
#endif

#define NTILES 4

/////////////////////////////////////////////////////////////////////////////
// CCreateDlg dialog

CCreateDlg::CCreateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateDlg)
	m_strFilename = _T("");
	m_fVerticalExag = 1.0f;
	m_bTimeOn = FALSE;
	m_bSky = TRUE;
	m_bFog = FALSE;
	m_iTexture = 3;
	m_iTilesize = 512;
	m_strTextureSingle = _T("");
	m_strTextureBase = _T("");
	m_strTexture4x4 = _T("");
	m_bMipmap = FALSE;
	m_b16bit = TRUE;
	m_bRoads = FALSE;
	m_strRoadFile = _T("");
	m_bTexRoads = TRUE;
	m_bPlants = FALSE;
	m_strTreeFile = _T("");
	m_iVegDistance = 10;
	m_fFogDistance = 10;
	m_bOceanPlane = FALSE;
	m_iMinHeight = 10;
	m_strBuildingFile = _T("");
	m_bVehicles = FALSE;
	m_fVehicleSize = 1.0f;
	m_bTriStrips = FALSE;
	m_bDetailTexture = FALSE;
	m_bPreLight = TRUE;
	m_bDirt = FALSE;
	m_bHwy = TRUE;
	m_bPaved = TRUE;
	m_tTime = CTime(1999, 3, 20, 10, 0, 0);
	m_fRoadDistance = 4.0f;
	m_fRoadHeight = 2.0f;
	m_fNavSpeed = 10.0f;
	m_fPreLightFactor = 1.0f;
	m_bRoadCulture = FALSE;
	m_iLodMethod = 0;
	m_fVehicleSpeed = 1.0f;
	m_bPreLit = FALSE;
	//}}AFX_DATA_INIT

	// Beware: we need to store and display the same time on all
	// systems, but using MFC's CDateTimeCtrl will always show
	// the local time (different on each machine.
	// To work around this, we determine the offset between local
	// and Greenwich Mean time, subtract it when importing values,
	// and add it when exporting values.

	// compute standard time difference
	time_t dummy = 20000;
	struct tm tm_gm, tm_local;
	tm_gm = *gmtime(&dummy);
	tm_local = *localtime(&dummy);
	m_iTimeDiff = mktime(&tm_local) - mktime(&tm_gm);
}

CCreateDlg::~CCreateDlg()
{
	// a place to check to make sure the object is deleted
	int foo = 1;
}


void CCreateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateDlg)
	DDX_Text(pDX, IDC_TNAME, m_strName);
	DDX_Control(pDX, IDC_LODMETHOD, m_cbLodMethod);
	DDX_Control(pDX, IDC_LIGHT_FACTOR, m_cePreLightFactor);
	DDX_Control(pDX, IDC_INITTIME, m_dtTime);
	DDX_Control(pDX, IDC_BUILDINGFILE, m_cbBuildingFile);
	DDX_Control(pDX, IDC_ROADFILE, m_cbRoadFile);
	DDX_Control(pDX, IDC_TREEFILE, m_cbTreeFile);
	DDX_Control(pDX, IDC_TIMEOFDAY, m_TimeOfDay);
	DDX_Control(pDX, IDC_PRELIGHT, m_PreLight);
	DDX_Control(pDX, IDC_TRISTRIPS, m_TriStrips);
	DDX_Control(pDX, IDC_TFILESINGLE, m_cbTextureFileSingle);
	DDX_Control(pDX, IDC_FILENAME, m_cbLODFilename);
	DDX_CBString(pDX, IDC_FILENAME, m_strFilename);
	DDX_Text(pDX, IDC_VERTEXAG, m_fVerticalExag);
	DDV_MinMaxFloat(pDX, m_fVerticalExag, 0.1f, 5.f);
	DDX_Check(pDX, IDC_TIMEOFDAY, m_bTimeOn);
	DDX_Check(pDX, IDC_SKY, m_bSky);
	DDX_Check(pDX, IDC_FOG, m_bFog);
	DDX_Radio(pDX, IDC_NONE, m_iTexture);
	DDX_Text(pDX, IDC_TILESIZE, m_iTilesize);
	DDV_MinMaxUInt(pDX, m_iTilesize, 0, 4096);
	DDX_Text(pDX, IDC_TFILESINGLE, m_strTextureSingle);
	DDX_Text(pDX, IDC_TFILEBASE, m_strTextureBase);
	DDX_Text(pDX, IDC_TFILENAME, m_strTexture4x4);
	DDX_Check(pDX, IDC_MIPMAP, m_bMipmap);
	DDX_Check(pDX, IDC_16BIT, m_b16bit);
	DDX_Check(pDX, IDC_ROADS, m_bRoads);
	DDX_CBString(pDX, IDC_ROADFILE, m_strRoadFile);
	DDX_Check(pDX, IDC_TEXROADS, m_bTexRoads);
	DDX_Check(pDX, IDC_PLANTS, m_bPlants);
	DDX_CBString(pDX, IDC_TREEFILE, m_strTreeFile);
	DDX_Text(pDX, IDC_VEGDISTANCE, m_iVegDistance);
	DDV_MinMaxUInt(pDX, m_iVegDistance, 10, 100000);
	DDX_Text(pDX, IDC_FOGDISTANCE, m_fFogDistance);
	DDX_Check(pDX, IDC_OCEANPLANE, m_bOceanPlane);
	DDX_Text(pDX, IDC_MINHEIGHT, m_iMinHeight);
	DDX_CBString(pDX, IDC_BUILDINGFILE, m_strBuildingFile);
	DDX_Check(pDX, IDC_VEHICLES, m_bVehicles);
	DDX_Text(pDX, IDC_VEHICLESIZE, m_fVehicleSize);
	DDV_MinMaxFloat(pDX, m_fVehicleSize, 1.e-002f, 1000.f);
	DDX_Check(pDX, IDC_TRISTRIPS, m_bTriStrips);
	DDX_Check(pDX, IDC_DETAILTEXTURE, m_bDetailTexture);
	DDX_Check(pDX, IDC_PRELIGHT, m_bPreLight);
	DDX_Check(pDX, IDC_DIRT, m_bDirt);
	DDX_Check(pDX, IDC_HWY, m_bHwy);
	DDX_Check(pDX, IDC_PAVED, m_bPaved);
	DDX_DateTimeCtrl(pDX, IDC_INITTIME, m_tTime);
	DDX_Text(pDX, IDC_ROADDISTANCE, m_fRoadDistance);
	DDX_Text(pDX, IDC_ROADHEIGHT, m_fRoadHeight);
	DDX_Text(pDX, IDC_NAVSPEED, m_fNavSpeed);
	DDX_Text(pDX, IDC_LIGHT_FACTOR, m_fPreLightFactor);
	DDV_MinMaxFloat(pDX, m_fPreLightFactor, 0.f, 1.f);
	DDX_Check(pDX, IDC_ROADCULTURE, m_bRoadCulture);
	DDX_CBIndex(pDX, IDC_LODMETHOD, m_iLodMethod);
	DDX_Text(pDX, IDC_VEHICLESPEED, m_fVehicleSpeed);
	DDV_MinMaxFloat(pDX, m_fVehicleSpeed, 1.e-002f, 1000.f);
	DDX_Check(pDX, IDC_PRELIT, m_bPreLit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateDlg, CDialog)
	//{{AFX_MSG_MAP(CCreateDlg)
	ON_EN_CHANGE(IDC_TILESIZE, OnChangeTilesize)
	ON_EN_CHANGE(IDC_TFILEBASE, OnChangeTfilebase)
	ON_CBN_EDITCHANGE(IDC_FILENAME, OnEditchangeFilename)
	ON_CBN_SELCHANGE(IDC_FILENAME, OnSelchangeFilename)
	ON_CBN_SELCHANGE(IDC_TFILESINGLE, OnSelchangeTfilesingle)
	ON_BN_CLICKED(IDC_PRELIGHT, OnPrelight)
	ON_CBN_SELCHANGE(IDC_LODMETHOD, OnSelchangeLodmethod)
	ON_COMMAND(ID_ENVIRO_SHOWTREES, OnEnviroShowtrees)
	ON_BN_CLICKED(IDC_DYNAMIC, OnChangeMem)
	ON_BN_CLICKED(IDC_NONE, OnChangeMem)
	ON_BN_CLICKED(IDC_SINGLE, OnChangeMem)
	ON_BN_CLICKED(IDC_TILED, OnChangeMem)
	ON_BN_CLICKED(IDC_DERIVED, OnChangeMem)
	ON_BN_CLICKED(IDC_PRELIT, OnPrelit)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_TFILEBASE2, OnEnChangeTfilebase2)
	ON_EN_CHANGE(IDC_TNAME, OnEnChangeTname)
END_MESSAGE_MAP()

//
// This function replaces CComboBox::Dir, because that method
//  does not support non-8.3 directory names (or file names)
//  in Win9x operating systems.
//
void AddFilenamesToComboBox(CComboBox *box, const char *pattern)
{
	CString wildcard = pattern;
	CFileFind finder;
	BOOL bWorking = finder.FindFile(wildcard);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (!finder.IsDirectory())
		{
			box->AddString(finder.GetFileName());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCreateDlg message handlers

BOOL CCreateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

//	unsigned int iFlags = DDL_READWRITE | DDL_READONLY;

	unsigned int i;
	vtStringArray &paths = vtGetDataPath();

	for (i = 0; i < paths.size(); i++)
	{
		// fill the "terrain filename" control with available terrain files
		AddFilenamesToComboBox(&m_cbLODFilename, paths[i] + "Elevation/*.bt*");
		m_cbLODFilename.SelectString(-1, m_strFilename);

		// fill the "single texture filename" control with available bitmap files
		AddFilenamesToComboBox(&m_cbTextureFileSingle, paths[i] + "GeoSpecific/*.bmp");
		m_cbLODFilename.SelectString(-1, m_strTextureSingle);

		// fill in Road files
		AddFilenamesToComboBox(&m_cbRoadFile, paths[i] + "RoadData/*.rmf");
		m_cbRoadFile.SelectString(-1, m_strRoadFile);

		// fill in Building files
		AddFilenamesToComboBox(&m_cbBuildingFile, paths[i] + "BuildingData/*.vtst");
		m_cbBuildingFile.SelectString(-1, m_strBuildingFile);

		// fill in Tree files
		AddFilenamesToComboBox(&m_cbTreeFile, paths[i] + "PlantData/*.vf");
		m_cbTreeFile.SelectString(-1, m_strTreeFile);
	}

	m_cbLodMethod.AddString(_T("Roettger"));
	m_cbLodMethod.AddString(_T("TopoVista"));
	m_cbLodMethod.AddString(_T("McNally"));
	m_cbLodMethod.SetCurSel(m_iLodMethod);

	DetermineTerrainSizeFromBT();
	DetermineSizeFromBMP();

	OnChangeMem();
	UpdateTiledTextureFilename();

	UpdateTimeEnable();

	ShowWindow(SW_SHOW);

	return TRUE;
}

void CCreateDlg::SetParams(TParams &Params)
{
	VTLOG("CCreateDlg::SetParams\n");

	// set the values in the dialog from the supplied paramter structure
	m_strName =			Params.GetValueString(STR_NAME);
	m_strFilename =		Params.GetValueString(STR_ELEVFILE);
	// LocationsFilename
	m_fVerticalExag =	Params.GetValueFloat(STR_VERTICALEXAG);
	m_iMinHeight =		Params.GetValueInt(STR_MINHEIGHT);
	m_fNavSpeed =		Params.GetValueFloat(STR_NAVSPEED);

	m_iLodMethod =		Params.GetLodMethod();
	m_bTriStrips =		Params.GetValueBool(STR_TRISTRIPS);
	m_bDetailTexture =	Params.GetValueBool(STR_DETAILTEXTURE);

	m_bTimeOn =			Params.GetValueBool(STR_TIMEON);
	m_tTime =			Params.GetValueInt(STR_INITTIME)
		- m_iTimeDiff;	// beware local/gmtime difference

	// texture
	m_iTexture =		Params.GetTextureEnum();

	// single
	if (m_iTexture != TE_TILESET)
		m_strTextureSingle = Params.GetValueString(STR_TEXTUREFILE);

	// tile4x4
	m_strTextureBase =	Params.GetValueString(STR_TEXTUREBASE);
	m_iTilesize =		Params.GetValueInt(STR_TILESIZE);
	m_strTexture4x4 =	Params.GetValueString(STR_TEXTURE4BY4);

	m_bMipmap =			Params.GetValueBool(STR_MIPMAP);
	m_b16bit =			Params.GetValueBool(STR_REQUEST16BIT);
	m_bPreLight =		Params.GetValueBool(STR_PRELIGHT);
	m_fPreLightFactor =	Params.GetValueFloat(STR_PRELIGHTFACTOR);

	m_bRoads =			Params.GetValueBool(STR_ROADS);
	m_strRoadFile =		Params.GetValueString(STR_ROADFILE);
	m_bHwy =			Params.GetValueBool(STR_HWY);
	m_bPaved =			Params.GetValueBool(STR_PAVED);
	m_bDirt =			Params.GetValueBool(STR_DIRT);
	m_fRoadHeight =		Params.GetValueFloat(STR_ROADHEIGHT);
	m_fRoadDistance =	Params.GetValueFloat(STR_ROADDISTANCE);
	m_bTexRoads =		Params.GetValueBool(STR_TEXROADS);
	m_bRoadCulture =	Params.GetValueBool(STR_ROADCULTURE);

	m_bPlants =			Params.GetValueBool(STR_TREES);
	m_strTreeFile =		Params.GetValueString(STR_TREEFILE);
	m_iVegDistance =	Params.GetValueInt(STR_VEGDISTANCE);

	m_bFog =			Params.GetValueBool(STR_FOG);
	m_fFogDistance =	Params.GetValueFloat(STR_FOGDISTANCE);

	// TODO: real layer support for the MFC version
	m_strBuildingFile = "";

	m_bVehicles =		Params.GetValueBool(STR_VEHICLES);
	m_fVehicleSize =	Params.GetValueFloat(STR_VEHICLESIZE);
	m_fVehicleSpeed =	Params.GetValueFloat(STR_VEHICLESPEED);

	m_bSky =			Params.GetValueBool(STR_SKY);
	m_bOceanPlane =		Params.GetValueBool(STR_OCEANPLANE);
}

void CCreateDlg::GetParams(TParams &Params)
{
	VTLOG("CCreateDlg::GetParams\n");

	// get the values from the dialog into the supplied paramter structure
	Params.SetValueString(STR_NAME, ToUTF8(m_strName));
	Params.SetValueString(STR_ELEVFILE, ToUTF8(m_strFilename));
	// LocationsFilename
	Params.SetValueFloat(STR_VERTICALEXAG, m_fVerticalExag);
	Params.SetValueInt(STR_MINHEIGHT, m_iMinHeight);
	Params.SetValueFloat(STR_NAVSPEED, m_fNavSpeed);

	Params.SetLodMethod((enum LodMethodEnum) m_iLodMethod);
	Params.SetValueBool(STR_TRISTRIPS, m_bTriStrips);
	Params.SetValueBool(STR_DETAILTEXTURE, m_bDetailTexture);

	Params.SetValueBool(STR_TIMEON, m_bTimeOn);

	int time = m_tTime.GetTime();
	time += m_iTimeDiff;	// beware local/gmtime difference
	Params.SetValueInt(STR_INITTIME, time);

	// texture
	Params.SetTextureEnum((enum TextureEnum)m_iTexture);

	// single
	if (m_iTexture != TE_TILESET)
	Params.SetValueString(STR_TEXTUREFILE, ToUTF8(m_strTextureSingle));

	// tile4x4
	Params.SetValueInt(STR_TILESIZE, m_iTilesize);
	Params.SetValueString(STR_TEXTUREBASE, ToUTF8(m_strTextureBase));
	Params.SetValueString(STR_TEXTURE4BY4, ToUTF8(m_strTexture4x4));

	Params.SetValueBool(STR_MIPMAP, m_bMipmap);
	Params.SetValueBool(STR_REQUEST16BIT, m_b16bit);
	Params.SetValueBool(STR_PRELIGHT, m_bPreLight);
	Params.SetValueFloat(STR_PRELIGHTFACTOR, m_fPreLightFactor);

	Params.SetValueBool(STR_ROADS, m_bRoads);
	Params.SetValueString(STR_ROADFILE, ToUTF8(m_strRoadFile));
	Params.SetValueBool(STR_HWY, m_bHwy);
	Params.SetValueBool(STR_PAVED, m_bPaved);
	Params.SetValueBool(STR_DIRT, m_bDirt);
	Params.SetValueFloat(STR_ROADHEIGHT, m_fRoadHeight);
	Params.SetValueFloat(STR_ROADDISTANCE, m_fRoadDistance);
	Params.SetValueBool(STR_TEXROADS, m_bTexRoads);
	Params.SetValueBool(STR_ROADCULTURE, m_bRoadCulture);

	Params.SetValueBool(STR_TREES, m_bPlants);
	Params.SetValueString(STR_TREEFILE, ToUTF8(m_strTreeFile));
	Params.SetValueInt(STR_VEGDISTANCE, m_iVegDistance);

	Params.SetValueBool(STR_FOG, m_bFog);
	Params.SetValueFloat(STR_FOGDISTANCE, m_fFogDistance);

	// TODO: support layers in MFC version
//	psl.m_strStructFile = (const char *) m_strBuildingFile;

	Params.SetValueBool(STR_VEHICLES, m_bVehicles);
//	Params.SetValueFloat(STR_VEHICLESIZE, m_fVehicleSize);
//	Params.SetValueFloat(STR_VEHICLESPEED, m_fVehicleSpeed);

	Params.SetValueBool(STR_SKY, m_bSky);

	Params.SetValueBool(STR_OCEANPLANE, m_bOceanPlane);
}


void CCreateDlg::UpdateTiledTextureFilename()
{
	m_strTexture4x4.Format(_T("%s%d.bmp"), m_strTextureBase,
		NTILES * (m_iTilesize-1) + 1);
	UpdateData(FALSE);
}

void CCreateDlg::OnChangeTilesize()
{
	UpdateData(TRUE);
	UpdateTiledTextureFilename();
	OnChangeMem();
}


void CCreateDlg::OnChangeTfilebase()
{
	UpdateData(TRUE);
	UpdateTiledTextureFilename();
}


void CCreateDlg::OnChangeMem()
{
	UpdateData(TRUE);	// retrieve data

	// if TVTerrain or tiled LKTerrain, no tristrips
	bool enable_strips = true;
	if (m_iLodMethod == LM_TOPOVISTA ||
		(m_iTexture == 2 && m_iLodMethod == LM_ROETTGER))
	{
		m_bTriStrips = FALSE;
		enable_strips = false;
	}
	m_TriStrips.EnableWindow(enable_strips);

	// if TVTerrain, no tiled textures
	if (m_iLodMethod == LM_TOPOVISTA)
		bool todo = true;

	// if derived texture, prelit only if prelighting
	if (m_iTexture == 3) m_bPreLit = m_bPreLight;
}


void CCreateDlg::DetermineTerrainSizeFromBT()
{
	vtString fname = "Elevation/";
	fname += ToUTF8(m_strFilename);
	vtString fpath = FindFileOnPaths(vtGetDataPath(), fname);
	if (fpath == "")
	{
		m_iTerrainSize = 1024;
		return;
	}

	vtElevationGrid grid;
	if (!grid.LoadBTHeader(fpath))
	{
		m_iTerrainSize = 1024;
		return;
	}

	int col, row;
	grid.GetDimensions(col, row);
	m_iTerrainSize = col;
	m_iTerrainDepth = grid.IsFloatMode() ? 4 : 2;
}

void CCreateDlg::DetermineSizeFromBMP()
{
	BITMAPINFOHEADER	Hdr;
	HFILE				fileHandle;
	OFSTRUCT			openFileStruct;
	BITMAPFILEHEADER	bitmapHdr;

	vtString fname = "GeoSpecific/";
	fname += ToUTF8(m_strTextureSingle);
	vtString fpath = FindFileOnPaths(vtGetDataPath(), fname);
	if (fpath == "")
		goto ErrExit;

	fileHandle = OpenFile(fpath, &openFileStruct, OF_READ);
	if (fileHandle == HFILE_ERROR) {
		return;
	}

	if (sizeof (bitmapHdr) !=
		_lread (fileHandle, (LPSTR)&bitmapHdr, sizeof (bitmapHdr))) {
		goto ErrExit;
	}

	if (bitmapHdr.bfType != 0x4d42) {
		goto ErrExit;
	}

	if (sizeof(BITMAPINFOHEADER) !=
		_lread (fileHandle, (LPSTR)&Hdr, sizeof(BITMAPINFOHEADER))) {
		goto ErrExit;
	}
	_lclose(fileHandle);

	if (Hdr.biSize == sizeof(BITMAPCOREHEADER)) {
		goto ErrExit;
	}
	m_iTextureSize = Hdr.biWidth;
	return;

ErrExit:
	m_iTextureSize = 1024;
	return;
}

void CCreateDlg::OnEditchangeFilename()
{
//	UpdateData(TRUE);
//	DetermineTerrainSizeFromBT();
//	OnChangeMem();
}

void CCreateDlg::OnSelchangeFilename()
{
	// always do this first
	UpdateData(TRUE);

	CString str;
	int cursel = m_cbLODFilename.GetCurSel();
	if (cursel != -1)
	{
		m_cbLODFilename.GetLBText(cursel, m_strFilename);
		DetermineTerrainSizeFromBT();
	}
}

void CCreateDlg::OnSelchangeTfilesingle()
{
	// always do this first
	UpdateData(TRUE);

	CString str;
	m_cbTextureFileSingle.GetLBText(m_cbTextureFileSingle.GetCurSel(), m_strTextureSingle);
	DetermineSizeFromBMP();
}

void CCreateDlg::UpdateTimeEnable()
{
//	m_TimeOfDay.EnableWindow(!m_bPreLight);
//	m_dtTime.EnableWindow(!m_bPreLight);
	m_TimeOfDay.EnableWindow(false);
	m_dtTime.EnableWindow(false);
	m_cePreLightFactor.EnableWindow(m_bPreLight);
//	if (m_bPreLight)
		m_bTimeOn = false;
}

void CCreateDlg::OnPrelight()
{
	// always do this first
	UpdateData(TRUE);

	UpdateTimeEnable();
	if (m_bPreLight) m_bPreLit = TRUE;

	// if derived texture, prelit only if prelighting
	if (m_iTexture == 3) m_bPreLit = m_bPreLight;

	UpdateData(FALSE);
}

void CCreateDlg::OnSelchangeLodmethod()
{
	OnChangeMem();
}

void CCreateDlg::OnEnviroShowtrees()
{
	m_bPlants = !m_bPlants;
}

void CCreateDlg::OnPrelit()
{
	// always do this first
	UpdateData(TRUE);
	if (m_bPreLight) m_bPreLit = TRUE;
	UpdateData(FALSE);
}

void CCreateDlg::OnCbnSelchangeFilename2()
{
	// TODO: Add your control notification handler code here
}

void CCreateDlg::OnEnChangeTfilebase2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CCreateDlg::OnEnChangeTname()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
