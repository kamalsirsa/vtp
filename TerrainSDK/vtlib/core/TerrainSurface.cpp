//
// vtTerrainGeom: A subclass of vtGeom which contains a
//	grid of TerrainPatch objects.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "TerrainSurface.h"

/////////////////////////////////////////////////////////////////////////

vtTerrainGeom::vtTerrainGeom() : vtGeom(), vtHeightFieldGrid()
{
	m_pPatches = NULL;
}

vtTerrainGeom::~vtTerrainGeom()
{
	if (m_pPatches)
	{
		int i, j;
		for (i = 0; i < m_iXPatches; i++)
		{
			for (j = 0; j < m_iZPatches; j++)
			{
				TerrainPatch *pPatch = GetPatch(i, j);
				if (pPatch) delete pPatch;
			}
		}
		free(m_pPatches);
		m_pPatches = NULL;
	}
}


FPoint3 *vtTerrainGeom::ComputeNormals(FPoint3 *pLocations)
{
	FPoint3 *pNormals = (FPoint3 *)malloc(m_iXPoints * m_iYPoints * sizeof(FPoint3));

	// Calculate normals for each vertex, for shading
//		FPoint3 v0, v1, v2;
	FPoint3 p0, c;
	FPoint3 v1, v2, v3, v4;
	int i, j;
	for (i = 0; i < m_iXPoints; i++)
	for (j = 0; j < m_iYPoints; j++)
	{
		c.Set(0.0f, 0.0f, 0.0f);
		p0 = pLocations[(i)*m_iYPoints+(j)];
		if (i > 0)
			v1 = pLocations[(i-1)*m_iYPoints+(j)] - p0;
		if (j > 0)
			v2 = pLocations[(i)*m_iYPoints+(j-1)] - p0;
		if (i < m_iXPoints-1)
			v3 = pLocations[(i+1)*m_iYPoints+(j)] - p0;
		if (j < m_iYPoints-1)
			v4 = pLocations[(i)*m_iYPoints+(j+1)] - p0;

		if (i > 0 && j > 0)
			c += (v1.Cross(v2));
		if (i < m_iXPoints-1 && j > 0)
			c += (v2.Cross(v3));
		if (i < m_iXPoints-1 && j < m_iYPoints-1)
			c += (v3.Cross(v4));
		if (i > 0 && j < m_iYPoints-1)
			c += (v4.Cross(v1));
#if 1
		// exaggerate X and Z components for heightened shading effect
		c.Normalize();
		c.x *= 3.0f;
		c.z *= 3.0f;
#endif
		c.Normalize();
		pNormals[(i)*m_iYPoints+(j)] = c;
	}
	return pNormals;
}


