//
// Elastic.cpp
//
// Copyright (c) 2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Terrain.h"
#include "Elastic.h"

ElasticPolyline::ElasticPolyline()
{
	m_Materials = new vtMaterialArray;
	m_Materials->AddRGBMaterial1(RGBf(1, 0.5, 0), true, true);			// orange solid
	m_Materials->AddRGBMaterial1(RGBf(1, 1, 0.5), false, false, true);	// light yellow wireframe

	// Create a marker post to use for each corner of the polyline
	int matidx = 0;		// orange
	float fRadius = 0.1f;
	m_Marker = CreateCylinderGeom(m_Materials, matidx, VT_Normals, 1.0f,
		fRadius, 10, true, false, false, 1);

	m_Container = new vtGroup;
	m_pTerr = NULL;

	// Defaults
	m_fPostHeight = 1.0f;
	m_fLineHeight = 1.0f;
}

void ElasticPolyline::SetTerrain(vtTerrain *pTerr)
{
	// Remove from any other terrain, add to this one
	if (m_Container->getNumParents() != 0)
		m_Container->getParent(0)->removeChild(m_Container.get());

	m_pTerr = pTerr;
	m_pTerr->GetTopGroup()->addChild(m_Container);
}

void ElasticPolyline::SetLineColor(const RGBAf &color)
{
	m_Materials->at(1)->SetDiffuse1(color);
}

void ElasticPolyline::SetPolyline(const DLine2 &line)
{
	m_Line = line;
	Realize();
}

void ElasticPolyline::AddPoint(const DPoint2 &point)
{
	m_Line.Append(point);
	Realize();
}

void ElasticPolyline::SetPoint(int iPoint, const DPoint2 &point)
{
	m_Line.SetAt(iPoint, point);
	Realize();
}

void ElasticPolyline::Clear()
{
	m_Line.Empty();
	Realize();
}

void ElasticPolyline::Realize()
{
	if (!m_pTerr)	// safety check
		return;

	// Clear previous geometry
	m_Container->removeChildren(0, m_Container->getNumChildren());

	// Make markers
	for (unsigned int i = 0; i < m_Line.GetSize(); i++)
	{
		vtTransform *xform = new vtTransform;
		m_pTerr->PlantModelAtPoint(xform, m_Line[i]);
		xform->Scale3(1.0f, m_fPostHeight, 1.0f);

		m_Container->addChild(xform);
		xform->addChild(m_Marker);
	}

	// Make lines between them
	vtGeode *LineGeode = new vtGeode;
	LineGeode->SetMaterials(m_Materials);
	m_Container->addChild(LineGeode);
	int matidx = 1;		// yellow
	vtGeomFactory mf(LineGeode, osg::PrimitiveSet::LINE_STRIP, 0, 30000, matidx);
	m_pTerr->AddSurfaceLineToMesh(&mf, m_Line, m_fLineHeight, true);
}

