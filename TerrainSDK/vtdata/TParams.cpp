//
// TParams.cpp
//
// Defines all the construction parameters for a terrain.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/FilePath.h"
#include <string.h>
#include "TParams.h"

#include <fstream>
using namespace std;

//---------------------------------------------------------------------------

PointStyle::PointStyle()
{
	m_field_index = 0;
	m_label_elevation = 100.0f;	// 100m above the ground
	m_label_size = 10.0f;		// 10m tall text
	m_label_color.Set(255,255,255);	// white
}

//---------------------------------------------------------------------------

// default (currently fixed) number of tiles
#define NTILES	4

TParams::TParams()
{
	// provide some defaults
	m_fVerticalExag = 1.0f;
	m_iMinHeight = 20;
	m_iNavStyle = 0;
	m_fNavSpeed = 100;
	m_fHither = 5;
	m_bAccel = false;

	m_eLodMethod = LM_ROETTGER;
	m_fPixelError = 2.0f;
	m_iTriCount = 10000;
	m_bTriStrips = true;
	m_bDetailTexture = false;

	m_bTin = false;

	m_bTimeOn = false;
	m_iInitTime = 10;
	m_fTimeSpeed = 1;

	m_eTexture = TE_NONE;
	m_iTilesize = 512;
	m_bJPEG = false;
	m_bMipmap = false;
	m_b16bit = true;
	m_bPreLight = true;
	m_bPreLit = true;
	m_fPreLightFactor = 1.0f;

	m_bRoads = false;
	m_bHwy = true;
	m_bPaved = true;
	m_bDirt = true;
	m_fRoadHeight = 2;		// 2 m
	m_fRoadDistance = 2;	// 2 km
	m_bTexRoads = true;
	m_bRoadCulture = false;

	m_bPlants = false;
	m_iVegDistance = 2000;	// 2 km
	m_bAgriculture = false;
	m_bWildVeg = false;

	m_bFog = false;
	m_fFogDistance = 50;	// 50 km
	m_FogColor.Set(-1, -1, -1); // unset

	m_iStructDistance = 2000;	// 2 km

	m_bTransTowers = false;

	m_bVehicles = false;
	m_fVehicleSize = 1;
	m_fVehicleSpeed = 1;
	m_iNumCars = 0;

	m_bSky = true;
	m_strSkyTexture = "";
	m_bOceanPlane = false;
	m_fOceanPlaneLevel = -20.0f;
	m_bDepressOcean = false;
	m_fDepressOceanLevel = -40.0f;
	m_bHorizon = false;
	m_bOverlay = false;
	m_bLabels = false;
	m_strLabelFile = "";

	m_bRouteEnable = false;
}

TParams::~TParams()
{
}


//
// copy constructor
//
TParams::TParams(const TParams &paramsSrc)
{
	*this = paramsSrc;
}

