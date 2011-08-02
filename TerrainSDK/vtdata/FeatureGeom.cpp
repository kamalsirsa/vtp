//
// Features.cpp
//
// Copyright (c) 2002-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Features.h"
#include "xmlhelper/easyxml.hpp"
#include "PolyChecker.h"
#include "vtLog.h"
#include "DLG.h"


/////////////////////////////////////////////////////////////////////////////
// vtFeatureSetPoint2D
//

vtFeatureSetPoint2D::vtFeatureSetPoint2D() : vtFeatureSet()
{
	m_eGeomType = wkbPoint;
}

unsigned int vtFeatureSetPoint2D::GetNumEntities() const
{
	return m_Point2.GetSize();
}

void vtFeatureSetPoint2D::SetNumGeometries(int iNum)
{
	m_Point2.SetSize(iNum);
}

void vtFeatureSetPoint2D::Reserve(int iNum)
{
	m_Point2.SetMaxSize(iNum);
}

bool vtFeatureSetPoint2D::ComputeExtent(DRECT &rect) const
{
	int i, entities = GetNumEntities();

	if (!entities)
		return false;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);
	for (i = 0; i < entities; i++)
		rect.GrowToContainPoint(m_Point2[i]);

	return true;
}

void vtFeatureSetPoint2D::Offset(const DPoint2 &p, bool bSelectedOnly)
{
	for (unsigned int i = 0; i < m_Point2.GetSize(); i++)
	{
		if (bSelectedOnly && !IsSelected(i))
			continue;
		m_Point2[i] += p;
	}
}

bool vtFeatureSetPoint2D::TransformCoords(OCT *pTransform, bool progress_callback(int))
{
	unsigned int i, bad = 0, size = m_Point2.GetSize();
	for (i = 0; i < size; i++)
	{
		if (progress_callback != NULL && (i%200)==0)
			progress_callback(i * 99 / size);

		int success = pTransform->Transform(1, &m_Point2[i].x, &m_Point2[i].y);
		if (success != 1)
			bad++;
	}
	if (bad)
		VTLOG("Warning: %d of %d coordinates did not transform correctly.\n", bad, size);
	return (bad == 0);
}

bool vtFeatureSetPoint2D::AppendGeometryFrom(vtFeatureSet *pFromSet)
{
	vtFeatureSetPoint2D *pFrom = dynamic_cast<vtFeatureSetPoint2D*>(pFromSet);
	if (!pFrom)
		return false;

	for (unsigned int i = 0; i < pFrom->GetNumEntities(); i++)
		m_Point2.Append(pFrom->m_Point2[i]);
	return true;
}

int vtFeatureSetPoint2D::AddPoint(const DPoint2 &p)
{
	int rec = m_Point2.Append(p);
	AddRecord();
	return rec;
}

void vtFeatureSetPoint2D::SetPoint(unsigned int num, const DPoint2 &p)
{
	if (m_eGeomType == wkbPoint)
		m_Point2.SetAt(num, p);
}

void vtFeatureSetPoint2D::GetPoint(unsigned int num, DPoint2 &p) const
{
	p = m_Point2.GetAt(num);
}

int vtFeatureSetPoint2D::FindClosestPoint(const DPoint2 &p, double epsilon)
{
	int entities = GetNumEntities();
	double dist, closest = 1E9;
	int found = -1;
	DPoint2 diff;

	int i;
	for (i = 0; i < entities; i++)
	{
		diff = p - m_Point2.GetAt(i);
/*		if (m_eGeomType == wkbPoint25D)
		{
			DPoint3 p3 = m_Point3.GetAt(i);
			diff.x = p.x - p3.x;
			diff.y = p.y - p3.y;
		} */
		dist = diff.Length();
		if (dist < closest && dist < epsilon)
		{
			closest = dist;
			found = i;
		}
	}
	return found;
}

void vtFeatureSetPoint2D::FindAllPointsAtLocation(const DPoint2 &loc, vtArray<int> &found)
{
	int entities = GetNumEntities();

	int i;
	for (i = 0; i < entities; i++)
	{
		if (loc == m_Point2.GetAt(i))
			found.Append(i);

	/*	if (m_eGeomType == wkbPoint25D)
		{
			DPoint3 p3 = m_Point3.GetAt(i);
			if (loc.x == p3.x && loc.y == p3.y)
				found.Append(i);
		} */
	}
}

bool vtFeatureSetPoint2D::IsInsideRect(int iElem, const DRECT &rect)
{
	return rect.ContainsPoint(m_Point2[iElem]);
}

void vtFeatureSetPoint2D::CopyGeometry(unsigned int from, unsigned int to)
{
	m_Point2[to] = m_Point2[from];
}

