//
// Building.h
//
// Implements the vtBuilding class which represents a single built structure.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef BUILDINGH
#define BUILDINGH

#include <stdio.h>
#include "MathTypes.h"
#include "LocalConversion.h"
#include "Structure.h"
#include "ogrsf_frmts.h"

class vtHeightField;
class vtStructureArray;

////////////////////////////////////////////////////

#define MINIMUM_BASEMENT_SIZE 0.5 // Mininum size of an automatically generated basement layer in a building

#define DEFAULT_BUILDING_SIZE 10.0  // Default size for buildings imported from points

enum RoofType
{
	ROOF_FLAT, ROOF_SHED, ROOF_GABLE, ROOF_HIP, ROOF_UNKNOWN, NUM_ROOFTYPES
};

enum BldColor
{
	BLD_BASIC,
	BLD_ROOF,
};

enum BldMaterial {
	BMAT_UNKNOWN,
	BMAT_PLAIN,
	BMAT_WOOD,
	BMAT_SIDING,
	BMAT_GLASS,
	BMAT_BRICK,
	BMAT_PAINTED_BRICK,
	BMAT_ROLLED_ROOFING,
	BMAT_CEMENT,
	BMAT_STUCCO,
	BMAT_CORRUGATED,
	BMAT_DOOR,
	BMAT_WINDOW,
	BMAT_WINDOWWALL,
	TOTAL_BUILDING_MATS
};

// When giving the user a choice of building materials, omit "unknown",
// "door", "window", and "window-wall"
#define EXPOSED_BUILDING_MATS	TOTAL_BUILDING_MATS-4

// Edge feature codes
#define WFC_WALL		1
#define WFC_GAP			2
#define WFC_POST		3
#define WFC_WINDOW		4
#define WFC_DOOR		5

class vtEdgeFeature
{
public:
	vtEdgeFeature();
	vtEdgeFeature(int code, float width = -1.0f, float vf1 = 0.0f, float vf2 = 1.0f);

	void SetDefaults();

	// classification code - door, window, wall etc.
	int		m_code;

	// style (to classify window/door/post style)
	// TODO

	// overall color, or special values:
	// TODO?

	// width: positive values mean meters,
	//	negative values mean proportional scaling factors
	float	m_width;

	// vertical factors: from base, start/end of feature, normalized (0-1)
	float	m_vf1;
	float	m_vf2;
};

class vtEdge
{
public:
	vtEdge();
	vtEdge(const vtEdge &lhs);
	~vtEdge();

	void Set(int doors, int windows, BldMaterial material);
	void AddFeature(int code, float width = -1.0f, float vf1 = 0.0f, float vf2 = 1.0f);
	int NumFeatures() const { return m_Features.GetSize(); }
	int NumFeaturesOfCode(int code);
	float FixedFeaturesWidth();
	float ProportionTotal();

	// color
	RGBi	m_Color;	// overall edge color

	// slope in degrees: 90 is vertical, 0 is horizontal
	int	m_iSlope;

	// eave_length
	float m_fEaveLength;

	// members
	enum BldMaterial	 m_Material;
	Array<vtEdgeFeature> m_Features;
	vtString			 m_Facade;
};

#define MAX_WALLS	360	// the largest number of walls
						// (largest number of points in a poly-shaped building)

class vtLevel
{
	friend class vtBuilding;
public:
	vtLevel();
	vtLevel(const vtLevel &from) { *this = from; }
	~vtLevel();

	// assignment operator
	vtLevel &operator=(const vtLevel &v);

	int GetNumEdges() { return m_Edges.GetSize(); }
	vtEdge *GetEdge(int i) { return m_Edges[i]; }
	float GetEdgeLength(int i);
	BldMaterial GetOverallEdgeMaterial();
	bool GetOverallEdgeColor(RGBi &color);
	RoofType GuessRoofType();
	void FlipFootprintDirection();

	bool HasSlopedEdges();
	bool IsHorizontal();
	bool IsEdgeConvex(int i);
	bool IsCornerConvex(int i);
	bool IsUniform();

	void SetEdgeMaterial(BldMaterial bm);
	void SetEdgeColor(RGBi color);
	void SetRoofType(RoofType rt, int iSlopeDegrees);
	void SetEaveLength(float fMeters);

	int		m_iStories;
	float	m_fStoryHeight;

	Array<vtEdge *> m_Edges;

	void SetFootprint(const DLine2 &dl);
	void SetFootprint(const OGRPolygon *poly);
	DLine2 &GetFootprint() { return m_Footprint; }

	void DetermineLocalFootprint(float fHeight);
	const FLine3 &GetLocalFootprint() { return m_LocalFootprint; }

private:
	void SetWalls(int n);
	void GetEdgePlane(int i, FPlane &plane);
	bool DetermineHeightFromSlopes();
	void DeleteEdges();

	// footprint of the stories in this level
	DLine2		m_Footprint;

	// alternate storage of earth-CS footprint, in development
	OGRPolygon	m_Foot;

	// footprint in the local CS of this building
	FLine3		m_LocalFootprint;
};

class vtBuilding : public vtStructure
{
public:
	vtBuilding();
	~vtBuilding();

	// copy
	vtBuilding &operator=(const vtBuilding &v);

	// footprint methods
	void SetFootprint(int i, const DLine2 &dl);
	DLine2 &GetFootprint(int i) { return m_Levels[i]->GetFootprint(); }
	bool GetBaseLevelCenter(DPoint2 &p) const;

	void SetRectangle(const DPoint2 &center, float fWidth, float fDepth,
		float fRotation = 0.0f);
	void SetCircle(const DPoint2 &center, float fRad);

	void FlipFootprintDirection();
	float CalculateBaseElevation(vtHeightField *pHeightField);
	void TransformCoords(OCT *trans);

	// roof methods
	void SetRoofType(RoofType rt, int iSlope = -1, int iLev = -1);
	RoofType GetRoofType();

	void SetColor(BldColor which, RGBi col);
	RGBi GetColor(BldColor which) const;

	void SetStories(int i);
	int GetStories() const;

	int GetNumLevels() const { return m_Levels.GetSize(); }
	vtLevel *GetLevel(int i) { return m_Levels[i]; }
	vtLevel *CreateLevel(const DLine2 &footprint);
	vtLevel *CreateLevel();
	void InsertLevel(int iLev, vtLevel *pLev);
	void DeleteLevel(int iLev);

	bool GetExtents(DRECT &rect) const;
	void Offset(const DPoint2 &delta);
	double GetDistanceToInterior(const DPoint2 &point) const;

	void WriteXML(FILE *fp, bool bDegrees);
	void WriteXML_Old(FILE *fp, bool bDegrees);
	void AddDefaultDetails();
	void DetermineLocalFootprints();
	const FLine3 &GetLocalFootprint(int i) { return m_Levels[i]->GetLocalFootprint(); }

	static vtLocalConversion s_Conv;
	static const char *GetMaterialString(BldMaterial mat);
	static BldMaterial GetMaterialValue(const char *value);
	static const char *GetEdgeFeatureString(int edgetype);
	static int		   GetEdgeFeatureValue(const char *value);

	bool IsContainedBy(const DRECT &rect) const;
	void SwapLevels(int lev1, int lev2);

protected:
	// information about each story
	Array<vtLevel *> m_Levels;

private:
	void DeleteStories();
};

typedef vtBuilding *vtBuildingPtr;

///////////////////////////////////////////////////////////////////////

#endif // BUILDINGH

