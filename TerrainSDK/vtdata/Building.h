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

enum WallType
{
	WALL_FLAT, WALL_SIDING, WALL_GLASS
};
enum RoofType
{
	ROOF_FLAT, ROOF_SHED, ROOF_GABLE, ROOF_HIP, NUM_ROOFTYPES
};

enum BldShape
{
	SHAPE_RECTANGLE,
	SHAPE_CIRCLE,
	SHAPE_POLY,
	NUM_BLDSHAPES
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
	void Set(int iDoors, int iWindows, WallType m_Type);

	int		m_iDoors;
	int		m_iWindows;
	WallType m_Type;
};

#define MAX_WALLS	24	// the largest number of walls
						// (largest number of points in a poly-shaped building)

class vtStory
{
public:
	vtStory();
	vtStory(const vtStory &from) { *this = from; }
	~vtStory();

	// assignment operator
	vtStory &operator=(const vtStory &v);

	void DeleteWalls();
	void SetWalls(int n);

	Array<vtWall *> m_Wall;
//	float	m_fHeight;	// TODO
};

class vtBuilding
{
public:
	vtBuilding();
	~vtBuilding();

	// copy
	vtBuilding &operator=(const vtBuilding &v);

	// center of the building
	void SetLocation(double x, double y);
	void SetLocation(const DPoint2 &p) { m_EarthPos = p; }
	DPoint2 GetLocation() const { return m_EarthPos; }

	//depth will be set to the greater of the 2 values.
	void SetRectangle(float fWidth, float fDepth);
	void GetRectangle(float &fWidth, float &fDepth) const
	{
		fWidth = m_fWidth;
		fDepth = m_fDepth;
	}
	void SetRadius(float fRad) { m_fRadius = fRad; }
	float GetRadius() const { return m_fRadius; }

	void SetFootprint(DLine2 &dl);
	DLine2 &GetFootprint() { return m_Footprint; }

	void SetColor(BldColor which, RGBi col);
	RGBi GetColor(BldColor which);

	void SetRotation(float fRadians);
	void GetRotation(float &fRadians) const { fRadians = m_fRotation; }

	void SetShape(BldShape s) { m_BldShape = s; }
	BldShape GetShape() { return m_BldShape; }

	void SetStories(int i);
	int GetStories() const { return m_Story.GetSize(); }

	bool GetExtents(DRECT &rect);
	void SetCenterFromPoly();
	void Offset(const DPoint2 &p);
	void RectToPoly();

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
	Array<vtStory *> m_Story;

	// fields that affect placement
	DPoint2		m_EarthPos;			// location of building center
	float		m_fRotation;		// in Radians

	// fields that affect size
	BldShape	m_BldShape;

	// size of base (for rectanguloid buildings)
	float		m_fWidth, m_fDepth;	// in meters

	// radius (for cylindroid buildings)
	float		m_fRadius;

	// footprint (for polygonal buildings)
	DLine2		m_Footprint;

private:
	void DeleteStories();
};

typedef vtBuilding *vtBuildingPtr;


///////////////////////////////////////////////////////////////////////

#endif // BUILDINGH
