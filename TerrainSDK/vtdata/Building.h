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

class vtWall
{
public:
	enum WallMaterial {
		UNKNOWN, WOOD, SIDING, GLASS, BRICK, CEMENT, STUCCO
	};
	
	void Set(int iDoors, int iWindows, WallMaterial m_Type);

	WallMaterial m_Type;

	int		m_iDoors;
	int		m_iWindows;
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

	Array<vtWall *> m_Wall;

	int		m_iStories;
	float	m_fStoryHeight;
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

	void SetFootprint(DLine2 &dl);
	DLine2 &GetFootprint() { return m_Footprint; }

	void SetColor(BldColor which, RGBi col);
	RGBi GetColor(BldColor which) const;

	void SetStories(int i);
	int GetStories() const;

	bool GetExtents(DRECT &rect) const;
	void SetCenterFromPoly();
	void Offset(const DPoint2 &delta);
	void RectToPoly(float fWidth, float fDepth, float fRotation);

	void WriteXML(FILE *fp, bool bDegrees);

	void RebuildWalls();

	RoofType	m_RoofType;
	bool		m_bMoulding;
	bool		m_bElevated;

protected:
	//colors
	RGBi		m_Color;			// overall building color
	RGBi		m_RoofColor;		// roof color
	RGBi		m_MouldingColor;	// color of trim

	// information about each story
	Array<vtLevel *> m_Levels;

	// fields that affect placement
	DPoint2		m_EarthPos;			// location of building center

	// footprint (for polygonal buildings)
	DLine2		m_Footprint;

private:
	void DeleteStories();
};

typedef vtBuilding *vtBuildingPtr;


///////////////////////////////////////////////////////////////////////

#endif // BUILDINGH