//
// Constructor for vtTerrainGeom
//
// iEveryX and iEveryZ are number of grid posts to subsample by
//
bool vtTerrainGeom::CreateFromLocalGrid(vtLocalGrid *pGrid, int VtxType,
							int iEveryX, int iEveryZ,
							int largest_block_size, int texture_patches,
							bool bSuppressLand, float fOceanDepth,
							bool bLighting)
{
	Initialize(pGrid);

	int i, j;
	int nColumns, nRows;
	FPoint3 p;
	float fValue;
	FPoint3 *pLocations = NULL, *pNormals = NULL;

	pGrid->GetDimensions(nColumns, nRows);

	int iXQuads = nColumns - 1;
	int iZQuads = nRows - 1;
	iXQuads /= iEveryX;
	iZQuads /= iEveryZ;
	m_iXPoints = iXQuads + 1;
	m_iYPoints = iZQuads + 1;

	float fXoffset = m_Conversion.m_WorldExtents.left;
	float fZoffset = m_Conversion.m_WorldExtents.bottom;

	// compute the number of patches we'll have to break it into
	m_iXPatches = texture_patches;
	m_iZPatches = texture_patches;
	while (iXQuads / m_iXPatches >= largest_block_size) m_iXPatches += texture_patches;
	m_iXPatches -= texture_patches;
	while (iZQuads / m_iZPatches >= largest_block_size) m_iZPatches += texture_patches;
	m_iZPatches -= texture_patches;
	AllocatePatches();

	// determine places to chop the mesh
	for (i = 0; i < m_iXPatches; i++) m_iChopx[i] = (m_iXPoints * i / m_iXPatches);
	m_iChopx[i] = m_iXPoints-1;
	for (i = 0; i < m_iZPatches; i++) m_iChopz[i] = (m_iYPoints * i / m_iZPatches);
	m_iChopz[i] = m_iYPoints-1;

	// remember where we chopped it, for quick testing later
	for (i = 0; i <= m_iXPatches; i++)
		m_fChopx[i] = fXoffset + m_iChopx[i] * iEveryX * m_fXStep;
	for (i = 0; i <= m_iZPatches; i++)
		m_fChopz[i] = fZoffset - m_iChopz[i] * iEveryZ * m_fZStep;

	pLocations = (FPoint3 *)malloc(m_iXPoints * m_iYPoints * sizeof(FPoint3));
	for (i = 0; i < m_iXPoints; i++)
	for (j = 0; j < m_iYPoints; j++)
	{
		fValue = pGrid->GetWorldValue(i*iEveryX, j*iEveryZ);
		p.x = fXoffset + (i * iEveryX * m_fXStep);
		p.y = fValue;
		p.z = fZoffset - (j * iEveryZ * m_fZStep);

		if (fValue == 0.0f) p.y = fOceanDepth;

		pLocations[(i)*m_iYPoints+(j)] = p;
	}

	if (bLighting)
		pNormals = ComputeNormals(pLocations);

	//
	// Create the quadmesh patches
	// Populate them with locations
	//
	int a, b, iNVerts;
	for (a = 0; a < m_iXPatches; a++)
	{
		for (b = 0; b < m_iZPatches; b++)
		{
			int iXVerts = m_iChopx[a+1] - m_iChopx[a] + 1;
			int iZVerts = m_iChopz[b+1] - m_iChopz[b] + 1;
			iNVerts = iXVerts * iZVerts;

			// check if data is flat
			bool bIsFlat = true;
			bool bHasSeaLevel = false;
			int iStepX = 1;
			int iStepZ = 1;
			for (i = m_iChopx[a]; i <= m_iChopx[a+1]; i++)
			{
				for (j = m_iChopz[b]; j <= m_iChopz[b+1]; j++)
				{
					fValue = pGrid->GetWorldValue(i*iEveryX, j*iEveryZ);
					if (fValue != 0.0f)
						bIsFlat = false;
					else
						bHasSeaLevel = true;
				}
			}
			bool bSuppressPatch = bIsFlat;
			if (bSuppressPatch)
			{
				iNVerts = 4;
				iStepX = m_iChopx[a+1] - m_iChopx[a];
				iStepZ = m_iChopz[b+1] - m_iChopz[b];
				iXVerts = 2;
				iZVerts = 2;
			}

			// create Patch geometry
			TerrainPatch *pPatch = new TerrainPatch(VtxType, iNVerts);

			// what is the size of this patch?
			pPatch->m_iXVerts = iXVerts;
			pPatch->m_iZVerts = iZVerts;
			pPatch->m_fXSize = iEveryX * m_fXStep;
			pPatch->m_fZSize = iEveryZ * m_fZStep;
			pPatch->m_bIsFlat = bIsFlat;

			// set elevation values of the mesh
			int vidx;
			for (i = m_iChopx[a]; i <= m_iChopx[a+1]; i += iStepX)
			{
				for (j = m_iChopz[b]; j <= m_iChopz[b+1]; j += iStepZ)
				{
					// vertex location
					vidx = pPatch->AddVertex(pLocations[(i)*m_iYPoints+(j)]);

					// compute vertex colors
					if (VtxType & VT_Colors)
					{
						fValue = pGrid->GetWorldValue(i*iEveryX, j*iEveryZ);
						RGBf c(1.0f, 1.0f, 1.0f);
						if (fValue == 0.0f)
							c.g = c.r = 0.0f;
						else
							c.g = c.b = 0.0f;
						if (((a+b)&1) != 0)
							c.g = 0.4f;
						pPatch->SetVtxColor(vidx, c);
					}

					// set vertex normals
					if (VtxType & VT_Normals)
					{
						pPatch->SetVtxNormal(vidx, pNormals[(i)*m_iYPoints+(j)]);
					}
				}
			}
			if (bSuppressPatch)
				pPatch->CreateRectangularMesh(2, 2);
			else
				pPatch->CreateRectangularMesh(pPatch->m_iXVerts, pPatch->m_iZVerts);

			SetPatch(a, b, pPatch);
			AddMesh(pPatch, 0);	// Material index will be set later in texture draping
		}
	}
	if (pLocations)
		free(pLocations);
	if (pNormals)
		free(pNormals);
	return true;
}