void vtFeatureSetPoint2D::SaveGeomToSHP(SHPHandle hSHP, bool progress_callback(int)) const
{
	unsigned int size = m_Point2.GetSize();
	for (unsigned int i = 0; i < size; i++)
	{
		if (progress_callback && ((i%16)==0))
			progress_callback(i*100/size);

		// Save to SHP
		DPoint2 p2 = m_Point2[i];
		SHPObject *obj = SHPCreateSimpleObject(SHPT_POINT, 1, &p2.x, &p2.y, NULL);
		SHPWriteObject(hSHP, -1, obj);
		SHPDestroyObject(obj);
	}
}

void vtFeatureSetPoint2D::LoadGeomFromSHP(SHPHandle hSHP, bool progress_callback(int))
{
	VTLOG(" vtFeatureSetPoint2D::LoadGeomFromSHP\n");

	int nElems;
	SHPGetInfo(hSHP, &nElems, NULL, NULL, NULL);

	m_Point2.SetSize(nElems);

	// Read Data from SHP into memory
	DPoint2 p2;
	for (int i = 0; i < nElems; i++)
	{
		if (progress_callback && ((i%16)==0))
			progress_callback(i*100/nElems);

		// Get the i-th Shape in the SHP file
		SHPObject *pObj = SHPReadObject(hSHP, i);

		// Beware: it is possible for the shape to not actually have vertices
		if (pObj->nVertices == 0)
			p2.Set(0,0);
		else
			p2.Set(*pObj->padfX, *pObj->padfY);
		m_Point2.SetAt(i, p2);

		SHPDestroyObject(pObj);
	}
}


/////////////////////////////////////////////////////////////////////////////
// vtFeatureSetPoint3D
//

vtFeatureSetPoint3D::vtFeatureSetPoint3D() : vtFeatureSet()
{
	m_eGeomType = wkbPoint25D;
}

unsigned int vtFeatureSetPoint3D::GetNumEntities() const
{
	return m_Point3.GetSize();
}

void vtFeatureSetPoint3D::SetNumGeometries(int iNum)
{
	m_Point3.SetSize(iNum);
}

void vtFeatureSetPoint3D::Reserve(int iNum)
{
	m_Point3.SetMaxSize(iNum);
}

bool vtFeatureSetPoint3D::ComputeExtent(DRECT &rect) const
{
	int i, entities = GetNumEntities();

	if (!entities)
		return false;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);
	DPoint2 p;
	for (i = 0; i < entities; i++)
	{
		p.Set(m_Point3[i].x, m_Point3[i].y);
		rect.GrowToContainPoint(p);
	}

	return true;
}

void vtFeatureSetPoint3D::Offset(const DPoint2 &p, bool bSelectedOnly)
{
	for (unsigned int i = 0; i < m_Point3.GetSize(); i++)
	{
		if (bSelectedOnly && !IsSelected(i))
			continue;
		m_Point3[i] += DPoint3(p.x, p.y, 0);
	}
}

bool vtFeatureSetPoint3D::TransformCoords(OCT *pTransform, bool progress_callback(int))
{
	unsigned int i, bad = 0, size = m_Point3.GetSize();
	for (i = 0; i < size; i++)
	{
		if (progress_callback != NULL && (i%200)==0)
			progress_callback(i * 99 / size);

		int success = pTransform->Transform(1, &m_Point3[i].x, &m_Point3[i].y);
		if (success != 1)
			bad++;
	}
	if (bad)
		VTLOG("Warning: %d of %d coordinates did not transform correctly.\n", bad, size);
	return (bad == 0);
}

bool vtFeatureSetPoint3D::AppendGeometryFrom(vtFeatureSet *pFromSet)
{
	vtFeatureSetPoint3D *pFrom = dynamic_cast<vtFeatureSetPoint3D*>(pFromSet);
	if (!pFrom)
		return false;

	for (unsigned int i = 0; i < pFrom->GetNumEntities(); i++)
		m_Point3.Append(pFrom->m_Point3[i]);
	return true;
}

int vtFeatureSetPoint3D::AddPoint(const DPoint3 &p)
{
	int rec = m_Point3.Append(p);
	AddRecord();
	return rec;
}

void vtFeatureSetPoint3D::SetPoint(unsigned int num, const DPoint3 &p)
{
	m_Point3.SetAt(num, p);
}

void vtFeatureSetPoint3D::GetPoint(unsigned int num, DPoint3 &p) const
{
	p = m_Point3.GetAt(num);
}