//
// assignment operator
//
const TParams &TParams::operator = (const TParams &rhs)
{
	m_strName = rhs.m_strName;
	m_strElevFile = rhs.m_strElevFile;
	m_fVerticalExag = rhs.m_fVerticalExag;
	m_iMinHeight = rhs.m_iMinHeight;
	m_iNavStyle = rhs.m_iNavStyle;
	m_fNavSpeed = rhs.m_fNavSpeed;
	m_strLocFile = rhs.m_strLocFile;
	m_strInitLocation = rhs.m_strInitLocation;
	m_fHither = rhs.m_fHither;
	m_bAccel = rhs.m_bAccel;

	m_eLodMethod = rhs.m_eLodMethod;
	m_fPixelError = rhs.m_fPixelError;
	m_iTriCount = rhs.m_iTriCount;
	m_bTriStrips = rhs.m_bTriStrips;
	m_bDetailTexture = rhs.m_bDetailTexture;

	m_bTin = rhs.m_bTin;

	m_bTimeOn = rhs.m_bTimeOn;
	m_iInitTime = rhs.m_iInitTime;
	m_fTimeSpeed = rhs.m_fTimeSpeed;

	m_eTexture = rhs.m_eTexture;
	m_iTilesize = rhs.m_iTilesize;
	m_strTextureSingle = rhs.m_strTextureSingle;
	m_strTextureBase = rhs.m_strTextureBase;
	m_bJPEG = rhs.m_bJPEG;
	m_strTextureFilename = rhs.m_strTextureFilename;
	m_bMipmap = rhs.m_bMipmap;
	m_b16bit = rhs.m_b16bit;

	m_bRoads = rhs.m_bRoads;
	m_strRoadFile = rhs.m_strRoadFile;
	m_bHwy = rhs.m_bHwy;
	m_bPaved = rhs.m_bPaved;
	m_bDirt = rhs.m_bDirt;
	m_fRoadHeight = rhs.m_fRoadHeight;
	m_fRoadDistance = rhs.m_fRoadDistance;
	m_bTexRoads = rhs.m_bTexRoads;
	m_bRoadCulture = rhs.m_bRoadCulture;

	m_bPlants = rhs.m_bPlants;
	m_strVegFile = rhs.m_strVegFile;
	m_iVegDistance = rhs.m_iVegDistance;
	m_bAgriculture = rhs.m_bAgriculture;
	m_bWildVeg = rhs.m_bWildVeg;

	m_bFog = rhs.m_bFog;
	m_fFogDistance = rhs.m_fFogDistance;
	m_FogColor = rhs.m_FogColor;

	m_strStructFiles = rhs.m_strStructFiles;
	m_iStructDistance = rhs.m_iStructDistance;

	m_bTransTowers = rhs.m_bTransTowers;
	m_strTowerFile = rhs.m_strTowerFile;

	m_bVehicles = rhs.m_bVehicles;
	m_fVehicleSize = rhs.m_fVehicleSize;
	m_fVehicleSpeed = rhs.m_fVehicleSpeed;
	m_iNumCars = rhs.m_iNumCars;

	m_bSky = rhs.m_bSky;
	m_strSkyTexture = rhs.m_strSkyTexture;
	m_bOceanPlane = rhs.m_bOceanPlane;
	m_fOceanPlaneLevel = rhs.m_fOceanPlaneLevel;
	m_bDepressOcean = rhs.m_bDepressOcean;
	m_fDepressOceanLevel = rhs.m_fDepressOceanLevel;
	m_bHorizon = rhs.m_bHorizon;
	m_bOverlay = rhs.m_bOverlay;
	m_bLabels = rhs.m_bLabels;
	m_strLabelFile = rhs.m_strLabelFile;
	m_Style = rhs.m_Style;

	m_bPreLight = rhs.m_bPreLight;
	m_bPreLit = rhs.m_bPreLit;
	m_fPreLightFactor = rhs.m_fPreLightFactor;

	m_strRouteFile = rhs.m_strRouteFile;
	m_bRouteEnable = rhs.m_bRouteEnable;

	return *this;
}

#define STR_NAME "Name"
#define STR_ELEVFILE "Filename"
#define STR_LOCFILENAME "Locations_File"
#define STR_VERTICALEXAG "Vertical_Exag"

#define STR_MINHEIGHT "Min_Height"
#define STR_NAVSTYLE "Nav_Style"
#define STR_NAVSPEED "Nav_Speed"
#define STR_LOCFILE "Locations_File"
#define STR_INITLOCATION "Init_Location"
#define STR_HITHER "Hither_Distance"
#define STR_ACCEL "Accel"

#define STR_LODMETHOD "LOD_Method"
#define STR_PIXELERROR "Pixel_Error"
#define STR_TRICOUNT "Tri_Count"
#define STR_TRISTRIPS "Tristrips"
#define STR_DETAILTEXTURE "Detail_Texture"

#define STR_TIN "Is_TIN"

#define STR_TIMEON "Time_On"
#define STR_INITTIME "Init_Time"
#define STR_TIMESPEED "Time_Speed"

