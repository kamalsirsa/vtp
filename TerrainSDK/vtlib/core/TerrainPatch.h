//
// TerrainPatch : a simple rectangular grid mesh
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINPATCHH
#define TERRAINPATCHH

class TerrainPatch : public vtMesh
{
public:
	TerrainPatch(int VtxType, int NumVertices);

	bool MakeGrid(int iXSteps = 4, int iZSteps = 4,
		float fXSize = 1.0f, float fZSize = 1.0f,
		float fXOffset = 0.0f, float fZOffset = 0.0f,
		float xTiling = 1.0f, float zTiling = 1.0f);

	void MoveVerticesIntoSineCurve(float fOffset);
	void SetVertexTextureCoords();
	void SetVertexColors();
	void FindAltitudeAtPoint(FPoint3 point, float &fAltitude, FPoint3 *vNormal);

	int		m_iXVerts, m_iZVerts;
	float	m_fXSize, m_fZSize;
	bool	m_bIsFlat;				// perfectly flat

protected:
};

vtMovGeom *CreatePlaneMGeom(vtMaterialArray *pMats, int iMatIdx,
							FPoint2 org, FPoint2 size,
							float xTiling, float zTiling, int steps);

#endif