bool vtFeatureSetPoint3D::ComputeHeightRange(float &fmin, float &fmax)
{
	unsigned int count = m_Point3.GetSize();
	if (!count)
		return false;

	fmin = 1E9;
	fmax = -1E9;
	for (unsigned int i = 0; i < count; i++)
	{
		if ((float)m_Point3[i].z > fmax) fmax = (float)m_Point3[i].z;
		if ((float)m_Point3[i].z < fmin) fmin = (float)m_Point3[i].z;
	}
	return true;
}

bool vtFeatureSetPoint3D::IsInsideRect(int iElem, const DRECT &rect)
{
	return rect.ContainsPoint(DPoint2(m_Point3[iElem].x, m_Point3[iElem].y));
}

void vtFeatureSetPoint3D::CopyGeometry(unsigned int from, unsigned int to)
{
	m_Point3[to] = m_Point3[from];
}

void vtFeatureSetPoint3D::SaveGeomToSHP(SHPHandle hSHP, bool progress_callback(int)) const
{
	unsigned int size = m_Point3.GetSize();
	for (unsigned int i = 0; i < size; i++)
	{
		if (progress_callback && ((i%16)==0))
			progress_callback(i*100/size);

		// Save to SHP
		DPoint3 p3 = m_Point3[i];
		SHPObject *obj = SHPCreateSimpleObject(SHPT_POINTZ, 1, &p3.x, &p3.y, &p3.z);
		SHPWriteObject(hSHP, -1, obj);
		SHPDestroyObject(obj);
	}
}

void vtFeatureSetPoint3D::LoadGeomFromSHP(SHPHandle hSHP, bool progress_callback(int))
{
	VTLOG(" vtFeatureSetPoint3D::LoadGeomFromSHP\n");

	int nElems;
	SHPGetInfo(hSHP, &nElems, NULL, NULL, NULL);

	m_Point3.SetSize(nElems);

	// Read Data from SHP into memory
	DPoint3 p3;
	for (int i = 0; i < nElems; i++)
	{
		if (progress_callback && ((i%16)==0))
			progress_callback(i*100/nElems);

		// Get the i-th Shape in the SHP file
		SHPObject *pObj = SHPReadObject(hSHP, i);

		// Beware: it is possible for the shape to not actually have vertices
		if (pObj->nVertices == 0)
			p3.Set(0,0,0);
		else
			p3.Set(*pObj->padfX, *pObj->padfY, *pObj->padfZ);
		m_Point3.SetAt(i, p3);
		SHPDestroyObject(pObj);
	}
}


/////////////////////////////////////////////////////////////////////////////
// vtFeatureSetLineString
//

vtFeatureSetLineString::vtFeatureSetLineString() : vtFeatureSet()
{
	m_eGeomType = wkbLineString;
}

unsigned int vtFeatureSetLineString::GetNumEntities() const
{
	return m_Line.size();
}

void vtFeatureSetLineString::SetNumGeometries(int iNum)
{
	m_Line.resize(iNum);
}

void vtFeatureSetLineString::Reserve(int iNum)
{
	m_Line.reserve(iNum);
}

bool vtFeatureSetLineString::ComputeExtent(DRECT &rect) const
{
	int i, entities = GetNumEntities();

	if (!entities)
		return false;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);
	for (i = 0; i < entities; i++)
		rect.GrowToContainLine(m_Line[i]);

	return true;
}

void vtFeatureSetLineString::Offset(const DPoint2 &p, bool bSelectedOnly)
{
	for (unsigned int i = 0; i < m_Line.size(); i++)
	{
		if (bSelectedOnly && !IsSelected(i))
			continue;
		m_Line[i].Add(p);
	}
}

bool vtFeatureSetLineString::TransformCoords(OCT *pTransform, bool progress_callback(int))
{
	unsigned int i, j, pts, bad = 0, size = m_Line.size();
	for (i = 0; i < size; i++)
	{
		if (progress_callback != NULL && (i%200)==0)
			progress_callback(i * 99 / size);

		DLine2 &dline = m_Line[i];
		pts = dline.GetSize();
		for (j = 0; j < pts; j++)
		{
			DPoint2 &p = dline.GetAt(j);
			int success = pTransform->Transform(1, &p.x, &p.y);
			if (success != 1)
				bad++;
		}
	}
	if (bad)
		VTLOG("Warning: %d of %d coordinates did not transform correctly.\n", bad, size);
	return (bad == 0);
}

bool vtFeatureSetLineString::AppendGeometryFrom(vtFeatureSet *pFromSet)
{
	vtFeatureSetLineString *pFrom = dynamic_cast<vtFeatureSetLineString*>(pFromSet);
	if (!pFrom)
		return false;

	for (unsigned int i = 0; i < pFrom->GetNumEntities(); i++)
		m_Line.push_back(pFrom->m_Line[i]);
	return true;
}

