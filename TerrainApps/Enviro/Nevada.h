
#ifndef NEVADAH
#define NEVADAH

#include "vtlib/core/Terrain.h"
#include "PTerrain.h"

class NevadaTerrain : public PTerrain
{
public:
	NevadaTerrain();
	~NevadaTerrain();

	void CreateCustomCulture(bool bDoSound);

	void SetWaterHeight(float fWaterHeight);
	void SetWaterOn(bool on);

	void CreateWater();
	void CreateDetailTextures();

	void CreatePast();
	void CreatePresent();
	void CreateFuture();

	vtGroup			m_Past, m_Present, m_Future;
	vtLodGrid			*m_pTreeGrid;
	vtMovGeom		*m_pWaterShape, *m_pWaterShape2;
//	vtFog			*m_pFog;
	vtMaterial		*m_pDetailMat;
	vtMaterial		*m_pDetailMat2;

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

//	vtFog m_pFog1, m_pFog2;
	float m_fLow;
	float m_fHigh;
	float m_fWaterHeight;

	vtSprite *m_pSprite;

	vtMaterial *m_pPastApp;
	vtMaterial *m_pPresentApp;

	vtGroup *m_pTrees;
};

#endif
