//
// Route.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
//////////////////////////////////////////////////////////////////////
// 
// Define a route on a terrain given (at minimum) an inorder list of 
//	2D corner points.
//
// 2001.09.30 JD - cloned from Fence.h
// 2001.10.05 JD - added station points
//
//////////////////////////////////////////////////////////////////////

#ifndef ROUTEH
#define ROUTEH

#include "HeightField.h"
class vtTerrain;
class vtTower3D;

typedef DPoint3 RoutePoint;

class vtStation
{
public:
	long lRoutePointIndex;
	DPoint3 m_dpStationPoint;	// x,y,z in World Coord on the ground
	DPoint2 m_dpOffsetElevations;	// x=Left, y=Right
	DPoint2 m_dpOffsetDistances;	// x=Left, y=Right
	double dRadAzimuth;		// azimuth.
	double dRadLineAngle;	// diff from last azimuth.
	int m_iRoutePointNumber;	// if the station point is also
								//	a route point. -1 otherewise
	vtNode*	m_pTower;		// The station may have a tower placed on it
	char sStructure[30];	// Structure Family
	float m_fTowerHeight;
	int m_iNUmWires;
	FPoint3 m_fpWireAtt[7]; // where are the wires?


//	vtStation();
};


struct PolePlacement
{
	DPoint2 m_Point;
	long m_StationIndex;

};

typedef struct _profileheader
{
	DPoint2 dp2Station0UTM;
	double dStation0Azimuth;
} ProfileHeader;

typedef struct _profilerecord
{
	double dStationNumber;
	double dElevationLeft;
	double dElevationCenter;
	double dElevationRight;
	double OffsetLeft;
	double OffsetRight;
	double LineAngleDeg;
	double LineAngleMin;
	double LineAngleSec;
	char strStructureFamily[80];
} ProfileRecord;


class vtRoute
{
public:
	vtRoute(float fHeight, float fSpacing, float fOffR
		, float fOffL, float fStInc, vtString sName, vtTerrain* pT);

	void add_point(const DPoint2 &epos);
	bool close_route(); // return true if redraw necessary

	void set_post_size(FPoint3 size ) { m_PostSize = size; }

	void save(FILE *fp);
	bool load(FILE *fp);

	void BuildGeometry(vtHeightField *pHeightField);
	void DestroyGeometry();

	vtGeom *GetGeom() { return m_pRouteGeom; }

	static void SetScale(float fScale) { m_fRouteScale = fScale; }
	// Station set readers.
	bool TestReader(FILE* f);		// Read in the output test route file
	bool logReader(FILE* f);
	bool p3DReader(FILE* f);
	bool StructureReader(vtString sPath);
	bool WireReader(vtString s, long st);
	vtStation& operator[](long iter) 
	{
		return (iter < m_StationArray.GetSize()
			? m_StationArray[iter] : m_StationArray[0]); 
	}
	long GetSize() { return m_StationArray.GetSize(); }

protected:
	void add_Pole(FPoint3 &p1, int iMatIdx);	//Copied from Fences

	void add_Pole(DPoint3 &p1, long lStationIndex);	// Mine

	// route size is exaggerated by this amount
	static float m_fRouteScale;

	// all routes share the same set of materials
	static vtMaterialArray *m_pRouteMats;
	static void CreateMaterials();
	static int m_mi_woodpost;
	static int m_mi_wire;
	static int m_mi_metalpost;

	void  AddRouteMeshes(vtHeightField *pHeightField);

	vtLOD		*m_pLOD;
	vtGeom		*m_pRouteGeom;
//	DLine2		m_pRoutePts;	// in earth coordinates
	Array<PolePlacement>  m_aRoutePlaces;
	FPoint3		m_PostSize;
	vtTerrain*	m_pTheTerrain;

	float	m_fHeight;
	float	m_fSpacing;

	bool m_bClosed;
	bool m_bBuilt;

	// "User defined" attributes of the route
	vtString m_sBranchName;
	float m_fOffsetLeft;	// default values overridden at the 
	float m_fOffsetRight;	//	Station
	float m_fStationIncrement;

	// An array of station points
	Array<vtStation> m_StationArray;
	Array<ProfileRecord> m_StationRefArray;	//read-in reference numbers 

	ProfileHeader m_phTheHeader;

	vtString m_sDataPath;

	// Add a station
	bool AddStation(vtHeightField* pHeightField, vtStation &sp);
	// Add a list of stations
	bool AddStations(vtHeightField* pHeightField, RoutePoint rp);
	double isangle(DLine3 L);
	double isangle(DPoint3 P0, DPoint3 P1, DPoint3 P2);
	void StringWires(long lTowerIndex, vtHeightField *pHeightField);	
	void DrawCat(FPoint3 p0, FPoint3 p1, double Azimuth
		, double catenary, double segmentlen, vtMesh* pWireMesh);
};

typedef class vtRoute *vtRoutePtr;

#endif //ROUTEH
