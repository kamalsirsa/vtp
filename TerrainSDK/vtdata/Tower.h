//
// Tower.h
//
// Implements the vtTower class which represents a single Tansmission Tower.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TOWER_H
#define TOWER_H

#include "MathTypes.h"
#include "Selectable.h"

enum TowerType
{
	TOWER_LATTICE, TOWER_TUBEFRAME, TOWER_HFRAME
};

enum TowerShape // used for footprints and as a base for tower types
{
	TSHAPE_CIRCLE=0,
	TSHAPE_RECTANGLE,
	TSHAPE_POLY,
	NUM_TOWERSHAPES
};

enum TowerMaterial
{
	TWR_WOOD,
	TWR_STEEL
};

enum TowerColor // may not really need but leave there til object model complete
{
	TWR_BASE,
	TWR_ARM
};

class vtTowerArm: public Selectable
{
public:
	vtTowerArm();
	~vtTowerArm();
	
	void Set(int iArms, TowerMaterial ArmMaterial, TowerType Type, RGBi color );

	TowerMaterial m_ArmMaterial;
	int			m_iArms;	// number of arms
	TowerType	m_Type;		// need to possibly create a enum arm type when we know the different types
	RGBi		m_ArmColor;	// used for stress analysis
	
};

#define MAX_ARMS	6	// the largest number of hanging arms
						

class vtTower : public Selectable
{
public:
	vtTower();
	//~vtTower();

	// copy
	vtTower &operator=(const vtTower &v);

	// center of the Tower
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

	void SetTowerType(TowerType t){m_TowerType=t;}
	TowerType GetTowerType(){return m_TowerType;}

	void SetTowerMaterial(TowerMaterial m){m_TowerMaterial = m;}
	TowerMaterial GetTowerMaterial(){ return m_TowerMaterial;}

	void SetFootprint(DLine2 &dl);
	DLine2 &GetFootprint() { return m_Footprint; }

	void SetColor(RGBi col);
	RGBi GetColor();

	void SetRotation(float fRadians);
	void GetRotation(float &fRadians) const { fRadians = m_fRotation; }

	void SetShape(TowerShape s) { m_TowerShape = s; }
	TowerShape GetShape() { return m_TowerShape; }

	DRECT GetExtents();
	void SetCenterFromPoly();
	void Offset(DPoint2 &p);
	void RectToPoly();

	// used to reference which model of a certain type is used
	int m_iTowerTypeIndex;

	TowerType	m_TowerType;
	TowerMaterial m_TowerMaterial;
	bool		m_bElevated;// for elevated foundations... probably not used

protected:
	//colors
	RGBi		m_BaseColor;		// overall base tower color

	// fields that affect placement
	DPoint2		m_EarthPos;			// location of building center
	float		m_fRotation;		// in Radians

	// fields that affect size
	TowerShape	m_TowerShape;

	// size of base (for rectanguloid towers)
	float		m_fWidth, m_fDepth;	// in meters

	// radius (for cylindroid towers)
	float		m_fRadius;

	// footprint (for polygonal towers)
	DLine2		m_Footprint;
};

typedef vtTower *vtTowerPtr;

#endif
