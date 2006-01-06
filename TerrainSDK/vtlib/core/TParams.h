//
// class TParams
//
// defines all the construction parameters for a terrain
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TPARAMSH
#define TPARAMSH

#include "vtdata/MathTypes.h"
#include "vtdata/Content.h"

/** \addtogroup terrain */
/*@{*/

typedef unsigned int unint;

enum TextureEnum {
	TE_NONE,
	TE_SINGLE,
	TE_TILED,
	TE_DERIVED,
	TE_TILESET
};

enum LodMethodEnum {
	LM_ROETTGER,
	LM_TOPOVISTA,
	LM_MCNALLY,
	LM_DEMETER,
	LM_CUSTOM,
	LM_BRYANQUAD
};

// TParam Layer Types
#define TERR_LTYPE_STRUCTURE	"Structure"
#define TERR_LTYPE_ABSTRACT		"Abstract"

////////////////////////////////////////////////////////////////////////
// Class to encapsulate a scenarios parameters.

class ScenarioParams : public vtTagArray
{
public:
	ScenarioParams();

	// copy constructor and assignment operator
	ScenarioParams(const ScenarioParams& paramsSrc);
	ScenarioParams &ScenarioParams::operator = (const ScenarioParams &rhs);

	// override to catch active layers list
	void WriteOverridesToXML(FILE *fp) const;

	vtStringArray& GetActiveLayers() { return m_ActiveLayers; }

protected:
	vtStringArray m_ActiveLayers;
};