int vtFeatureSetLineString::AddPolyLine(const DLine2 &pl)
{
	int rec = m_Line.size();
	m_Line.push_back(pl);
	AddRecord();
	return rec;
}

int vtFeatureSetLineString::NumTotalVertices() const
{
	int total = 0;
	for (unsigned int i = 0; i < m_Line.size(); i++)
		total += m_Line[i].GetSize();
	return total;
}

bool vtFeatureSetLineString::IsInsideRect(int iElem, const DRECT &rect)
{
	return rect.ContainsLine(m_Line[iElem]);
}

void vtFeatureSetLineString::CopyGeometry(unsigned int from, unsigned int to)
{
	// copy geometry
	m_Line[to] = m_Line[from];
}

void vtFeatureSetLineString::SaveGeomToSHP(SHPHandle hSHP, bool progress_callback(int)) const
{
	unsigned int i, j, size = m_Line.size();
	for (i = 0; i < size; i++)
	{
		if (progress_callback && ((i%16)==0))
			progress_callback(i*100/size);

		const DLine2 &dl = m_Line[i];
		double* dX = new double[dl.GetSize()];
		double* dY = new double[dl.GetSize()];

		for (j = 0; j < dl.GetSize(); j++) //for each vertex
		{
			DPoint2 pt = dl.GetAt(j);
			dX[j] = pt.x;
			dY[j] = pt.y;

		}
		// Save to SHP
		SHPObject *obj = SHPCreateSimpleObject(SHPT_ARC, dl.GetSize(),
			dX, dY, NULL);

		delete dX;
		delete dY;

		SHPWriteObject(hSHP, -1, obj);
		SHPDestroyObject(obj);
	}
}

void vtFeatureSetLineString::LoadGeomFromSHP(SHPHandle hSHP, bool progress_callback(int))
{
	VTLOG(" vtFeatureSetLineString::LoadGeomFromSHP\n");

	int nElems;
	SHPGetInfo(hSHP, &nElems, NULL, NULL, NULL);

	m_Line.reserve(nElems);

	// Read Data from SHP into memory
	for (int i = 0; i < nElems; i++)
	{
		if (progress_callback && ((i%16)==0))
			progress_callback(i*100/nElems);

		DLine2 dline;

		// Get the i-th Shape in the SHP file
		SHPObject *pObj = SHPReadObject(hSHP, i);

		// Beware: it is possible for the shape to not actually have vertices
		if (pObj->nVertices == 0)
			m_Line.push_back(dline);
		else
		{
			// Copy each part
			for (int part = 0; part < pObj->nParts; part++)
			{
				int start = pObj->panPartStart[part], end;
				if (part+1 < pObj->nParts)
					end = pObj->panPartStart[part+1]-1;
				else
					end = pObj->nVertices-1;

				dline.SetSize(end - start + 1);
				for (int j = start; j <= end; j++)
					dline.SetAt(j-start, DPoint2(pObj->padfX[j], pObj->padfY[j]));

				m_Line.push_back(dline);
			}
		}
		SHPDestroyObject(pObj);
	}
}


/////////////////////////////////////////////////////////////////////////////
// vtFeatureSetLineString
//

vtFeatureSetLineString3D::vtFeatureSetLineString3D() : vtFeatureSet()
{
	m_eGeomType = wkbLineString25D;
}

unsigned int vtFeatureSetLineString3D::GetNumEntities() const
{
	return m_Line.size();
}

void vtFeatureSetLineString3D::SetNumGeometries(int iNum)
{
	m_Line.resize(iNum);
}

void vtFeatureSetLineString3D::Reserve(int iNum)
{
	m_Line.reserve(iNum);
}

bool vtFeatureSetLineString3D::ComputeExtent(DRECT &rect) const
{
	int i, entities = GetNumEntities();

	if (!entities)
		return false;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);
	for (i = 0; i < entities; i++)
		rect.GrowToContainLine(m_Line[i]);

	return true;
}

void vtFeatureSetLineString3D::Offset(const DPoint2 &p, bool bSelectedOnly)
{
	for (unsigned int i = 0; i < m_Line.size(); i++)
	{
		if (bSelectedOnly && !IsSelected(i))
			continue;
		m_Line[i].Add(p);
	}
}

bool vtFeatureSetLineString3D::TransformCoords(OCT *pTransform, bool progress_callback(int))
{
	unsigned int i, j, pts, bad = 0, size = m_Line.size();
	for (i = 0; i < size; i++)
	{
		if (progress_callback != NULL && (i%200)==0)
			progress_callback(i * 99 / size);

		DLine3 &dline = m_Line[i];
		pts = dline.GetSize();
		for (j = 0; j < pts; j++)
		{
			DPoint3 &p = dline.GetAt(j);
			int success = pTransform->Transform(1, &p.x, &p.y);
			if (success != 1)
				bad++;
		}
	}
	if (bad)
		VTLOG("Warning: %d of %d coordinates did not transform correctly.\n", bad, size);
	return (bad == 0);
}

