//
// NavEngines.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef NAVENGINESH
#define NAVENGINESH

#include "vtdata/HeightField.h"
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

	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	float GetSpeed() { return m_fSpeed; }

	void SetAlwaysMove(bool bMove);

	void Eval();

protected:
	float	m_fSpeed;	// max units per frame of movement
	bool	m_bPreventRoll;
	bool	m_bAlwaysMove;
};

/**
 * This engine extends vtFlyer with the ability to do terrain following
 * (keeping the target "above the ground" relative to a given heightfield).
 *
 * You can set minimum height above the ground.
 */
class vtTerrainFlyer : public vtFlyer
{
public:
	vtTerrainFlyer(float fSpeed, float fHeightAboveTerrain, bool bMin);

	/// Set the heightfield on which to do the terrain following.
	void SetHeightField(vtHeightField3d *pGrid) { m_pHeightField = pGrid; }

	/// Enables whether to do terrain following or not.  Default is true.
	void FollowTerrain(bool bFollow);

	/// Set the desired minimum height above the terrain.
	void SetHeight(float fHeightAboveTerrain) { m_fHeightAboveTerrain = fHeightAboveTerrain; }

	/// Get the minimum height about the terrain.
	float GetHeight() { return m_fHeightAboveTerrain; }

	/// If true, the current height above the terrain is maintained.  Default is false.
	void MaintainHeight(bool bMaintain) { m_bMaintain = bMaintain; }

	/// Set the height about the terrain to maintain, or pass 0 to use the current value.
	void SetMaintainHeight(float fheight) { m_fMaintainHeight = fheight; }

	// override
	void Eval();

protected:
	void KeepAboveGround();

protected:
	vtHeightField3d *m_pHeightField;
	float	m_fHeightAboveTerrain;
	bool	m_bMin;
	bool	m_bFollow;
	float	m_fMaintainHeight;
	bool	m_bMaintain;
};

class vtTin3d;

/**
 * This engine has the ability to do usefully constrained navigation over
 * a (generally sparse) TIN terrain.
 */
class vtTinFlyer : public vtLastMouse
{
public:
	vtTinFlyer(float fSpeed);

	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	float GetSpeed() { return m_fSpeed; }

	/// Set the TIN heightfield to which to constrain
	void SetTin(vtTin3d *pTin);

	/// Set the desired minimum height above the terrain.
	void SetHeight(float fHeightAboveTerrain) { m_fHeightAboveTerrain = fHeightAboveTerrain; }

	/// Get the minimum height about the terrain.
	float GetHeight() { return m_fHeightAboveTerrain; }

	// implementation override
	void Eval();

protected:
	float	m_fSpeed;	// max units per frame of movement
	vtTin3d	*m_pTin;
	float	m_fHeightAboveTerrain;
	float	m_fPitch;

	void KeepAboveGround();
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

