//
// Building.h
//
// Implements the vtBuilding class which represents a single built structure.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef BUILDINGH
#define BUILDINGH

#include <stdio.h>
#include "MathTypes.h"

////////////////////////////////////////////////////

enum RoofType
{
	ROOF_FLAT, ROOF_SHED, ROOF_GABLE, ROOF_HIP, NUM_ROOFTYPES
};

enum BldColor
{
	BLD_BASIC,
	BLD_ROOF,
	BLD_MOULDING
};

enum BldMaterial {
	BMAT_UNKNOWN,
	BMAT_PLAIN,
	BMAT_WOOD,
	BMAT_SIDING,
	BMAT_GLASS,
	BMAT_BRICK,
	BMAT_CEMENT,
	BMAT_STUCCO,
	BMAT_CORRUGATED,
	BMAT_DOOR,
	BMAT_WINDOW
};


// Wall feature codes
#define WFC_WALL		1
#define WFC_GAP			2
#define WFC_POST		3
#define WFC_WINDOW		4
#define WFC_DOOR		5

class vtWallFeature
{
public:
	vtWallFeature();
	vtWallFeature(int code, float width = -1.0f, float vf1 = 0.0f, float vf2 = 1.0f);

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

class vtWall
{
public:
	vtWall();
	vtWall(const vtWall &lhs);
	~vtWall();

	void Set(int doors, int windows, BldMaterial material);
	int NumFeatures() const { return m_Features.GetSize(); }
	float FixedFeaturesWidth();
	float ProportionTotal();

	// members
	enum BldMaterial	m_Material;
	Array<vtWallFeature>	m_Features;
};

#define MAX_WALLS	24	// the largest number of walls
						// (largest number of points in a poly-shaped building)

class vtLevel
{
public:
	vtLevel();
	vtLevel(const vtLevel &from) { *this = from; }
	~vtLevel();

	// assignment operator
	vtLevel &operator=(const vtLevel &v);

	void DeleteWalls();
	void SetWalls(int n);
	void SetFootprint(const DLine2 &dl);
	DLine2 &GetFootprint() { return m_Footprint; }

	Array<vtWall *> m_Wall;

	int		m_iStories;
	float	m_fStoryHeight;

	// footprint of the stories in this level
protected:
	DLine2		m_Footprint;
};

class vtBuilding
{
public:
	vtBuilding();
	~vtBuilding();

	// copy
	vtBuilding &operator=(const vtBuilding &v);

	// center of the building
	void SetLocation(double x, double y) { m_EarthPos.Set(x, y); }
	void SetLocation(const DPoint2 &p) { m_EarthPos = p; }
	DPoint2 GetLocation() const { return m_EarthPos; }

	void SetRectangle(float fWidth, float fDepth, float fRotation = 0.0f);
	bool GetRectangle(float &fWidth, float &fDepth) const;

	void SetRadius(float fRad);
	float GetRadius() const;

	void SetFootprint(int i, const DLine2 &dl);
	DLine2 &GetFootprint(int i) { return m_Levels[i]->GetFootprint(); }

	void SetColor(BldColor which, RGBi col);
	RGBi GetColor(BldColor which) const;

	void SetStories(int i);
	int GetStories() const;

	int GetNumLevels() const { return m_Levels.GetSize(); }
	vtLevel *GetLevel(int i) { return m_Levels[i]; }
	void AddLevel(vtLevel *pLev) { m_Levels.Append(pLev); }

	bool GetExtents(DRECT &rect) const;
	void SetCenterFromPoly();
	void Offset(const DPoint2 &delta);
	void RectToPoly(float fWidth, float fDepth, float fRotation);

	void WriteXML(FILE *fp, bool bDegrees);

	RoofType	m_RoofType;
	bool		m_bMoulding;
	bool		m_bElevated;

protected:
	//colors
	RGBi		m_Color;			// overall building color
	RGBi		m_RoofColor;		// roof color
	RGBi		m_MouldingColor;	// color of trim
	BldMaterial	m_RoofMaterial;

	// information about each story
	Array<vtLevel *> m_Levels;

	// fields that affect placement
	DPoint2		m_EarthPos;			// location of building center

private:
	void DeleteStories();
};

typedef vtBuilding *vtBuildingPtr;


///////////////////////////////////////////////////////////////////////

#endif // BUILDINGH
