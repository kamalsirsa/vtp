
#ifndef WINGSH
#define WINGSH

#include "vtlib/core/Terrain.h"
#include "vtlib/core/Engine.h"

class Butterfly : public vtTransform
{
public:
	Butterfly(vtTerrain *terrain, float radius, float speed, 
			  float height, FPoint3 center, float size_exag);

	static vtMaterialArray *m_pApps;
	vtMovGeom *m_pWing1, *m_pWing2;
};

class FlyingCritterEngine : public vtEngine
{
public:
	FlyingCritterEngine(Butterfly *critter, class vtTerrain* terrain, 
						FPoint3 center, float speed, float height, 
						vtHeightField* grid, float radius);

	void Eval();

	Butterfly *m_critter;
	vtTerrain* m_terrain;
	int m_num;

	float m_fSpeed; 
	float m_fCircleRadius;
	FPoint3 m_vCurPos;
	FPoint3 m_vCenterPos;
	float m_fLastTime;
	float m_fHeightAboveGround;
	float m_fCurRotation;

	vtHeightField* m_pHeightField;
};

#endif