#define STR_TEXTURE "Texture"
#define STR_NTILES "Num_Tiles"
#define STR_TILESIZE "Tile_Size"
#define STR_TEXTURESINGLE "Single_Texture"
#define STR_TEXTUREBASE "Base_Texture"
#define STR_TEXTUREFORMAT "Texture_Format"
#define STR_MIPMAP "MIP_Map"
#define STR_16BIT "16_Bit"
#define STR_PRELIGHT "Pre-Light"
#define STR_PRELIT "Pre-Lit"
#define STR_PRELIGHTFACTOR "PreLight_Factor"

#define STR_ROADS "Roads"
#define STR_ROADFILE "Road_File"
#define STR_HWY "Highway"
#define STR_PAVED "Paved"
#define STR_DIRT "Dirt"
#define STR_ROADHEIGHT "Road_Height"
#define STR_ROADDISTANCE "Road_Distance"
#define STR_TEXROADS "Road_Texture"
#define STR_ROADCULTURE "Road_Culture"

#define STR_TREES "Trees"
#define STR_TREEFILE "Tree_File"
#define STR_VEGDISTANCE "Tree_Distance"
#define STR_AGRICULTURE "Agriculture"
#define STR_WILDVEG	"Wild_Vegetation"

#define STR_FOG "Fog"
#define STR_FOGDISTANCE "Fog_Distance"
#define STR_FOGCOLORR "Fog_Color_R"
#define STR_FOGCOLORG "Fog_Color_G"
#define STR_FOGCOLORB "Fog_Color_B"

#define STR_BUILDINGFILE "Building_File"	// for backward compatibility
#define STR_STRUCTFILE	 "Structure_File"
#define STR_STRUCTDIST	 "Structure_Distance"

#define STR_TOWERS "Trans_Towers"
#define	STR_TOWERFILE "Tower File"

#define STR_VEHICLES "Vehicles"
#define STR_VEHICLESIZE "Vehicle_Size"
#define STR_VEHICLESPEED "Vehicle_Speed"
#define STR_NUMCARS "Number_of_Cars"

#define STR_SKY "Sky"
#define STR_SKYTEXTURE "Sky_Texture"
#define STR_OCEANPLANE "Ocean_Plane"
#define STR_OCEANPLANELEVEL "Ocean_Plane_Level"
#define STR_DEPRESSOCEAN "Depress_Ocean"
#define STR_DEPRESSOCEANLEVEL "Depress_Ocean_Level"
#define STR_HORIZON "Horizon"
#define STR_OVERLAY	"Overlay"

#define STR_LABELS "Labels"
#define STR_LABELFILE "LabelFile"
#define STR_LABELFIELD "Label_Field"
#define STR_LABELHEIGHT "Label_Height"
#define STR_LABELSIZE "Label_Size"

#define STR_ROUTEFILE "Route_File"
#define STR_ROUTEENABLE "Route_Enable"