bool vtFeatureSetLineString3D::AppendGeometryFrom(vtFeatureSet *pFromSet)
{
	vtFeatureSetLineString3D *pFrom = dynamic_cast<vtFeatureSetLineString3D*>(pFromSet);
	if (!pFrom)
		return false;

	for (unsigned int i = 0; i < pFrom->GetNumEntities(); i++)
		m_Line.push_back(pFrom->m_Line[i]);
	return true;
}

int vtFeatureSetLineString3D::AddPolyLine(const DLine3 &pl)
{
	int rec = m_Line.size();
	m_Line.push_back(pl);
	AddRecord();
	return rec;
}

bool vtFeatureSetLineString3D::ComputeHeightRange(float &fmin, float &fmax)
{
	unsigned int count = m_Line.size();
	if (!count)
		return false;

	fmin = 1E9;
	fmax = -1E9;
	for (unsigned int i = 0; i < count; i++)
	{
		const DLine3 &dl = m_Line[i];
		int num = dl.GetSize();
		for (int j = 0; j < num; j++)
		{
			DPoint3 &p3 = dl.GetAt(j);
			if ((float)p3.z > fmax) fmax = (float)p3.z;
			if ((float)p3.z < fmin) fmin = (float)p3.z;
		}
	}
	return true;
}

int vtFeatureSetLineString3D::NumTotalVertices() const
{
	int total = 0;
	for (unsigned int i = 0; i < m_Line.size(); i++)
		total += m_Line[i].GetSize();
	return total;
}

/**
 For a given 2D point, find the linear feature closest to it (horizontally),
 and the closest point on that feature. Return true if a feature was found.
 */
bool vtFeatureSetLineString3D::FindClosest(const DPoint2 &p, int &close_feature, DPoint3 &close_point)
{
	close_feature = -1;
	close_point.Set(0,0,0);

	double dist, closest_dist = 1E9;
	int point_index;
	DPoint3 intersection;
	for (unsigned int i = 0; i < m_Line.size(); i++)
	{
		if (m_Line[i].NearestSegment2D(p, point_index, dist, intersection))
		{
			if (dist < closest_dist)
			{
				closest_dist = dist;
				close_feature = i;
				close_point = intersection;
			}
		}
	}
	return (close_feature != -1);
}

bool vtFeatureSetLineString3D::IsInsideRect(int iElem, const DRECT &rect)
{
	return rect.ContainsLine(m_Line[iElem]);
}

void vtFeatureSetLineString3D::CopyGeometry(unsigned int from, unsigned int to)
{
	// copy geometry
	m_Line[to] = m_Line[from];
}

void vtFeatureSetLineString3D::SaveGeomToSHP(SHPHandle hSHP, bool progress_callback(int)) const
{
	unsigned int i, j, size = m_Line.size();
	for (i = 0; i < size; i++)
	{
		if (progress_callback && ((i%16)==0))
			progress_callback(i*100/size);

		const DLine3 &dl = m_Line[i];
		double* dX = new double[dl.GetSize()];
		double* dY = new double[dl.GetSize()];
		double* dZ = new double[dl.GetSize()];

		for (j = 0; j < dl.GetSize(); j++) //for each vertex
		{
			DPoint3 pt = dl.GetAt(j);
			dX[j] = pt.x;
			dY[j] = pt.y;
			dZ[j] = pt.z;
		}
		// Save to SHP
		SHPObject *obj = SHPCreateSimpleObject(SHPT_ARCZ, dl.GetSize(),
			dX, dY, dZ);

		delete dX;
		delete dY;
		delete dZ;

		SHPWriteObject(hSHP, -1, obj);
		SHPDestroyObject(obj);
	}
}

void vtFeatureSetLineString3D::LoadGeomFromSHP(SHPHandle hSHP, bool progress_callback(int))
{
	VTLOG(" vtFeatureSetLineString3D::LoadGeomFromSHP\n");

	int nElems;
	SHPGetInfo(hSHP, &nElems, NULL, NULL, NULL);

	m_Line.reserve(nElems);

	// Read Data from SHP into memory
	for (int i = 0; i < nElems; i++)
	{
		if (progress_callback && ((i%16)==0))
			progress_callback(i*100/nElems);

		DLine3 dline;

		// Get the i-th Shape in the SHP file
		SHPObject *pObj = SHPReadObject(hSHP, i);

		// Beware: it is possible for the shape to not actually have vertices
		if (pObj->nVertices == 0)
			m_Line[i] = dline;
		else
		{
			// Store each coordinate
			dline.SetSize(pObj->nVertices);
			for (int j = 0; j < pObj->nVertices; j++)
				dline.SetAt(j, DPoint3(pObj->padfX[j], pObj->padfY[j], pObj->padfZ[j]));

			m_Line.push_back(dline);
		}
		SHPDestroyObject(pObj);
	}
}