//
// Quick n' dirty special-case raycast for perfectly regular grid terrain
// Find altitude (y) and surface normal, given (x,z) coordinates
//
// Returns true if the point was over the terrain
//
bool vtTerrainGeom::FindAltitudeAtPoint(const FPoint3 &point, float &fAltitude,
										FPoint3 *vNormal) const
{
	int iX = -1, iZ = -1;
	for (int a = 0; a < m_iXPatches; a++)
		if (point.x > m_fChopx[a] && point.x < m_fChopx[a+1])
		{
			iX = a;
			break;
		}
	for (int b = 0; b < m_iZPatches; b++)
		if (point.z < m_fChopz[b] && point.z > m_fChopz[b+1])
		{
			iZ = b;
			break;
		}

	// safety check
	if (iX < 0 || iX >= m_iXPatches || iZ < 0 || iZ >= m_iZPatches)
	{
		fAltitude = 0.0f;
		if (vNormal) vNormal->Set(0.0f, 1.0f, 0.0f);
		return false;
	}
	TerrainPatch *pPatch = GetPatch(iX, iZ);

	if (!pPatch) return false;	// safety

	return pPatch->FindAltitudeAtPoint(point, fAltitude, vNormal);
}

TerrainPatch *vtTerrainGeom::GetPatch(int x, int z) const
{
	return m_pPatches[x * m_iZPatches + z];
}

void vtTerrainGeom::SetPatch(int x, int z, TerrainPatch *pPatch)
{
	m_pPatches[x * m_iZPatches + z] = pPatch;
}

void vtTerrainGeom::AllocatePatches()
{
	m_pPatches = (TerrainPatch **)calloc(m_iXPatches * m_iZPatches, sizeof(TerrainPatch *));
}


bool vtTerrainGeom::DrapeTextureUV()
{
	// just do exact (0,0 - 1,1) draping
	// use extent of terrain in the ground plane
	FPoint3 size(m_Conversion.m_WorldExtents.Width(), 0.0f,
				 m_Conversion.m_WorldExtents.Height());
	FPoint3 p3;
	int iNum;

	int i, j;
	for (i = 0; i < m_iXPatches; i++)
	{
		for (j = 0; j < m_iZPatches; j++)
		{
			TerrainPatch *pPatch = GetPatch(i, j);
			iNum = pPatch->GetNumVertices();
			for (int i = 0; i < iNum; i++)
			{
				p3 = pPatch->GetVtxPos(i);
				pPatch->SetVtxTexCoord(i, FPoint2(p3.x / size.x,
											 -p3.z / size.z));
			}
		}
	}
	return true;
}


bool vtTerrainGeom::DrapeTextureUVTiled(vtTextureCoverage *cover)
{
	// find extent of surface in the ground plane
	FPoint3 size(m_Conversion.m_WorldExtents.Width(), 0.0f,
				 m_Conversion.m_WorldExtents.Height());

	FPoint3 p3;
	int iNum;

	vtTextureCoverage *c, *this_c;

	int i, j, k;
	for (i = 0; i < m_iXPatches; i++)
	{
		for (j = 0; j < m_iZPatches; j++)
		{
			TerrainPatch *pPatch = GetPatch(i, j);

			for (c = cover; c; c=c->m_pNext)
				c->m_usecount = 0;

			iNum = pPatch->GetNumVertices();
			for (k = 0; k < iNum; k++)
			{
				p3 = pPatch->GetVtxPos(k);
				float u = p3.x / size.x;
				float v = -p3.z / size.z;

				for (c = cover; c; c=c->m_pNext)
				{
					if (c->m_xmin <= u && u <= c->m_xmax &&
						c->m_zmin >= v && v >= c->m_zmax)
					{
						break;
					}
				}
				if (c)
					c->m_usecount++;
			}
			// find the most-reference appidx
			int most_used = 0;
			for (c = cover; c; c=c->m_pNext)
			{
				if (c->m_usecount > most_used)
				{
					most_used = c->m_usecount;
					this_c = c;
				}
			}
			// now reset all the u,v values to local values, not global
			for (k = 0; k < iNum; k++)
			{
				p3 = pPatch->GetVtxPos(k);
				float global_u = p3.x / size.x;
				float global_v = -p3.z / size.z;

				float local_u = (global_u - this_c->m_xmin) / (this_c->m_xmax - this_c->m_xmin);
				float local_v = (global_v - this_c->m_zmin) / (this_c->m_zmin - this_c->m_zmax);

				pPatch->SetVtxTexCoord(k, FPoint2(local_u, local_v));
			}
			SetMeshMatIndex(pPatch, this_c->m_appidx);
		}
	}
	return true;
}

