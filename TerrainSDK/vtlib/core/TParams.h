//
// class TParams
//
// defines all the construction parameters for a terrain
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TPARAMSH
#define TPARAMSH

#include <time.h>	// for time_t

typedef unsigned int unint;

enum TextureEnum {
	TE_NONE,
	TE_SINGLE,
	TE_TILED,
	TE_DERIVED
};

enum LodMethodEnum {
//	LM_LINDSTROMKOLLER,
	LM_ROETTGER,
	LM_TOPOVISTA,
	LM_MCNALLY,
	LM_CUSTOM,
	LM_BRYANQUAD
};


class TParams
{
public:
	TParams();

	// copy constructor and assignment operator
	TParams(const TParams& paramsSrc);
	const TParams & operator= (const TParams & rhs);

	bool SaveToFile(const char *fname);
	bool LoadFromFile(const char *fname);

	// params
	vtString	m_strName;
	vtString	m_strElevFile;
	float		m_fVerticalExag;
	int			m_iMinHeight;
	float		m_fNavSpeed;
	vtString	m_strLocFile;

	bool		m_bRegular;
	unint		m_iSubsample;

	bool		m_bDynamic;
	LodMethodEnum m_eLodMethod;
	float		m_fPixelError;
	int			m_iTriCount;
	bool		m_bTriStrips;
	bool		m_bDetailTexture;

	bool		m_bTin;

	bool		m_bTimeOn;
	unint		m_iInitTime;
	float		m_fTimeSpeed;

	TextureEnum	m_eTexture;
	unint		m_iTilesize;
	vtString	m_strTextureSingle;
	vtString	m_strTextureBase;
	vtString	m_strTextureFilename;
	bool		m_bMipmap;
	bool		m_b16bit;
	bool		m_bPreLight;
	bool		m_bPreLit;
	float		m_fPreLightFactor;

	bool		m_bRoads;
	vtString	m_strRoadFile;
	bool		m_bHwy;
	bool		m_bPaved;
	bool		m_bDirt;
	float		m_fRoadHeight;
	float		m_fRoadDistance;
	bool		m_bTexRoads;
	bool		m_bRoadCulture;

	bool		m_bTrees;
	vtString	m_strTreeFile;
	unint		m_iTreeDistance;
	bool		m_bAgriculture;
	bool		m_bWildVeg;

	bool		m_bFog;
	int			m_iFogDistance;

	bool		m_bBuildings;
	vtString	m_strBuildingFile;

	bool		m_bTransTowers;
	vtString	m_strTowerFile;

	bool		m_bVehicles;
	float		m_fVehicleSize;
	float		m_fVehicleSpeed;
	int			m_iNumCars;

	bool		m_bSky;
	vtString	m_strSkyTexture;
	bool		m_bOceanPlane;
	float		m_fOceanPlaneLevel;
	bool		m_bDepressOcean;
	float		m_fDepressOceanLevel;
	bool		m_bHorizon;
	bool		m_bVertexColors;
	bool		m_bOverlay;
	bool		m_bSuppressLand;
	bool		m_bLabels;

	bool		m_bAirports;
	vtString	m_strRouteFile;
	bool		m_bRouteEnable;
};

#endif

