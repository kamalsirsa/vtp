//
// class TParams
//
// defines all the construction parameters for a terrain
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TPARAMSH
#define TPARAMSH

#include "MathTypes.h"
#include "Content.h"

typedef unsigned int unint;

enum TextureEnum {
	TE_NONE,
	TE_SINGLE,
	TE_TILED,
	TE_DERIVED
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

	// Combine the parameters to produce the texture name to use
	vtString CookTextureFilename() const;

	// Convenience method
	void SetOverlay(const vtString &fname, int x, int y);
	bool GetOverlay(vtString &fname, int &x, int &y) const;

public:
	// this must be a public member (currently..)
	std::vector<vtTagArray> m_Layers;

private:
	bool LoadFromIniFile(const char *fname);
	void ConvertOldTimeValue();
};


////////////////////////////////////////////////////////////////////////
// Visitor class for XML parsing of TParams files.

class TParamsVisitor : public TagVisitor
{
public:
	TParamsVisitor(TParams *pParams) : TagVisitor(pParams), m_pParams(pParams) {}
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement (const char *name);

protected:
	TParams *m_pParams;
	vtTagArray m_layer;
	bool m_bViz;
};


#define STR_TPARAMS_FORMAT_NAME "Terrain_Parameters"

#define STR_NAME "Name"
#define STR_ELEVFILE "Filename"
#define STR_VERTICALEXAG "Vertical_Exag"
#define STR_SUPPRESS "Suppress_Surface"

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
#define STR_REQUEST16BIT "Request_16_Bit"
#define STR_PRELIGHT "Pre-Light"
#define STR_PRELIGHTFACTOR "PreLight_Factor"
#define STR_CAST_SHADOWS "Cast_Shadows"
#define STR_COLOR_MAP "Color_Map"

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

#endif	// TPARAMSH

