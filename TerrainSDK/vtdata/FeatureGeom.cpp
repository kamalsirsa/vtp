//
// Features.cpp
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Features.h"
#include "xmlhelper/easyxml.hpp"
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

void vtFeatureSetPoint2D::Offset(const DPoint2 &p)
{
	for (unsigned int i = 0; i < m_Point2.GetSize(); i++)
		m_Point2[i] += p;
}

bool vtFeatureSetPoint2D::TransformCoords(OCT *pTransform)
{
	unsigned int i, bad = 0, size = m_Point2.GetSize();
	for (i = 0; i < size; i++)
	{
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

void vtFeatureSetPoint2D::FindAllPointsAtLocation(const DPoint2 &loc, Array<int> &found)
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

void vtFeatureSetPoint2D::SaveGeomToSHP(SHPHandle hSHP) const
{
	unsigned int size = m_Point2.GetSize();
	for (unsigned int i = 0; i < size; i++)
	{
		// Save to SHP
		DPoint2 p2 = m_Point2[i];
		SHPObject *obj = SHPCreateSimpleObject(SHPT_POINT, 1, &p2.x, &p2.y, NULL);
		SHPWriteObject(hSHP, -1, obj);
		SHPDestroyObject(obj);
	}
}

void vtFeatureSetPoint2D::LoadGeomFromSHP(SHPHandle hSHP)
{
	int nElems;
	SHPGetInfo(hSHP, &nElems, NULL, NULL, NULL);

	m_Point2.SetSize(nElems);

	// Read Data from SHP into memory
	DPoint2 p2;
	for (int i = 0; i < nElems; i++)
	{
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

void vtFeatureSetPoint3D::Offset(const DPoint2 &p)
{
	for (unsigned int i = 0; i < m_Point3.GetSize(); i++)
		m_Point3[i] += DPoint3(p.x, p.y, 0);
}

bool vtFeatureSetPoint3D::TransformCoords(OCT *pTransform)
{
	unsigned int i, bad = 0, size = m_Point3.GetSize();
	for (i = 0; i < size; i++)
	{
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

void vtFeatureSetPoint3D::SaveGeomToSHP(SHPHandle hSHP) const
{
	unsigned int size = m_Point3.GetSize();
	for (unsigned int i = 0; i < size; i++)
	{
		// Save to SHP
		DPoint3 p3 = m_Point3[i];
		SHPObject *obj = SHPCreateSimpleObject(SHPT_POINTZ, 1, &p3.x, &p3.y, &p3.z);
		SHPWriteObject(hSHP, -1, obj);
		SHPDestroyObject(obj);
	}
}

void vtFeatureSetPoint3D::LoadGeomFromSHP(SHPHandle hSHP)
{
	int nElems;
	SHPGetInfo(hSHP, &nElems, NULL, NULL, NULL);

	m_Point3.SetSize(nElems);

	// Read Data from SHP into memory
	DPoint3 p3;
	for (int i = 0; i < nElems; i++)
	{
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

void vtFeatureSetLineString::Offset(const DPoint2 &p)
{
	for (unsigned int i = 0; i < m_Line.size(); i++)
		m_Line[i].Add(p);
}

bool vtFeatureSetLineString::TransformCoords(OCT *pTransform)
{
	unsigned int i, j, pts, bad = 0, size = m_Line.size();
	for (i = 0; i < size; i++)
	{
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

bool vtFeatureSetLineString::IsInsideRect(int iElem, const DRECT &rect)
{
	return rect.ContainsLine(m_Line[iElem]);
}

void vtFeatureSetLineString::CopyGeometry(unsigned int from, unsigned int to)
{
	// copy geometry
	m_Line[to] = m_Line[from];
}

void vtFeatureSetLineString::SaveGeomToSHP(SHPHandle hSHP) const
{
	unsigned int i, j, size = m_Line.size();
	for (i = 0; i < size; i++)
	{
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

void vtFeatureSetLineString::LoadGeomFromSHP(SHPHandle hSHP)
{
	int nElems;
	SHPGetInfo(hSHP, &nElems, NULL, NULL, NULL);

	m_Line.reserve(nElems);

	// Read Data from SHP into memory
	for (int i = 0; i < nElems; i++)
	{
		DLine2 dline;

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
				dline.SetAt(j, DPoint2(pObj->padfX[j], pObj->padfY[j]));

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

void vtFeatureSetPolygon::Offset(const DPoint2 &p)
{
	for (unsigned int i = 0; i < m_Poly.size(); i++)
		m_Poly[i].Add(p);
}

bool vtFeatureSetPolygon::TransformCoords(OCT *pTransform)
{
	unsigned int i, j, k, pts, bad = 0, size = m_Poly.size();
	for (i = 0; i < size; i++)
	{
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
		m_Poly.push_back(pFrom->m_Poly[i]);
	return true;
}

int vtFeatureSetPolygon::AddPolygon(const DPolygon2 &poly)
{
	int rec = m_Poly.size();
	m_Poly.push_back(poly);
	AddRecord();
	return rec;
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
	return rect.ContainsLine(dpoly[0]);
}

void vtFeatureSetPolygon::CopyGeometry(unsigned int from, unsigned int to)
{
	// copy geometry
	m_Poly[to] = m_Poly[from];
}

void vtFeatureSetPolygon::SaveGeomToSHP(SHPHandle hSHP) const
{
	unsigned int i, j, size = m_Poly.size();
	int part;
	for (i = 0; i < size; i++)		// for each polyline
	{
		const DPolygon2 &poly = m_Poly[i];
		int parts = poly.size();

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
		SHPObject *obj = SHPCreateObject(SHPT_POLYGON, -1, parts, panPartStart,
			NULL, total, dX, dY, NULL, NULL );

		SHPWriteObject(hSHP, -1, obj);
		SHPDestroyObject(obj);

		delete [] panPartStart;
		delete [] dY;
		delete [] dX;
	}
}

void vtFeatureSetPolygon::LoadGeomFromSHP(SHPHandle hSHP)
{
	int nElems;
	SHPGetInfo(hSHP, &nElems, NULL, NULL, NULL);

	m_Poly.reserve(nElems);

	// Read Data from SHP into memory
	for (int i = 0; i < nElems; i++)
	{
		DLine2 dline;
		DPolygon2 dpoly;

		// Get the i-th Shape in the SHP file
		SHPObject *pObj = SHPReadObject(hSHP, i);

		// Beware: it is possible for the shape to not actually have vertices
		if (pObj->nVertices == 0)
			m_Poly[i] = dpoly;
		else
		{
			// Store each part
			for (int part = 0; part < pObj->nParts; part++)
			{
				int start, end;
				
				start = pObj->panPartStart[part];
				if (part+1 < pObj->nParts)
					end = pObj->panPartStart[part+1]-1;
				else
					end = pObj->nVertices-1;

				// SHP files always duplicate the first point of each ring (part)
				// which we can ignore
				end--;

				dline.SetSize(end - start + 1);
				for (int j = start; j <= end; j++)
					dline.SetAt(j-start, DPoint2(pObj->padfX[j], pObj->padfY[j]));

				dpoly.push_back(dline);
			}
			m_Poly.push_back(dpoly);
		}
		SHPDestroyObject(pObj);
	}
}

