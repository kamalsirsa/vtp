//
// class TParams
//
// defines all the construction parameters for a terrain
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TPARAMSH
#define TPARAMSH

#include <time.h>	// for time_t
#include "MathTypes.h"
#include "vtString.h"

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

class PointStyle
{
public:
	PointStyle();

	int m_field_index;			// index of field for Text to show
	float m_label_elevation;
	float m_label_size;
	RGBi m_label_color;
};


class TParams
{
public:
	TParams();
	~TParams();

	// copy constructor and assignment operator
	TParams(const TParams& paramsSrc);
	const TParams & operator= (const TParams & rhs);

	bool SaveToFile(const char *fname);
	bool LoadFromFile(const char *fname);

	// parameters, starting with terrain name
	vtString	m_strName;

	// elevation
	vtString	m_strElevFile;
	float		m_fVerticalExag;
	bool		m_bTin;

	// camera / navigation
	int			m_iMinHeight;
	int			m_iNavStyle;
	float		m_fNavSpeed;
	vtString	m_strLocFile;
	vtString	m_strInitLocation;
	float		m_fHither;
	bool		m_bAccel;

	// LOD
	LodMethodEnum m_eLodMethod;
	float		m_fPixelError;
	int			m_iTriCount;
	bool		m_bTriStrips;
	bool		m_bDetailTexture;

	// time
	bool		m_bTimeOn;
	unint		m_iInitTime;
	float		m_fTimeSpeed;

	// texture
	TextureEnum	m_eTexture;
	unint		m_iTilesize;
	vtString	m_strTextureSingle;
	vtString	m_strTextureBase;
	vtString	m_strTextureFilename;
	bool		m_bJPEG;
	bool		m_bMipmap;
	bool		m_b16bit;
	bool		m_bPreLight;
	bool		m_bPreLit;
	float		m_fPreLightFactor;

	// culture
	bool		m_bRoads;
	vtString	m_strRoadFile;
	bool		m_bHwy;
	bool		m_bPaved;
	bool		m_bDirt;
	float		m_fRoadHeight;
	float		m_fRoadDistance;
	bool		m_bTexRoads;
	bool		m_bRoadCulture;

	bool		m_bPlants;
	vtString	m_strVegFile;
	unint		m_iVegDistance;
	bool		m_bAgriculture;
	bool		m_bWildVeg;

	bool		m_bFog;
	float		m_fFogDistance;
	RGBf		m_FogColor;

	vtStringArray	m_strStructFiles;
	unint		m_iStructDistance;

	bool		m_bSky;
	vtString	m_strSkyTexture;
	bool		m_bOceanPlane;
	float		m_fOceanPlaneLevel;
	bool		m_bDepressOcean;
	float		m_fDepressOceanLevel;
	bool		m_bHorizon;
	bool		m_bOverlay;

	bool		m_bLabels;
	vtString	m_strLabelFile;
	PointStyle	m_Style;

	bool		m_bVehicles;
	float		m_fVehicleSize;
	float		m_fVehicleSpeed;
	int			m_iNumCars;

	bool		m_bTransTowers;
	vtString	m_strTowerFile;

	vtString	m_strRouteFile;
	bool		m_bRouteEnable;
};

#endif	// TPARAMSH