/////////////////////////////////////////////////////////////////////////////
// vtFeatureSetPolygon
//

vtFeatureSetPolygon::vtFeatureSetPolygon() : vtFeatureSet()
{
	m_eGeomType = wkbPolygon;
	m_pIndex = NULL;
}

unsigned int vtFeatureSetPolygon::GetNumEntities() const
{
	return m_Poly.size();
}

void vtFeatureSetPolygon::SetNumGeometries(int iNum)
{
	m_Poly.resize(iNum);
}

void vtFeatureSetPolygon::Reserve(int iNum)
{
	m_Poly.reserve(iNum);
}

bool vtFeatureSetPolygon::ComputeExtent(DRECT &rect) const
{
	int i, entities = GetNumEntities();

	if (!entities)
		return false;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);
	for (i = 0; i < entities; i++)
	{
		// we only test the first, outer ring since it contains the rest
		const DPolygon2 &poly = m_Poly[i];
		int num_rings = poly.size();
		if (num_rings < 1)
			continue;
		const DLine2 &dline = poly[0];
		rect.GrowToContainLine(dline);
	}
	return true;
}

void vtFeatureSetPolygon::Offset(const DPoint2 &p, bool bSelectedOnly)
{
	for (unsigned int i = 0; i < m_Poly.size(); i++)
	{
		if (bSelectedOnly && !IsSelected(i))
			continue;
		m_Poly[i].Add(p);
	}
}

bool vtFeatureSetPolygon::TransformCoords(OCT *pTransform, bool progress_callback(int))
{
	unsigned int i, j, k, pts, bad = 0, size = m_Poly.size();
	for (i = 0; i < size; i++)
	{
		if (progress_callback != NULL && (i%200)==0)
			progress_callback(i * 99 / size);

		DPolygon2 &dpoly = m_Poly[i];
		for (j = 0; j < dpoly.size(); j++)
		{
			DLine2 &dline = dpoly[j];
			pts = dline.GetSize();
			for (k = 0; k < pts; k++)
			{
				DPoint2 &p = dline.GetAt(k);
				int success = pTransform->Transform(1, &p.x, &p.y);
				if (success != 1)
					bad++;
			}
		}
	}
	if (bad)
		VTLOG("Warning: %d of %d coordinates did not transform correctly.\n", bad, size);
	return (bad == 0);
}

bool vtFeatureSetPolygon::AppendGeometryFrom(vtFeatureSet *pFromSet)
{
	vtFeatureSetPolygon *pFrom = dynamic_cast<vtFeatureSetPolygon*>(pFromSet);
	if (!pFrom)
		return false;

	for (unsigned int i = 0; i < pFrom->GetNumEntities(); i++)
	{
		switch (m_eGeomType) {
		case wkbPolygon:
		case wkbMultiPolygon:
		m_Poly.push_back(pFrom->m_Poly[i]);
			break;
		default:
			break;
		}
	}
	return true;
}

int vtFeatureSetPolygon::AddPolygon(const DPolygon2 &poly)
{
	int rec = m_Poly.size();
	m_Poly.push_back(poly);
	AddRecord();
	return rec;
}

SpatialIndex::SpatialIndex(int iSize)
{
	m_iLastFound = -1;
	m_iSize = iSize;

	m_pArray = new IntVectorPtr [m_iSize];
	for (int i = 0; i < m_iSize; i++)
	{
		m_pArray[i] = new IntVector [m_iSize];
	}
}

SpatialIndex::~SpatialIndex()
{
	for (int i = 0; i < m_iSize; i++)
	{
		delete [] m_pArray[i];
	}
	delete [] m_pArray;
}

