//
// vtLocalGrid class:
//
// a subclass of ElevationGrid, cast into local coordinates, with
//		the ability to do terrain following
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "LocalProjection.h"
#include "LocalGrid.h"
#include "HeightField.h"

vtLocalGrid::vtLocalGrid() : vtElevationGrid()
{
	m_fXStep = m_fZStep = 0;
}

vtLocalGrid::vtLocalGrid(const DRECT &area, int iColumns, int iRows,
						 bool bFloat, vtProjection &proj, float fVerticalExag) :
	vtElevationGrid(area, iColumns, iRows, bFloat, proj)
{
	SetupConversion(fVerticalExag);
}

void vtLocalGrid::SetGlobalProjection()
{
	// copy projection and conversion to global variable
	g_Conv = m_Conversion;
}

void vtLocalGrid::SetupConversion(float fVerticalExag)
{
	m_Conversion.Setup(m_proj.GetUnits(), m_area);
	m_Conversion.m_fVerticalScale = fVerticalExag;

	// determine step size between each height sample
	m_fXStep = m_Conversion.m_WorldExtents.Width() / (m_iColumns - 1);
	m_fZStep = -m_Conversion.m_WorldExtents.Height() / (m_iRows - 1);
}

void vtLocalGrid::GetWorldLocation(int i, int j, FPoint3 &loc)
{
	if (m_bFloatMode)
		loc.Set(m_Conversion.m_WorldExtents.left + i * m_fXStep,
				m_pFData[i*m_iRows+j] * m_Conversion.m_fVerticalScale,
				m_Conversion.m_WorldExtents.bottom - j * m_fZStep);
	else
		loc.Set(m_Conversion.m_WorldExtents.left + i * m_fXStep,
				m_pData[i*m_iRows+j] * m_Conversion.m_fVerticalScale,
				m_Conversion.m_WorldExtents.bottom - j * m_fZStep);
}

//
// Quick n' dirty special-case raycast for perfectly regular grid terrain
// Find altitude (y) and surface normal, given (x,z) local coordinates
//
// This approach is really straightforward, so it could be majorly sped up if needed
//
bool vtLocalGrid::FindAltitudeAtPoint(const FPoint3 &p, float &fAltitude,
									  FPoint3 *vNormal)
{
	int iX = (int)((p.x - m_Conversion.m_WorldExtents.left) / m_fXStep);
	int iZ = (int)((p.z - m_Conversion.m_WorldExtents.bottom) / m_fZStep);

	// safety check
	if (iX < 0 || iX >= m_iColumns-1 || iZ < 0 || iZ >= m_iRows-1)
	{
		fAltitude = 0.0f;
		if (vNormal) vNormal->Set(0.0f, 1.0f, 0.0f);
		return false;
	}

	FPoint3 p0, p1, p2, p3;

	GetWorldLocation(iX, iZ, p0);
	GetWorldLocation(iX+1, iZ, p1);
	GetWorldLocation(iX+1, iZ+1, p2);
	GetWorldLocation(iX, iZ+1, p3);

	// find fractional amount (0..1 across quad)
	float fX = (p.x - p0.x) / m_fXStep;
	float fZ = (p.z - p0.z) / m_fZStep;

	// which of the two triangles in the quad is it?
	if (fX + fZ < 1)
	{
		fAltitude = p0.y + fX * (p1.y - p0.y) + fZ * (p3.y - p0.y);

		if (vNormal)
		{
			// find normal also
			FPoint3 edge0 = p1 - p0;
			FPoint3 edge1 = p3 - p0;
			*vNormal = edge0.Cross(edge1);
			vNormal->Normalize();
		}
	}
	else
	{
		fAltitude = p2.y + (1.0f-fX) * (p3.y - p2.y) + (1.0f-fZ) * (p1.y - p2.y);

		if (vNormal)
		{
			// find normal also
			FPoint3 edge0 = p3 - p2;
			FPoint3 edge1 = p1 - p2;
			*vNormal = edge0.Cross(edge1);
			vNormal->Normalize();
		}
	}
	return true;
}


float vtLocalGrid::GetWorldValue(int i, int j)
{
	if (m_bFloatMode)
		return m_pFData[i*m_iRows+j] * m_Conversion.m_fVerticalScale;
	else
		return m_pData[i*m_iRows+j] * m_Conversion.m_fVerticalScale;
}

void vtLocalGrid::ShadeDibFromElevation(vtDIB *pDIB, FPoint3 light_dir,
									float light_adj)
{
	FPoint3 p1, p2, p3;
	FPoint3 v1, v2, v3;

	light_dir = -light_dir;

	int w = pDIB->GetWidth();
	int h = pDIB->GetHeight();

	int gw, gh;
	GetDimensions(gw, gh);

	int i, j;
	int x, y;
	int r, g, b;

	float xFactor = (float)gw/(float)w;
	float yFactor = (float)gh/(float)h;

	bool mono = (pDIB->GetDepth() == 8);

	// iterate over the texels
	for (j = 0; j < h-1; j++)
	{
		// find corresponding location in terrain
		y = (int) (j * yFactor);
		for (i = 0; i < w-1; i++)
		{
			x = (int) (i * xFactor);

			float shade;
			if (x < gw-1)
			{
				// compute surface normal
				GetWorldLocation(x, y, p1);
				GetWorldLocation(x+1, y, p2);
				GetWorldLocation(x, y+1, p3);
				v1 = p2 - p1;
				v2 = p3 - p1;
				v3 = v1.Cross(v2);
				v3.Normalize();

				shade = v3.Dot(light_dir);	// shading 0 (dark) to 1 (light)
				shade /= .7071f;
				shade = 1.0f + ((shade - 1.0f) * 2.0f);
				if (shade < 0.3f)	// clip - don't shade down below ambient level
					shade = 0.3f;
				else if (shade > 1.2f)
					shade = 1.2f;
			}
			else
				shade = 1.0f;

			float diff = 1 - shade;
			diff = diff * (1 - light_adj);
			shade += diff;

			if (mono)
			{
				unsigned int texel = pDIB->GetPixel8(i, h-1-j);
				texel = (int) (texel * shade);
				if (texel > 255) texel = 255;
				pDIB->SetPixel8(i, h-1-j, texel);
			}
			else
			{
				// combine color and shading
				unsigned long packed = pDIB->GetPixel24(i, h-1-j);
				r = GetRValue(packed);
				g = GetGValue(packed);
				b = GetBValue(packed);

				r = (int) (r * shade);
				g = (int) (g * shade);
				b = (int) (b * shade);
				if (r > 255) r = 255;
				if (g > 255) g = 255;
				if (b > 255) b = 255;

				pDIB->SetPixel24(i, h-1-j, RGB(r, g, b));
			}
		}
	}
}

DPoint2 vtLocalGrid::GetWorldSpacing()
{
	return DPoint2(m_fXStep, m_fZStep);
}