/**
 * Terrain Parameters.  These are all the values which describe how a terrain
 *  will be created.
 *
 * This class inherits from vtTagArray, a simple collection of tags with
 *  values, which provides the methods to set and get values.  A standard
 *  set of tags which TParams stores is listed below.  Related user
 *  documentation for these parameters as they are displayed in the Enviro
 *  user interface is at http://vterrain.org/Doc/Enviro/terrain_creation.html
 *
 * \par Example of working with parameters:
 \code
 TParams params;
 int height = params.GetValueInt("Min_Height");
 params.SetValueInt("Min_Height", 100);
 \endcode
 *
 * \par The standard terrain parameter tags:
 *
<table border="1" cellpadding="4" cellspacing="0" bordercolor="#111111" style="border-collapse: collapse; vertical-align:top">
<tr>
	<th>Tag</th>
	<th>Type</th>
	<th>Default</th>
	<th>Description</th>
</tr>
<tr>
	<td>Name</td>
	<td>String</td>
	<td></td>
	<td>The name of the terrain.  Example: "Big Island of Hawai'i".</td>
</tr>
<tr>
	<td>Elevation_Filename</td>
	<td>String</td>
	<td></td>
	<td>Filename for source of elevation data.</td>
</tr>
<tr>
	<td>Vertical_Exag</td>
	<td>Float</td>
	<td>1.0</td>
	<td>Vertical exaggeration, 1.0 to draw the terrain surface at its true elevation.</td>
</tr>
<tr>
	<td>Suppress_Surface</td>
	<td>Bool</td>
	<td>false</td>
	<td>True to not draw the terrain surface at all.  It will be loaded, and used
	for purposes such as planting objects, but not drawn.</td>
</tr>
<tr>
	<td>Min_Height</td>
	<td>Float</td>
	<td>20</td>
	<td>For navigation, minimum height above the ground in meters.</td>
</tr>
<tr>
	<td>Nav_Style</td>
	<td>Int</td>
	<td>0</td>
	<td>For navigation, tells the viewer what stlye to use: 0=Normal, 1=Velocity, 2=Grab-Pivot</td>
</tr>
<tr>
	<td>Nav_Speed</td>
	<td>Float</td>
	<td>100</td>
	<td>For navigation, peak speed in meters per second.</td>
</tr>
<tr>
	<td>Locations_File</td>
	<td>String</td>
	<td></td>
	<td>File which contains the stored camera locations for this terrain.</td>
</tr>
<tr>
	<td>Init_Location</td>
	<td>String</td>
	<td></td>
	<td>Name of the location at which the camera should start when the terrain
	is first visited.</td>
</tr>
<tr>
	<td>Hither_Distance</td>
	<td>String</td>
	<td>5</td>
	<td>The hither ("near") clipping plan distance, in meters.</td>
</tr>
<tr>
	<td>Accel</td>
	<td>Bool</td>
	<td>false</td>
	<td>True means to accelerate navigation by the camera's height above the
	terrain surface.</td>
</tr>
<tr>
	<td>Allow_Roll</td>
	<td>Bool</td>
	<td>false</td>
	<td>True to allow the camera to roll (in addition to pitch and yaw).</td>
</tr>
<tr>
	<td>Surface_Type</td>
	<td>Int</td>
	<td>0</td>
	<td>The type of input elevation data. 0=single grid, 1=TIN, 2=Tileset</td>
</tr>
<tr>
	<td>LOD_Method</td>
	<td>Int</td>
	<td>0</td>
	<td>The type of CLOD to use for single elevation grids (Surface_Type=0).
		See LodMethodEnum for values.</td>
</tr>
<tr>
	<td>Tri_Count</td>
	<td>Int</td>
	<td>10000</td>
	<td>The number of triangles for the CLOD algorithm to target.</td>
</tr>
<tr>
	<td>Tristrips</td>
	<td>Bool</td>
	<td>true</td>
	<td>For the McNally CLOD, True to use triangle strips.</td>
</tr>
<tr>
	<td>Vert_Count</td>
	<td>Int</td>
	<td>20000</td>
	<td>For tiled terrain (Surface_Type=2), the number of vertices for the
	algorithm to target.</td>
</tr>
<tr>
	<td>Tile_Cache_Size</td>
	<td>Int</td>
	<td>80</td>
	<td>For tiled terrain (Surface_Type=2), the size of the tile cache to
	keep in host RAM, in MB.</td>
</tr>
</table>

Remaining to be documented in the table: 

Time_On
Init_Time
Time_Speed
Texture	// 0=none, 1=single, 2=tile4x4, 3=derived, 4=tileset
Tile_Size
Texture_Filename
Base_Texture
Texture_Format	// 0=bmp, 1=jpg
MIP_Map
Request_16_Bit
Pre-Light
PreLight_Factor
Cast_Shadows
Color_Map
Texture_Retain
Detail_Texture
DTexture_Name
DTexture_Scale
DTexture_Distance
Roads
Road_File
Highway
Paved
Dirt
Road_Height
Road_Distance
Road_Texture
Road_Culture
Trees
Tree_File
Tree_Distance
Fog
Fog_Distance
Fog_Color
Structure_File
Structure_Distance
Structure_Shadows
Shadow_Resolution
Shadow_Darkness
Content_File
Vehicles
Vehicle_Size
Vehicle_Speed
Sky
Sky_Texture
Ocean_Plane
Ocean_Plane_Level
Depress_Ocean
Depress_Ocean_Level
Horizon
Background_Color
Distance_Tool_Height
HUD_Overlay
Scenario_Name
	*/
class TParams : public vtTagArray
{
public:
	TParams();

	// copy constructor and assignment operator
	TParams(const TParams& paramsSrc);
	TParams &TParams::operator = (const TParams &rhs);

	// File IO
	bool LoadFrom(const char *fname);
	bool LoadFromXML(const char *fname);

	// override to catch structure files
	void WriteOverridesToXML(FILE *fp) const;

	void SetLodMethod(LodMethodEnum method);
	LodMethodEnum GetLodMethod() const;

	void SetTextureEnum(TextureEnum tex);
	TextureEnum GetTextureEnum() const;

	// Convenience method
	void SetOverlay(const vtString &fname, int x, int y);
	bool GetOverlay(vtString &fname, int &x, int &y) const;

public:
	// this must be a public member (currently..)
	std::vector<vtTagArray> m_Layers;
	vtStringArray m_AnimPaths;
	std::vector<ScenarioParams> m_Scenarios;

private:
	void ConvertOldTimeValue();
};

////////////////////////////////////////////////////////////////////////
// Visitor class for XML parsing of TParams files.

class TParamsVisitor : public TagVisitor
{
public:
	TParamsVisitor(TParams *pParams) : TagVisitor(pParams), m_pParams(pParams), m_bInLayer(false), m_bInScenario(false) {}
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement (const char *name);

protected:
	TParams *m_pParams;
	vtTagArray m_layer;
	bool m_bViz;
	ScenarioParams m_Scenario;
	bool m_bInLayer;
	bool m_bInScenario;
};