bool TParams::LoadFromFile(const char *filename)
{
	int iDummy;

	ifstream input(filename, ios::in | ios::binary);
	if (!input.is_open())
		return false;

	// read from file
	char buf[80];

	while (!input.eof())
	{
		if (input.peek() == '\n')
			input.ignore();
		input >> buf;

		// data value should been separated by a tab or space
		int next = input.peek();
		if (next != '\t' && next != ' ')
			continue;
		while (input.peek() == '\t' || input.peek() == ' ')
			input.ignore();

		if (strcmp(buf, STR_NAME) == 0)
			m_strName = get_line_from_stream(input);

		// elevation
		else if (strcmp(buf, STR_ELEVFILE) == 0)
			m_strElevFile = get_line_from_stream(input);
		else if (strcmp(buf, STR_VERTICALEXAG) == 0)
			input >> m_fVerticalExag;
		else if (strcmp(buf, STR_TIN) == 0)
			input >> m_bTin;

		// navigation
		else if (strcmp(buf, STR_MINHEIGHT) == 0)
			input >> m_iMinHeight;
		else if (strcmp(buf, STR_NAVSTYLE) == 0)
			input >> m_iNavStyle;
		else if (strcmp(buf, STR_NAVSPEED) == 0)
			input >> m_fNavSpeed;
		else if (strcmp(buf, STR_LOCFILENAME) == 0)
			m_strLocFile = get_line_from_stream(input);
		else if (strcmp(buf, STR_INITLOCATION) == 0)
			m_strInitLocation = get_line_from_stream(input);
		else if (strcmp(buf, STR_HITHER) == 0)
			input >> m_fHither;
		else if (strcmp(buf, STR_ACCEL) == 0)
			input >> m_bAccel;

		// LOD
		else if (strcmp(buf, STR_LODMETHOD) == 0)
		{
			int dummy;
			input >> dummy;
			m_eLodMethod = (enum LodMethodEnum)dummy;
		}
		else if (strcmp(buf, STR_PIXELERROR) == 0)
			input >> m_fPixelError;
		else if (strcmp(buf, STR_TRICOUNT) == 0)
			input >> m_iTriCount;
		else if (strcmp(buf, STR_TRISTRIPS) == 0)
			input >> m_bTriStrips;
		else if (strcmp(buf, STR_DETAILTEXTURE) == 0)
			input >> m_bDetailTexture;

		// time
		else if (strcmp(buf, STR_TIMEON) == 0)
			input >> m_bTimeOn;
		else if (strcmp(buf, STR_INITTIME) == 0)
			input >> m_iInitTime;
		else if (strcmp(buf, STR_TIMESPEED) == 0)
			input >> m_fTimeSpeed;

		// texture
		else if (strcmp(buf, STR_TEXTURE) == 0)
		{
			int dummy;
			input >> dummy;
			m_eTexture = (enum TextureEnum)dummy;
		}
		else if (strcmp(buf, STR_NTILES) == 0)
			input >> iDummy;		// ntiles
		else if (strcmp(buf, STR_TILESIZE) == 0)
			input >> m_iTilesize;
		else if (strcmp(buf, STR_TEXTURESINGLE) == 0)
			m_strTextureSingle = get_line_from_stream(input);
		else if (strcmp(buf, STR_TEXTUREBASE) == 0)
			m_strTextureBase = get_line_from_stream(input);
		else if (strcmp(buf, STR_TEXTUREFORMAT) == 0)
			input >> m_bJPEG;
		else if (strcmp(buf, STR_MIPMAP) == 0)
			input >> m_bMipmap;
		else if (strcmp(buf, STR_16BIT) == 0)
			input >> m_b16bit;
		else if (strcmp(buf, STR_PRELIGHT) == 0)
			input >> m_bPreLight;
		else if (strcmp(buf, STR_PRELIT) == 0)
			input >> m_bPreLit;
		else if (strcmp(buf, STR_PRELIGHTFACTOR) == 0)
			input >> m_fPreLightFactor;

		// culture
		else if (strcmp(buf, STR_ROADS) == 0)
			input >> m_bRoads;
		else if (strcmp(buf, STR_ROADFILE) == 0)
			m_strRoadFile = get_line_from_stream(input);
		else if (strcmp(buf, STR_HWY) == 0)
			input >> m_bHwy;
		else if (strcmp(buf, STR_PAVED) == 0)
			input >> m_bPaved;
		else if (strcmp(buf, STR_DIRT) == 0)
			input >> m_bDirt;
		else if (strcmp(buf, STR_ROADHEIGHT) == 0)
			input >> m_fRoadHeight;
		else if (strcmp(buf, STR_ROADDISTANCE) == 0)
			input >> m_fRoadDistance;
		else if (strcmp(buf, STR_TEXROADS) == 0)
			input >> m_bTexRoads;
		else if (strcmp(buf, STR_ROADCULTURE) == 0)
			input >> m_bRoadCulture;

		else if (strcmp(buf, STR_TREES) == 0)
			input >> m_bPlants;
		else if (strcmp(buf, STR_TREEFILE) == 0)
			m_strVegFile = get_line_from_stream(input);
		else if (strcmp(buf, STR_VEGDISTANCE) == 0)
		{
			input >> m_iVegDistance;
			if (m_iVegDistance < 20)	// surely an old km value
				m_iVegDistance *= 1000;
		}
		else if (strcmp(buf, STR_AGRICULTURE) == 0)
			input >> m_bAgriculture;
		else if (strcmp(buf, STR_WILDVEG) == 0)
			input >> m_bWildVeg;

		else if (strcmp(buf, STR_FOG) == 0)
			input >> m_bFog;
		else if (strcmp(buf, STR_FOGCOLORR) == 0)
			input >> m_FogColor.r;
		else if (strcmp(buf, STR_FOGCOLORG) == 0)
			input >> m_FogColor.g;
		else if (strcmp(buf, STR_FOGCOLORB) == 0)
			input >> m_FogColor.b;
		else if (strcmp(buf, STR_FOGDISTANCE) == 0)
			input >> m_fFogDistance;

		else if (strcmp(buf, STR_BUILDINGFILE) == 0 || strcmp(buf, STR_STRUCTFILE) == 0)
		{
			vtString strFile(get_line_from_stream(input));
			if (strFile != "")
				m_strStructFiles.push_back(strFile);
		}
		else if (strcmp(buf, STR_STRUCTDIST) == 0)
			input >> m_iStructDistance;

		else if (strcmp(buf, STR_SKY) == 0)
			input >> m_bSky;
		else if (strcmp(buf, STR_SKYTEXTURE) == 0)
			m_strSkyTexture = get_line_from_stream(input);
		else if (strcmp(buf, STR_OCEANPLANE) == 0)
			input >> m_bOceanPlane;
		else if (strcmp(buf, STR_OCEANPLANELEVEL) == 0)
			input >> m_fOceanPlaneLevel;
		else if (strcmp(buf, STR_DEPRESSOCEAN) == 0)
			input >> m_bDepressOcean;
		else if (strcmp(buf, STR_DEPRESSOCEANLEVEL) == 0)
			input >> m_fDepressOceanLevel;
		else if (strcmp(buf, STR_HORIZON) == 0)
			input >> m_bHorizon;
		else if (strcmp(buf, STR_OVERLAY) == 0)
			input >> m_bOverlay;

		else if (strcmp(buf, STR_LABELS) == 0)
			input >> m_bLabels;
		else if (strcmp(buf, STR_LABELFILE) == 0)
			m_strLabelFile = get_line_from_stream(input);
		else if (strcmp(buf, STR_LABELFIELD) == 0)
			input >> m_Style.m_field_index;
		else if (strcmp(buf, STR_LABELHEIGHT) == 0)
			input >> m_Style.m_label_elevation;
		else if (strcmp(buf, STR_LABELSIZE) == 0)
			input >> m_Style.m_label_size;

		else if (strcmp(buf, STR_VEHICLES) == 0)
			input >> m_bVehicles;
		else if (strcmp(buf, STR_VEHICLESIZE) == 0)
			input >> m_fVehicleSize;
		else if (strcmp(buf, STR_VEHICLESPEED) == 0)
			input >> m_fVehicleSpeed;
		else if (strcmp(buf, STR_NUMCARS) == 0)
			input >> m_iNumCars;

		else if (strcmp(buf, STR_TOWERS)==0)
			input>>m_bTransTowers;
		else if (strcmp(buf,STR_TOWERFILE)==0)
			m_strTowerFile= get_line_from_stream(input);
		else if (strcmp(buf, STR_ROUTEFILE) == 0)
			 m_strRouteFile = get_line_from_stream(input);
		else if (strcmp(buf, STR_ROUTEENABLE) == 0)
			input >> m_bRouteEnable;
		else
		{
//			cout << "Input from INI file unrecognized.\n";
			get_line_from_stream(input);
		}
	}

	input.close();

	m_strTextureFilename.Format("%s%d", (const char *) m_strTextureBase,
		NTILES * (m_iTilesize-1) + 1);
	if (m_bJPEG)
		m_strTextureFilename += ".jpg";
	else
		m_strTextureFilename += ".bmp";
	return true;
}