void SpatialIndex::GenerateIndices(const class vtFeatureSetPolygon *feat)
{
	feat->ComputeExtent(m_Extent);

	// Increase extents slightly to avoid numerical edge conditions
	m_Extent.Grow(0.001, 0.001);

	// A tightly optimized index grid would place each feature exactly into
	//  the index cells it overlaps.  That's a rather sophisticated operation
	//  that i don't have code handy to do, so instead we place features into
	//  the index using their extents.

	m_base.Set(m_Extent.left, m_Extent.bottom);
	m_step.Set(m_Extent.Width() / m_iSize, m_Extent.Height() / m_iSize);
	DRECT ext;
	int x1, x2, y1, y2;
	unsigned int e;
	int i, j;

	for (e = 0; e < feat->GetNumEntities(); e++)
	{
		const DPolygon2 &poly = feat->GetPolygon(e);
		poly.ComputeExtents(ext);
		x1 = (int) ((ext.left	- m_base.x) / m_step.x);
		x2 = (int) ((ext.right	- m_base.x) / m_step.x);
		y1 = (int) ((ext.bottom	- m_base.y) / m_step.y);
		y2 = (int) ((ext.top	- m_base.y) / m_step.y);
		for (i = x1; i <= x2; i++)
		{
			for (j = y1; j <= y2; j++)
			{
				IntVectorPtr ptr = m_pArray[i];
				ptr[j].push_back(e);
			}
		}
	}
}

const IntVector *SpatialIndex::GetIndexForPoint(const DPoint2 &p) const
{
	int x = (int) ((p.x	- m_base.x) / m_step.x);
	int y = (int) ((p.y	- m_base.y) / m_step.y);
	if (x < 0 || x >= m_iSize || y < 0 || y >= m_iSize)
		return NULL;

	IntVectorPtr ptr = m_pArray[x];
	return &(ptr[y]);
}

/**
 * Find the first polygon in this feature set which contains the given
 * point.
 *
 * The index of the polygon is return, or -1 if no polygon was found.
 */
int vtFeatureSetPolygon::FindPolygon(const DPoint2 &p) const
{
	unsigned int num, i;

	if (m_pIndex != NULL)
	{
		// use Index
		if (m_pIndex->m_iLastFound != -1)	// try last successful result
		{
			if (m_Poly[m_pIndex->m_iLastFound].ContainsPoint(p))
				return m_pIndex->m_iLastFound;		// found
		}
		const IntVector *index = m_pIndex->GetIndexForPoint(p);
		if (index)
		{
			num = index->size();
			for (i = 0; i < num; i++)
			{
				int e = index->at(i);
				if (m_Poly[e].ContainsPoint(p))
				{
					m_pIndex->m_iLastFound = e;
					return e;		// found
				}
			}
			m_pIndex->m_iLastFound = -1;
		}
	}
	else
	{
		num = m_Poly.size();
		for (i = 0; i < num; i++)
		{
			if (m_Poly[i].ContainsPoint(p))
				return i;		// found
		}
	}
	return -1;	// not found
}

/*
 Fix geometry: Remove redundant (coincident) points, remove colinear points,
  fix the winding direction of polygonal rings.
 */
int vtFeatureSetPolygon::FixGeometry(double dEpsilon)
{
	PolyChecker PolyChecker;

	int removed = 0;
	int num = m_Poly.size();

	for (int i = 0; i < num; i++)
	{
		DPolygon2 &dpoly = m_Poly[i];

		// Remove bad points: degenerate (coincident) and colinear
		removed += dpoly.RemoveDegeneratePoints(dEpsilon);
		removed += dpoly.RemoveColinearPoints(dEpsilon);

		DLine2 &outer = dpoly[0];
		if (PolyChecker.IsClockwisePolygon(outer) == false)
		{
			// Incorrect winding
			outer.ReverseOrder();
		}
		// Check clockwisdom (winding direction)
		for (size_t r = 1; r < dpoly.size(); r++)
		{
			DLine2 &inner = dpoly[r];
			if (PolyChecker.IsClockwisePolygon(inner) == true)
			{
				// Incorrect winding
				inner.ReverseOrder();
			}

		}
	}
	// potential TODO: remove too-small rings (with less than 3 points)
	return removed;
}

/**
 To detect coincident vertices, even between rings, we compare every vertex to
 every other.  It is too troublesome to try to automatically correct these
 points, so intead we just select them to let the user know which are bad.
 */
int vtFeatureSetPolygon::SelectBadFeatures(double dEpsilon)
{
	DeselectAll();
	int num_bad = 0;

	int num_features = m_Poly.size();
	for (int f = 0; f < num_features; f++)
	{
		DPolygon2 &dpoly = m_Poly[f];

		// Concatenate all the points into a single set
		DLine2 dline;
		for (size_t r = 0; r < dpoly.size(); r++)
			dline.Append(dpoly[r]);

		bool bGood = true;
		// A naive N^2 comparison should be fine, as the number of points won't
		//  be huge.
		int num = dline.GetSize();
		for (int i = 0; i < num-1; i++)
		{
			for (int j = i+1; j < num; j++)
			{
				double dist = (dline[j] - dline[i]).Length();
				if (dist < dEpsilon)
				{
					bGood = false;
					num_bad++;
					break;
				}
			}
		}
		if (!bGood)
			Select(f);
	}
	return num_bad;
}

