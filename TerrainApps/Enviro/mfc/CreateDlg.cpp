// CreateDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "vtlib/vtlib.h"
#include "stdio.h"
#include "CreateDlg.h"
#include "../Options.h"

#include "vtlib/core/SRTerrain.h"
#include "vtlib/core/TVTerrain.h"
#include "vtlib/core/SMTerrain.h"

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
	m_fPixelError = 2.0f;
	m_bTimeOn = FALSE;
	m_bSky = TRUE;
	m_bFog = FALSE;
	m_iTexture = 3;
	m_iTilesize = 512;
	m_strTextureSingle = _T("");
	m_strTextureBase = _T("");
	m_strTextureFilename = _T("");
	m_bMipmap = FALSE;
	m_b16bit = TRUE;
	m_bRoads = FALSE;
	m_strRoadFile = _T("");
	m_bTexRoads = TRUE;
	m_bPlants = FALSE;
	m_strTreeFile = _T("");
	m_iVegDistance = 10;
	m_fFogDistance = 10;
	m_bOverlay = FALSE;
	m_bOceanPlane = FALSE;
	m_bLabels = FALSE;
	m_iMinHeight = 10;
	m_strBuildingFile = _T("");
	m_bVehicles = FALSE;
	m_fVehicleSize = 1.0f;
	m_bTriStrips = FALSE;
	m_strMemRequired = _T("");
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
	m_iNumCars = 0;
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
	DDX_Text(pDX, IDC_PIXELERROR, m_fPixelError);
	DDX_Check(pDX, IDC_TIMEOFDAY, m_bTimeOn);
	DDX_Check(pDX, IDC_SKY, m_bSky);
	DDX_Check(pDX, IDC_FOG, m_bFog);
	DDX_Radio(pDX, IDC_NONE, m_iTexture);
	DDX_Text(pDX, IDC_TILESIZE, m_iTilesize);
	DDV_MinMaxUInt(pDX, m_iTilesize, 0, 4096);
	DDX_Text(pDX, IDC_TFILESINGLE, m_strTextureSingle);
	DDX_Text(pDX, IDC_TFILEBASE, m_strTextureBase);
	DDX_Text(pDX, IDC_TFILENAME, m_strTextureFilename);
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
	DDX_Check(pDX, IDC_OVERLAY, m_bOverlay);
	DDX_Check(pDX, IDC_OCEANPLANE, m_bOceanPlane);
	DDX_Check(pDX, IDC_LABELS, m_bLabels);
	DDX_Text(pDX, IDC_MINHEIGHT, m_iMinHeight);
	DDX_CBString(pDX, IDC_BUILDINGFILE, m_strBuildingFile);
	DDX_Check(pDX, IDC_VEHICLES, m_bVehicles);
	DDX_Text(pDX, IDC_VEHICLESIZE, m_fVehicleSize);
	DDV_MinMaxFloat(pDX, m_fVehicleSize, 1.e-002f, 1000.f);
	DDX_Check(pDX, IDC_TRISTRIPS, m_bTriStrips);
	DDX_Text(pDX, IDC_MEMREQUIRED, m_strMemRequired);
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
	DDX_Text(pDX, IDC_CARNUMBER, m_iNumCars);
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
	vtStringArray &paths = g_Options.m_DataPaths;

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

	m_cbLodMethod.AddString("Roettger");
	m_cbLodMethod.AddString("TopoVista");
	m_cbLodMethod.AddString("McNally");
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
	// set the values in the dialog from the supplied paramter structure
	m_strFilename = Params.m_strElevFile;
	// LocationsFilename
	m_fVerticalExag = Params.m_fVerticalExag;
	m_iMinHeight = Params.m_iMinHeight;
	m_fNavSpeed = Params.m_fNavSpeed;

	m_iLodMethod = Params.m_eLodMethod;
	m_fPixelError = Params.m_fPixelError;
	m_bTriStrips = Params.m_bTriStrips;
	m_bDetailTexture = Params.m_bDetailTexture;

	m_bTimeOn = Params.m_bTimeOn;
	m_tTime = Params.m_iInitTime - m_iTimeDiff;	// beware local/gmtime difference

	m_iTexture = Params.m_eTexture;
	m_iTilesize = Params.m_iTilesize;
	m_strTextureSingle = Params.m_strTextureSingle;
	m_strTextureBase = Params.m_strTextureBase;
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

	m_bPlants = Params.m_bPlants;
	m_strTreeFile = Params.m_strVegFile;
	m_iVegDistance = Params.m_iVegDistance;

	m_bFog = Params.m_bFog;
	m_fFogDistance = Params.m_fFogDistance;

	if (Params.m_strStructFiles.size() > 0)
		m_strBuildingFile = (const char *) Params.m_strStructFiles[0];
	else
		m_strBuildingFile = "";

	m_bVehicles = Params.m_bVehicles;
	m_fVehicleSize = Params.m_fVehicleSize;
	m_fVehicleSpeed = Params.m_fVehicleSpeed;
	m_iNumCars = Params.m_iNumCars;

	m_bSky = Params.m_bSky;
	m_bOceanPlane = Params.m_bOceanPlane;
	m_bOverlay = Params.m_bOverlay;
	m_bLabels = Params.m_bLabels;

	m_bPreLight = Params.m_bPreLight;
	m_bPreLit = Params.m_bPreLit;
	m_fPreLightFactor = Params.m_fPreLightFactor;
}