bool TParams::SaveToFile(const char *filename)
{
	unsigned int i;

	ofstream output(filename, ios::binary);
	if (!output.is_open())
		return false;

	// write to file
	output << STR_NAME << "\t\t\t";
	output << (const char *) m_strName << endl;

	output << "\n; Elevation\n";
	output << STR_ELEVFILE << "\t\t";
	output << (const char *) m_strElevFile << endl;
	output << STR_VERTICALEXAG << "\t";
	output << m_fVerticalExag << endl;
	output << STR_TIN << "\t\t\t";
	output << m_bTin << endl;

	output << "\n; Navigation\n";
	output << STR_MINHEIGHT << "\t\t";
	output << m_iMinHeight << endl;
	output << STR_NAVSPEED << "\t\t";
	output << m_fNavSpeed << endl;
	output << STR_NAVSTYLE << "\t\t";
	output << m_iNavStyle << endl;
	output << STR_LOCFILENAME << "\t";
	output << (const char *) m_strLocFile << endl;
	output << STR_INITLOCATION << "\t";
	output << (const char *) m_strInitLocation << endl;
	output << STR_HITHER << "\t\t";
	output << m_fHither << endl;
	output << STR_ACCEL << "\t\t";
	output << m_bAccel << endl;

	output << "\n; LOD\n";
	output << STR_LODMETHOD << "\t\t";
	output << m_eLodMethod << endl;
	output << STR_PIXELERROR << "\t\t";
	output << m_fPixelError << endl;
	output << STR_TRICOUNT << "\t\t";
	output << m_iTriCount << endl;
	output << STR_TRISTRIPS << "\t\t";
	output << m_bTriStrips << endl;
	output << STR_DETAILTEXTURE << "\t";
	output << m_bDetailTexture << endl;

	output << "\n; Time\n";
	output << STR_TIMEON << "\t\t\t";
	output << m_bTimeOn << endl;
	output << STR_INITTIME << "\t\t";
	output << m_iInitTime << endl;
	output << STR_TIMESPEED << "\t\t";
	output << m_fTimeSpeed << endl;

	output << "\n; Texture\n";
	output << STR_TEXTURE << "\t\t\t";
	output << m_eTexture << endl;
	output << STR_NTILES << "\t\t";
	output << NTILES << endl;
	output << STR_TILESIZE << "\t\t";
	output << m_iTilesize << endl;
	output << STR_TEXTURESINGLE << "\t";
	output << (const char *) m_strTextureSingle << endl;
	output << STR_TEXTUREBASE << "\t";
	output << (const char *) m_strTextureBase << endl;
	output << STR_TEXTUREFORMAT << "\t";
	output << m_bJPEG << endl;
	output << STR_MIPMAP << "\t\t\t";
	output << m_bMipmap << endl;
	output << STR_16BIT << "\t\t\t";
	output << m_b16bit << endl;
	output << STR_PRELIGHT << "\t\t";
	output << m_bPreLight << endl;
	output << STR_PRELIT << "\t\t\t";
	output << m_bPreLit << endl;
	output << STR_PRELIGHTFACTOR << "\t";
	output << m_fPreLightFactor << endl;

	output << "\n; Roads\n";
	output << STR_ROADS << "\t\t\t";
	output << m_bRoads << endl;
	output << STR_ROADFILE << "\t\t";
	output << (const char *) m_strRoadFile << endl;
	output << STR_HWY << "\t\t\t";
	output << m_bHwy << endl;
	output << STR_PAVED << "\t\t\t";
	output << m_bPaved << endl;
	output << STR_DIRT << "\t\t\t";
	output << m_bDirt << endl;
	output << STR_ROADHEIGHT << "\t\t";
	output << m_fRoadHeight << endl;
	output << STR_ROADDISTANCE << "\t";
	output << m_fRoadDistance << endl;
	output << STR_TEXROADS << "\t";
	output << m_bTexRoads << endl;
	output << STR_ROADCULTURE << "\t";
	output << m_bRoadCulture << endl;

	output << "\n; Vegetation\n";
	output << STR_TREES << "\t\t\t";
	output << m_bPlants << endl;
	output << STR_TREEFILE << "\t\t";
	output << (const char *) m_strVegFile << endl;
	output << STR_VEGDISTANCE << "\t";
	output << m_iVegDistance << endl;
	output << STR_AGRICULTURE << "\t\t";
	output << m_bAgriculture << endl;
	output << STR_WILDVEG << "\t";
	output << m_bWildVeg << endl;

	output << "\n; Fog\n";
	output << STR_FOG << "\t\t\t\t";
	output << m_bFog << endl;
	output << STR_FOGDISTANCE << "\t";
	output << m_fFogDistance << endl;
	if(m_FogColor.r != -1.0f)
	{ // don't pollute old scenes that aren't using this feature yet
		output << STR_FOGCOLORR << "\t\t";
		output << m_FogColor.r << endl;
		output << STR_FOGCOLORG << "\t\t";
		output << m_FogColor.g << endl;
		output << STR_FOGCOLORB << "\t\t";
		output << m_FogColor.b << endl;
	}

	output << "\n; Structures\n";
	for (i = 0; i < m_strStructFiles.size(); i++)
	{
		output << STR_STRUCTFILE << "\t";
		output << (const char *) m_strStructFiles[i] << endl;
	}
	output << STR_STRUCTDIST << "\t";
	output << m_iStructDistance << endl;

	output << "\n";
	output << STR_SKY << "\t\t\t\t";
	output << m_bSky << endl;
	output << STR_SKYTEXTURE << "\t\t";
	output << (const char *) m_strSkyTexture << endl;
	output << "\n";
	output << STR_OCEANPLANE << "\t\t";
	output << m_bOceanPlane << endl;
	output << STR_OCEANPLANELEVEL << "\t";
	output << m_fOceanPlaneLevel << endl;
	output << STR_DEPRESSOCEAN << "\t\t";
	output << m_bDepressOcean << endl;
	output << STR_DEPRESSOCEANLEVEL << "\t";
	output << m_fDepressOceanLevel << endl;
	output << STR_HORIZON << "\t\t\t";
	output << m_bHorizon << endl;
	output << STR_OVERLAY << "\t\t\t";
	output << m_bOverlay << endl;

	output << "\n; Labels\n";
	output << STR_LABELS << "\t\t\t";
	output << m_bLabels << endl;
	output << STR_LABELFILE << "\t\t";
	output << (const char *) m_strLabelFile << endl;
	output << STR_LABELFIELD << "\t\t";
	output << m_Style.m_field_index << endl;
	output << STR_LABELHEIGHT << "\t";
	output << m_Style.m_label_elevation << endl;
	output << STR_LABELSIZE << "\t\t";
	output << m_Style.m_label_size << endl;

	output << "\n";
	output << STR_VEHICLES << "\t\t";
	output << m_bVehicles << endl;
	output << STR_VEHICLESIZE << "\t";
	output << m_fVehicleSize << endl;
	output << STR_VEHICLESPEED << "\t";
	output << m_fVehicleSpeed << endl;
	output << STR_NUMCARS << "\t";
	output << m_iNumCars << endl;

	output << "\n";
	output << STR_TOWERS << "\t";
	output << m_bTransTowers<<endl;
	output << STR_TOWERFILE <<"\t";
	output << (const char *) m_strTowerFile<<endl;

	output << "\n";
	output << STR_ROUTEFILE << "\t\t";
	output << (const char *) m_strRouteFile << endl;
	output << STR_ROUTEENABLE << "\t";
	output << m_bRouteEnable << endl;

	return true;
}

