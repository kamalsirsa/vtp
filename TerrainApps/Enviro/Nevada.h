//
// Nevada.h
//
// Terrain implementation specific to Black Rock City, Nevada.
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef NEVADAH
#define NEVADAH

#include "vtlib/core/Terrain.h"
#include "vtlib/core/Engine.h"

class NevadaTerrain : public vtTerrain
{
public:
	NevadaTerrain();

	void CreateCustomCulture();

	void SetWaterHeight(float fWaterHeight);
	void SetWaterOn(bool on);

	void CreateWater();
	void CreateDetailTextures();

	void CreatePast();
	void CreatePresent();
	void CreateFuture();

	vtGroup		*m_pPast, *m_pPresent, *m_pFuture;
	vtLodGrid	*m_pTreeGrid;
	vtMovGeode	*m_pWaterShape, *m_pWaterShape2;
	vtImagePtr	 m_pDetailTexture, m_pDetailTexture2;
	vtMaterial	*m_pDetailMat;
	vtMaterial	*m_pDetailMat2;
	vtMaterialArray *m_pMats;

	FPoint3 man_location;
	float	m_fGround, m_fHigh, m_fLow;
};

class JumpingEngine : public vtEngine
{
public:
	JumpingEngine(FPoint3 center, float fScale, float fBase, float fJumpHeight, float fPhase);
	bool Eval(float t);

	FPoint3 m_center;
	float m_fBase;
	float m_fJumpHeight;
	FPoint3 m_vel;
	float m_fPhase;
	float m_fScale;
};

//////////////////////////////////////////////////

class EpochEngine : public vtEngine
{
public:
	EpochEngine(class NevadaTerrain *pNevada, float fLow, float fHigh,
		vtMaterial* pastApp, vtMaterial* presentApp);

	void Eval();

	void SetYear(int year);
	int	m_iYear;
	int m_iTargetYear;
	int m_iSpeed;
	int m_iShownYear;

	class NevadaTerrain *m_pNevada;

	float m_fLow;
	float m_fHigh;
	float m_fWaterHeight;

	vtGeode *m_pSprite;
	vtTextMesh *m_pSpriteText;

	vtMaterial *m_pPastMat;
	vtMaterial *m_pPresentMat;

	vtGroup *m_pPlants;
};

#endif	// NEVADAH

