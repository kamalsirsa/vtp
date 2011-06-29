//
// Engines.h
//
// Engines used by Enviro
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ENVIRO_ENGINESH
#define ENVIRO_ENGINESH

#include "vtlib/core/Engine.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/core/ImageSprite.h"

class NodeGeom;
class LinkGeom;
class vtRoute;
class vtHeightField;

///////////////////////////////////////////////////

class TerrainPicker : public vtLastMouse
{
public:
	TerrainPicker();
	void Eval();
	void SetHeightField(vtHeightField3d *pHeight) { m_pHeightField = pHeight; }

	bool GetCurrentPoint(FPoint3 &p);
	bool GetCurrentEarthPos(DPoint3 &p);

	void OnMouse(vtMouseEvent &event);

protected:
	void FindGroundPoint();

	FPoint3			m_GroundPoint;
	vtHeightField3d *m_pHeightField;
	DPoint3			m_EarthPos;
	bool			m_bOnTerrain;
};

///////////////////////////////////////////////////

class vtIcoGlobe;

class GlobePicker : public vtLastMouse
{
public:
	GlobePicker();
	void Eval();
	void SetRadius(double fRadius) { m_fRadius = fRadius; }
	void SetGlobe(vtIcoGlobe *pGlobe) { m_pGlobe = pGlobe; }
	void SetTargetScale(float s) { m_fTargetScale = s; }

	bool GetCurrentPoint(FPoint3 &p);
	bool GetCurrentEarthPos(DPoint3 &p);

protected:
	FPoint3		m_GroundPoint;
	DPoint3		m_EarthPos;
	bool		m_bOnTerrain;
	double		m_fRadius;
	vtIcoGlobe	*m_pGlobe;
	float		m_fTargetScale;
};

///////////////////////////////////////////////////

class FlatFlyer : public vtLastMouse
{
public:
	FlatFlyer();

	void Eval();
	void OnMouse(vtMouseEvent &event);

protected:
	bool m_bDrag;
	bool m_bZoom;
	FPoint3 m_start_wp;
	IPoint2 m_startpos;
};

///////////////////////////////////////////////////

class GrabFlyer : public vtTerrainFlyer
{
public:
	GrabFlyer(float fSpeed);
	void Eval();
	void OnMouse(vtMouseEvent &event);

	void SetTerrainPicker(TerrainPicker *pTP) { m_pTP = pTP; }

protected:
	bool m_bDrag;
	bool m_bPivot;
	bool m_bZoom;
	IPoint2 m_startpos;

	TerrainPicker *m_pTP;
	FPoint3 m_start_wp;
	FMatrix4 m_start_eye;

	// for dragging
	float m_fHeight;
};

/**
 * Provide a view of the terrain from the top
 * and display a cursor to show current canera position
 * User can move the camera by clicking on the map
 * with the mouse middle button.
 */
class MapOverviewEngine : public vtEngine
{
public:
	MapOverviewEngine();
	~MapOverviewEngine();

	void OnMouse(vtMouseEvent &event);
	void Eval();

	void ShowMapOverview(bool bShow) { m_pMapGroup->SetEnabled(bShow); }
	bool GetShowMapOverview() {return m_pMapGroup->GetEnabled();}
	void SetTerrain(vtTerrain *pTerr);

protected:
	void CreateMapView();
	void RefreshMapView();
	void CreateArrow();

	vtGroup			*m_pMapGroup;
	osg::ref_ptr<vtImageSprite>	m_pMapView;
	vtTransform		*m_pArrow;
	vtImagePtr		 m_pOwnedImage;

	float	anglePrec;
	int		MapWidth;
	int		MapMargin;
	float	MapRatio;
	float	ratioMapTerrain;
	bool	m_bDown;
};

#endif	// ENVIRO_ENGINESH