#define STR_TPARAMS_FORMAT_NAME "Terrain_Parameters"

#define STR_NAME "Name"
#define STR_ELEVFILE "Elevation_Filename"
#define STR_VERTICALEXAG "Vertical_Exag"
#define STR_SUPPRESS "Suppress_Surface"

#define STR_MINHEIGHT "Min_Height"
#define STR_NAVSTYLE "Nav_Style"
#define STR_NAVSPEED "Nav_Speed"
#define STR_LOCFILE "Locations_File"
#define STR_INITLOCATION "Init_Location"
#define STR_HITHER "Hither_Distance"
#define STR_ACCEL "Accel"
#define STR_ALLOW_ROLL "Allow_Roll"

#define STR_SURFACE_TYPE "Surface_Type"	// 0=grid, 1=TIN, 2=tiled grid
#define STR_LODMETHOD "LOD_Method"
#define STR_TRICOUNT "Tri_Count"
#define STR_TRISTRIPS "Tristrips"
#define STR_VERTCOUNT "Vert_Count"
#define STR_TILE_CACHE_SIZE "Tile_Cache_Size"	// in MB

#define STR_TIMEON "Time_On"
#define STR_INITTIME "Init_Time"
#define STR_TIMESPEED "Time_Speed"

#define STR_TEXTURE "Texture"	// 0=none, 1=single, 2=tile4x4, 3=derived, 4=tileset
#define STR_TILESIZE "Tile_Size"
#define STR_TEXTUREFILE "Texture_Filename"
#define STR_TEXTUREBASE "Base_Texture"
#define STR_TEXTURE4BY4 "Texture_4by4"
//#define STR_TEXTUREFORMAT "Texture_Format"	// 0=bmp, 1=jpg; obsolete
#define STR_MIPMAP "MIP_Map"
#define STR_REQUEST16BIT "Request_16_Bit"
#define STR_PRELIGHT "Pre-Light"
#define STR_PRELIGHTFACTOR "PreLight_Factor"
#define STR_CAST_SHADOWS "Cast_Shadows"
#define STR_COLOR_MAP "Color_Map"
#define STR_TEXTURE_RETAIN "Texture_Retain"

#define STR_DETAILTEXTURE "Detail_Texture"
#define STR_DTEXTURE_NAME "DTexture_Name"
#define STR_DTEXTURE_SCALE "DTexture_Scale"
#define STR_DTEXTURE_DISTANCE "DTexture_Distance"

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

#define STR_FOG "Fog"
#define STR_FOGDISTANCE "Fog_Distance"
#define STR_FOGCOLOR "Fog_Color"

#define STR_STRUCTFILE	 "Structure_File"
#define STR_STRUCTDIST	 "Structure_Distance"
#define STR_STRUCT_SHADOWS "Structure_Shadows"
#define STR_SHADOW_REZ	"Shadow_Resolution"
#define STR_SHADOW_DARKNESS "Shadow_Darkness"
#define STR_CONTENT_FILE "Content_File"

#define STR_TOWERS "Trans_Towers"
#define	STR_TOWERFILE "Tower_File"

#define STR_VEHICLES "Vehicles"
#define STR_VEHICLESIZE "Vehicle_Size"
#define STR_VEHICLESPEED "Vehicle_Speed"

#define STR_SKY "Sky"
#define STR_SKYTEXTURE "Sky_Texture"
#define STR_OCEANPLANE "Ocean_Plane"
#define STR_OCEANPLANELEVEL "Ocean_Plane_Level"
#define STR_DEPRESSOCEAN "Depress_Ocean"
#define STR_DEPRESSOCEANLEVEL "Depress_Ocean_Level"
#define STR_HORIZON "Horizon"
#define STR_BGCOLOR "Background_Color"

#define STR_ROUTEENABLE "Route_Enable"	// not used yet
#define STR_ROUTEFILE "Route_File"		// not used yet

#define STR_DIST_TOOL_HEIGHT "Distance_Tool_Height"
#define STR_HUD_OVERLAY "HUD_Overlay"

#define STR_SCENARIO_NAME "Scenario_Name"

/*@}*/	// Group terrain

#endif	// TPARAMSH