void CCreateDlg::GetParams(TParams &Params)
{
	// get the values from the dialog into the supplied paramter structure
	Params.m_strElevFile = m_strFilename;
	// LocationsFilename
	Params.m_fVerticalExag = m_fVerticalExag;
	Params.m_iMinHeight = m_iMinHeight;
	Params.m_fNavSpeed = m_fNavSpeed;

	Params.m_eLodMethod = (enum LodMethodEnum) m_iLodMethod;
	Params.m_fPixelError = m_fPixelError;
	Params.m_bTriStrips = m_bTriStrips;
	Params.m_bDetailTexture = m_bDetailTexture;

	Params.m_bTimeOn = m_bTimeOn;

	Params.m_iInitTime = m_tTime.GetTime();
	Params.m_iInitTime += m_iTimeDiff;	// beware local/gmtime difference

	Params.m_eTexture = (enum TextureEnum)m_iTexture;
	Params.m_iTilesize = m_iTilesize;
	Params.m_strTextureSingle = m_strTextureSingle;
	Params.m_strTextureBase = m_strTextureBase;
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

	Params.m_bPlants = m_bPlants;
	Params.m_strVegFile = m_strTreeFile;
	Params.m_iVegDistance = m_iVegDistance;

	Params.m_bFog = m_bFog;
	Params.m_fFogDistance = m_fFogDistance;

	Params.m_strStructFiles.clear();
	Params.m_strStructFiles.push_back(vtString((const char *) m_strBuildingFile));

	Params.m_bVehicles = m_bVehicles;
	Params.m_fVehicleSize = m_fVehicleSize;
	Params.m_fVehicleSpeed = m_fVehicleSpeed;
	Params.m_iNumCars = m_iNumCars;

	Params.m_bSky = m_bSky;
	Params.m_bOceanPlane = m_bOceanPlane;
	Params.m_bOverlay = m_bOverlay;
	Params.m_bLabels = m_bLabels;

	Params.m_bPreLight = m_bPreLight;
	Params.m_bPreLit = m_bPreLit;
	Params.m_fPreLightFactor = m_fPreLightFactor;
}


void CCreateDlg::UpdateTiledTextureFilename()
{
	m_strTextureFilename.Format("%s%d.bmp", m_strTextureBase,
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


void CCreateDlg::UpdateMem()
{
	int k = 0;

	k += (m_iTerrainSize * m_iTerrainSize * m_iTerrainDepth) / 1024;	// for ElevationGrid and LocalGrid

	k += (m_iTerrainSize * 8) / 1024;		 // Lookup tables
	switch (m_iLodMethod)
	{
		case LM_ROETTGER:
//			k += SRTerrain::MemoryRequired(m_iTerrainSize);
			break;
		case LM_TOPOVISTA:
			k += TVTerrain::MemoryRequired(m_iTerrainSize);
			break;
		case LM_MCNALLY:
			k += SMTerrain::MemoryRequired(m_iTerrainSize);
			break;
	}
	if (!m_bPreLit)
		k += (m_iTerrainSize * m_iTerrainSize * 12) / 1024;	// Normal vectors array
	k += (m_iTerrainSize * 4);	// Kludge factor

	switch (m_iTexture)
	{
	case 1:
		// single texture
		k += 200;	// overhead
		k += (m_iTextureSize * m_iTextureSize * 2) / 1024;
		break;
	case 2:
		k += 200;	// overhead
		k += (NTILES * NTILES) * (m_iTilesize * m_iTilesize) * 2 / 1024;
		break;
	}

	m_strMemRequired.Format("%3.1f MB + roads&trees", k/1024.0f);
	UpdateData(FALSE);
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

	UpdateMem();
}


void CCreateDlg::DetermineTerrainSizeFromBT()
{
	vtString fname = FindFileOnPaths(g_Options.m_DataPaths, "Elevation/" + m_strFilename);
	if (fname == "")
	{
		m_iTerrainSize = 1024;
		return;
	}

	vtElevationGrid grid;
	if (!grid.LoadBTHeader(fname))
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

	vtString fname = FindFileOnPaths(g_Options.m_DataPaths, "GeoSpecific/" + m_strTextureSingle);
	if (fname == "")
		goto ErrExit;

	fileHandle = OpenFile(fname, &openFileStruct, OF_READ);
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
		UpdateMem();
	}
}

void CCreateDlg::OnSelchangeTfilesingle() 
{
	// always do this first
	UpdateData(TRUE);

	CString str;
	m_cbTextureFileSingle.GetLBText(m_cbTextureFileSingle.GetCurSel(), m_strTextureSingle);
	DetermineSizeFromBMP();
	UpdateMem();
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
