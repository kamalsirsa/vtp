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

void SetLocalProjection(vtProjection &proj, DPoint2 lower_left)
{
	((vtProjection &)g_Proj) = proj;

	// set the base coordinate at the lower-left corner of the terrain
	if (proj.IsGeographic())
		g_Proj.SetDegreeOrigin(lower_left);
	else
		g_Proj.SetMeterOrigin(lower_left);
}

void vtLocalGrid::SetGlobalProjection()
{
	SetLocalProjection(m_proj, DPoint2(m_area.left, m_area.bottom));
}

void vtLocalGrid::Setup(float fVerticalExag)
{
	m_fHeightScale = WORLD_SCALE * fVerticalExag;

	if (m_proj.IsGeographic())
	{
		g_Proj.convert_latlon_to_local_xz(m_area.bottom, m_area.left, m_WorldExtents.left, m_WorldExtents.bottom);
		g_Proj.convert_latlon_to_local_xz(m_area.top, m_area.right, m_WorldExtents.right, m_WorldExtents.top);
	}
	else
	{
		g_Proj.convert_meters_to_local_xz(m_area.left, m_area.bottom, m_WorldExtents.left, m_WorldExtents.bottom);
		g_Proj.convert_meters_to_local_xz(m_area.right, m_area.top, m_WorldExtents.right, m_WorldExtents.top);
	}

	// determine step size between each height sample
	m_fXStep = (float) (m_WorldExtents.right - m_WorldExtents.left) / (m_iColumns - 1);
	m_fZStep = (float) (m_WorldExtents.bottom - m_WorldExtents.top) / (m_iRows - 1);
}


void vtLocalGrid::GetLocation(int i, int j, FPoint3 &loc)
{
	if (m_bFloatMode)
		loc.Set(m_WorldExtents.left + i * m_fXStep,
				m_pFData[i*m_iRows+j] * m_fHeightScale,
				m_WorldExtents.bottom - j * m_fZStep);
	else
		loc.Set(m_WorldExtents.left + i * m_fXStep,
				m_pData[i*m_iRows+j] * m_fHeightScale,
				m_WorldExtents.bottom - j * m_fZStep);
}

//
// Quick n' dirty special-case raycast for perfectly regular grid terrain
// Find altitude (y) and surface normal, given (x,z) local coordinates
//
// This approach is really straightforward, so it could be majorly sped up if needed
//
bool vtLocalGrid::FindAltitudeAtPoint(FPoint3 &p, float &fAltitude, FPoint3 *vNormal)
{
	int iX = (int)((p.x - m_WorldExtents.left) / m_fXStep);
	int iZ = (int)((p.z - m_WorldExtents.bottom) / m_fZStep);

	// safety check
	if (iX < 0 || iX >= m_iColumns-1 || iZ < 0 || iZ >= m_iRows-1)
	{
		fAltitude = 0.0f;
		if (vNormal) vNormal->Set(0.0f, 1.0f, 0.0f);
		return false;
	}

	FPoint3 p0, p1, p2, p3;

	GetLocation(iX, iZ, p0);
	GetLocation(iX+1, iZ, p1);
	GetLocation(iX+1, iZ+1, p2);
	GetLocation(iX, iZ+1, p3);

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


void vtLocalGrid::GetWorldExtents(FRECT &rect)
{
	rect = m_WorldExtents;
}


void vtLocalGrid::ColorDibFromElevation(vtDIB *pDIB, RGBi color_ocean)
{
	int w = pDIB->GetWidth();
	int h = pDIB->GetHeight();

	int gw, gh;
	GetDimensions(gw, gh);

	int i, j;
	int x, y;
	RGBi color;

	float fMin, fMax;
	GetHeightExtents(fMin, fMax);

	Array<RGBi> colors;
	colors.Append(RGBi(75, 155, 75));
	colors.Append(RGBi(180, 160, 120));
	colors.Append(RGBi(128, 128, 128));
	int bracket, num = colors.GetSize();
	float bracket_size = (fMax - fMin) / (num - 1);

	// iterate over the texels
	for (i = 0; i < w; i++)
	{
		x = i * gw / w;			// find corresponding location in terrain

		for (j = 0; j < h; j++)
		{
			y = j * gh / h;

			float m = GetFValue(x, y);	// local units
			float elev = m - fMin;

			color.r = color.g = color.b = 0;
			if (m == 0.0f)
			{
				color = color_ocean;
			}
			else
			{
				bracket = (int) (elev / bracket_size);
				if (bracket < 0)
					color = colors[0];
				else if (bracket < num-1)
				{
					float fraction = (elev / bracket_size) - bracket;
					RGBi diff = (colors[bracket+1] - colors[bracket]);
					color = colors[bracket] + (diff * fraction);
				}
				else
					color = colors[num-1];
			}
#if 0
			else if (elev < low_elev)
			{
				color = color_base;
			}
			else if (elev < low_elev*2)
			{
				float scale = (elev - low_elev) / low_elev;
				RGBi diff = (color_hill - color_base);
				RGBi offset = (diff * scale);
				color = color_base + offset;
			}
			else
			{
				color = color_hill;
			}
#endif
			pDIB->SetPixel24(i, h-1-j, RGB(color.r, color.g, color.b));
		}
	}
}


void vtLocalGrid::ShadeDibFromElevation(vtDIB *pDIB, FPoint3 light_dir,
									float light_adj,
									int xPatch, int yPatch, int nPatches)
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

	float xFactor = (float)gw/(float)w/(float)nPatches;
	float yFactor = (float)gh/(float)h/(float)nPatches;

	yPatch = (nPatches - 1) - yPatch;
	float x_off = (float)xPatch * (float)gw/(float)nPatches;
	float y_off = (float)yPatch * (float)gh/(float)nPatches;

	bool mono = (pDIB->GetDepth() == 8);

	// iterate over the texels
	for (j = 0; j < h-1; j++)
	{
		// find corresponding location in terrain
		y = (int) (y_off + j * yFactor);
		for (i = 0; i < w-1; i++)
		{
			x = (int) (x_off + i * xFactor);

			float shade;
			if (x < gw-1)
			{
				// compute surface normal
				GetLocation(x, y, p1);
				GetLocation(x+1, y, p2);
				GetLocation(x, y+1, p3);
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