void vtFeatureSetPolygon::CreateIndex(int iSize)
{
	m_pIndex = new SpatialIndex(iSize);
	m_pIndex->GenerateIndices(this);
}

void vtFeatureSetPolygon::FreeIndex()
{
	delete m_pIndex;
	m_pIndex = NULL;
}


/**
 * Find the first polygon in this feature set which contains the given
 * point.  This method makes the simplification assumption that none of
 * the polygons have holes.
 *
 * The index of the polygon is return, or -1 if no polygon was found.
 */
int vtFeatureSetPolygon::FindSimplePolygon(const DPoint2 &p) const
{
	int num = m_Poly.size();
	for (int i = 0; i < num; i++)
	{
		// look only at first ring
		const DLine2 &dline = (m_Poly[i])[0];
		if (dline.ContainsPoint(p))
		{
			// found
			return i;
		}
	}
	// not found
	return -1;
}

bool vtFeatureSetPolygon::IsInsideRect(int iElem, const DRECT &rect)
{
	// only test first, exterior ring
	const DPolygon2 &dpoly = m_Poly[iElem];

	// beware null polygons
	if (dpoly.size() == 0)
		return false;
	return rect.ContainsLine(dpoly[0]);
}

void vtFeatureSetPolygon::CopyGeometry(unsigned int from, unsigned int to)
{
	// copy geometry
	m_Poly[to] = m_Poly[from];
}

void vtFeatureSetPolygon::SaveGeomToSHP(SHPHandle hSHP, bool progress_callback(int)) const
{
	unsigned int i, j, size = m_Poly.size();
	int part;
	for (i = 0; i < size; i++)		// for each polyline
	{
		if (progress_callback && ((i%16)==0))
			progress_callback(i*100/size);

		const DPolygon2 &poly = m_Poly[i];

		int parts = poly.size();

		SHPObject *obj;

		// Beware: it is possible for the shape to not actually have vertices
		if (parts == 1 && poly[0].GetSize() == 0)
		{
			obj = SHPCreateObject(SHPT_POLYGON, -1, parts, NULL,
				NULL, 0, NULL, NULL, NULL, NULL );
			SHPWriteObject(hSHP, -1, obj);
			SHPDestroyObject(obj);
		}
		else
		{
			// count total vertices in all parts
			int total = 0;
			for (part = 0; part < parts; part++)
			{
				total += poly[part].GetSize();
				total++;	// duplicate first vertex
			}

			double *dX = new double[total];
			double *dY = new double[total];
			int *panPartStart = new int[parts];

			int vert = 0;
			for (part = 0; part < parts; part++)
			{
				panPartStart[part] = vert;

				const DLine2 &dl = poly[part];
				for (j=0; j < dl.GetSize(); j++) //for each vertex
				{
					DPoint2 pt = dl.GetAt(j);
					dX[vert] = pt.x;
					dY[vert] = pt.y;
					vert++;
				}
				// duplicate first vertex, it's just what SHP files do.
				DPoint2 pt = dl.GetAt(0);
				dX[vert] = pt.x;
				dY[vert] = pt.y;
				vert++;
			}

			// Save to SHP
			obj = SHPCreateObject(SHPT_POLYGON, -1, parts, panPartStart,
				NULL, total, dX, dY, NULL, NULL );
			SHPWriteObject(hSHP, -1, obj);
			SHPDestroyObject(obj);

			delete [] panPartStart;
			delete [] dY;
			delete [] dX;
		}
	}
}

void vtFeatureSetPolygon::LoadGeomFromSHP(SHPHandle hSHP, bool progress_callback(int))
{
	VTLOG(" vtFeatureSetPolygon::LoadGeomFromSHP\n");

	int nElems;
	SHPGetInfo(hSHP, &nElems, NULL, NULL, NULL);

	m_Poly.reserve(nElems);

	// Read Data from SHP into memory
	int iFailed = 0;
	for (int i = 0; i < nElems; i++)
	{
		if (progress_callback && ((i%16)==0))
			progress_callback(i*100/nElems);

		// Get the i-th Shape in the SHP file
		SHPObject *pObj = SHPReadObject(hSHP, i);

		DPolygon2 dpoly;
		bool success = SHPToDPolygon2(pObj, dpoly);
		if (!success)
			iFailed ++;
		m_Poly.push_back(dpoly);

		SHPDestroyObject(pObj);
	}
	if (iFailed > 0)
		VTLOG("  %d of the %d entities were bad.\n", iFailed, nElems);
}

