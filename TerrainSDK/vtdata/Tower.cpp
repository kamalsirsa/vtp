//
// Tower.cpp
//
// Implements the vtTower class which represents a single Transmission Tower.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Tower.h"

/////////////////////////////////////

vtTowerArm::vtTowerArm() : Selectable()
{
	m_iArms = 2;
	m_ArmMaterial = TWR_STEEL;
	m_Type = TOWER_TUBEFRAME;
	m_ArmColor.Set(100,100,100);
}

vtTower::vtTower() : Selectable()
{
	m_TowerShape = TSHAPE_CIRCLE;	// default shape
	m_TowerType = TOWER_TUBEFRAME;	// default tower
	m_bElevated = false;			// default placement

	// default colors
	m_BaseColor.Set(100,100,100);	// lt grey

	// size / position
	SetRectangle(10.0f, 10.0f);
	m_fRadius = 1.5f;
	m_fRotation = -1.0f;	// unknown rotation
}

vtTowerArm::~vtTowerArm()
{
	m_iArms=NULL;
	m_ArmColor.Set(100,100,100);
}

/////////////////////////////////////

void vtTowerArm::Set(int iArms, TowerMaterial ArmMaterial, TowerType Type, RGBi color)
{
	m_iArms = iArms;
	m_ArmMaterial= ArmMaterial;
	m_Type = Type;
	m_ArmColor = color;
}

vtTower &vtTower::operator=(const vtTower &v)
{
	m_TowerShape = v.m_TowerShape;
	m_TowerType = v.m_TowerType;
	m_bElevated = v.m_bElevated;

	m_BaseColor = v.m_BaseColor;
	//m_ArmColor = v.m_ArmColor;

	m_EarthPos = v.m_EarthPos;
	m_fRotation = v.m_fRotation;
	m_fWidth = v.m_fWidth;
	m_fDepth = v.m_fDepth;
	m_fRadius = v.m_fRadius;

	m_Footprint.SetSize(v.m_Footprint.GetSize());
	int i;
	for (i = 0; i < v.m_Footprint.GetSize(); i++)
		m_Footprint.SetAt(i, v.m_Footprint.GetAt(i));

	//SetArms(v.GetArms());

	return *this;
}

void vtTower::SetLocation(double utm_x, double utm_y)
{
	m_EarthPos.x = utm_x;
	m_EarthPos.y = utm_y;
}

void vtTower::SetRectangle(float fWidth, float fDepth)
{
	m_fWidth = fWidth;
	m_fDepth = fDepth;
}

//sets colors of the Base Tower
void vtTower::SetColor(RGBi col)
{
	m_BaseColor = col;
}

RGBi vtTower::GetColor()
{
	return m_BaseColor;
}

//sets rotation of the Tower.  (will affect the catenaries!)
void vtTower::SetRotation(float fRadians)
{
	m_fRotation = fRadians;
}

void vtTower::SetFootprint(const DLine2 &dl)
{
	int size_new = dl.GetSize();
	int size_old = m_Footprint.GetSize();

	m_Footprint = dl;
};

void vtTower::SetCenterFromPoly()
{
	DPoint2 p;

	int size = m_Footprint.GetSize();
	for (int i = 0; i < size; i++)
	{
		p += m_Footprint.GetAt(i);
	}
	p *= (1.0f / size);
	SetLocation(p);
}

void vtTower::Offset(const DPoint2 &p)
{
	m_EarthPos += p;

	if (m_TowerShape == TSHAPE_POLY)
	{
		for (int j = 0; j < m_Footprint.GetSize(); j++)
		{
			m_Footprint[j] += p;
		}
	}
}

//
// Get an extent rectangle around the Tower.
// It doesn't need to be exact.
//
DRECT vtTower::GetExtents()
{
	if (m_TowerShape == TSHAPE_RECTANGLE)
	{
		float greater = m_fWidth > m_fDepth ? m_fWidth : m_fDepth;
		return DRECT(m_EarthPos.x - greater, m_EarthPos.y + greater,
					 m_EarthPos.x + greater, m_EarthPos.y - greater);
	}

	if (m_TowerShape == TSHAPE_CIRCLE)
	{
		return DRECT(m_EarthPos.x - m_fRadius, m_EarthPos.y + m_fRadius,
					 m_EarthPos.x + m_fRadius, m_EarthPos.y - m_fRadius);
	}

	if (m_TowerShape == TSHAPE_POLY)
	{
		DRECT ext(1E9, -1E9, -1E9, 1E9);
		DPoint2 temp;

		for (int j = 0; j < m_Footprint.GetSize(); j++)
		{
			temp = m_Footprint[j];

			if (temp.x < ext.left) ext.left = temp.x;
			if (temp.x > ext.right) ext.right = temp.x;
			if (temp.y < ext.bottom) ext.bottom = temp.y;
			if (temp.y > ext.top) ext.top = temp.y;
		}
		return ext;
	}

	return DRECT(m_EarthPos.x, m_EarthPos.y,
				 m_EarthPos.x, m_EarthPos.y);
}


void vtTower::RectToPoly()
{
	if (m_TowerShape != TSHAPE_RECTANGLE)
		return;

	DPoint2 corner[4];

	// if rotation is unset, default to none
	float fRotation = m_fRotation == -1.0f ? 0.0f : m_fRotation;

	DPoint2 pt(m_fWidth / 2.0, m_fDepth / 2.0);
	corner[0].Set(-pt.x, -pt.y);
	corner[1].Set(pt.x, -pt.y);
	corner[2].Set(pt.x, pt.y);
	corner[3].Set(-pt.x, pt.y);
	corner[0].Rotate(fRotation);
	corner[1].Rotate(fRotation);
	corner[2].Rotate(fRotation);
	corner[3].Rotate(fRotation);
	m_Footprint.SetAt(0, m_EarthPos + corner[0]);
	m_Footprint.SetAt(1, m_EarthPos + corner[1]);
	m_Footprint.SetAt(2, m_EarthPos + corner[2]);
	m_Footprint.SetAt(3, m_EarthPos + corner[3]);
}

