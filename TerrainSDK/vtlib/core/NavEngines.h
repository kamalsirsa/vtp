//
// NavEngines.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef NAVENGINESH
#define NAVENGINESH

#include "HeightField.h"
#include "Engine.h"

///////////////////////////////////////////////////

/**
 *  This engine flies the viewpoint around using the mouse position.
 *
 *	Left button: forward-backward, yaw
 *  Right button: up-down, left-right
 *  Both buttons: pitch, roll
 *
 *  Position is considered relative to the center of the window.
 */
class vtFlyer : public vtLastMouse
{
public:
	vtFlyer(float fSpeed = 1.0f, bool bPreventRoll = false);

	void SetSpeed(float fSpeed);
	float GetSpeed();
	void SetAlwaysMove(bool bMove);

	void Eval();

protected:
	float	m_fSpeed;	// max units per frame of movement
	bool	m_bPreventRoll;
	bool	m_bAlwaysMove;
};

/**
 * This engine extends vtFlyer with the ability to do terrain following.
 *
 * You can set minimum height above the ground.
 */
class vtTerrainFlyer : public vtFlyer
{
public:
	vtTerrainFlyer(float fSpeed, float fHeightAboveTerrain, bool bMin);

	void SetHeightField(vtHeightField *pGrid) { m_pHeightField = pGrid; }
	void FollowTerrain(bool bFollow);
	void KeepAboveGround();
	void SetHeight(float fHeightAboveTerrain) { m_fHeightAboveTerrain = fHeightAboveTerrain; }
	float GetHeight() { return m_fHeightAboveTerrain; }
	void MaintainHeight(bool bMaintain) { m_bMaintain = bMaintain; }
	void SetMaintainHeight(float fheight) { m_fMaintainHeight = fheight; }

	// override
	void Eval();

protected:
	vtHeightField *m_pHeightField;
	float	m_fHeightAboveTerrain;
	bool	m_bMin;
	bool	m_bFollow;
	float	m_fMaintainHeight;
	bool	m_bMaintain;
};


//
// Similar to vtTerrainFlyer, but a velocity is maintained.
// Viewpoint moves even after mouse button is released.
//
class VFlyer : public vtTerrainFlyer
{
public:
	VFlyer(float scale, float fHeightAboveTerrain, bool bMin);

	void SetUpwardVelocity(float velocity);
	void SetGravity(bool bGrav) { m_bGravity = bGrav; }

	void Eval();	// overrides

protected:
	FPoint3	m_Velocity;
	bool	m_bGravity;
};


//
// Implemetation of a Quake-like engine
//
class QuakeFlyer : public vtTerrainFlyer
{
public:
	QuakeFlyer(float scale, float fHeightAboveTerrain, bool bMin);
	void Eval();

	// override
	void OnKey(int key, int flags);

protected:
	IPoint2	m_LastMousePt;
	float	m_flastmovex, m_flastmovey;
	short	m_sWrap;
	bool	m_bNavEnable;
};


//////////////////////////////////////////////////

class vtTrackball : public vtEngine
{
public:
	vtTrackball(float fRadius);

	void SetRadius(float fRadius);
	float GetRadius();
	void SetZoomScale(float s);
	void SetRotateButton(int button, int modifier);
	void SetZoomButton(int button, int modifier);

	void OnMouse(vtMouseEvent &event);
	void Eval();

protected:
	bool _IsRotate(vtMouseEvent &event);
	bool _IsZoom(vtMouseEvent &event);

	FPoint3	m_Pos, m_Start;
	bool	m_bRotate;
	bool	m_bZoom;
	IPoint2	m_MouseStart;
	float	m_fZoomScale;
	int		m_rotate_button;
	int		m_rotate_modifier;
	int		m_zoom_button;
	int		m_zoom_modifier;
};

#endif

