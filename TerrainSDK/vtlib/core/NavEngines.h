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
#include "Event.h"

/** \defgroup nav Navigation
 * These classes are used for navigation: moving a camera or similar object
 * based on user input.
 */
/*@{*/

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
	void SetMultiplier(float fMult) { m_fMult = fMult; }

	void Eval();

	enum DOF { DOF_X = 0, DOF_Y, DOF_Z, DOF_PITCH, DOF_YAW, DOF_ROLL };
	void SetDOF(DOF dof, bool bTrue) { m_bDOF[dof] = bTrue; }
	bool GetDOF(DOF dof) { return m_bDOF[dof]; }

protected:
	void DoKeyNavigation();

	float	m_fSpeed;		// max units per frame of movement
	float	m_fMult;		// multiply speed by this factor
	bool	m_bDOF[6];		// six degrees of freedom
	bool	m_bAlwaysMove;
};

/**
 * This engine extends vtFlyer for navigation of an orthographic
 * camera (usually, top-down).
 *
 *	Left button: forward-backward (implemented as zoom), yaw
 *  Right button: up-down, left-right
 *
 *  Position is considered relative to the center of the window.
 */
class vtOrthoFlyer : public vtFlyer
{
public:
	vtOrthoFlyer(float fSpeed);

	void Eval();

protected:
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
	vtTerrainFlyer(float fSpeed, float fMinHeight, bool bMin);

	/// Set the heightfield on which to do the terrain following.
	void SetHeightField(vtHeightField3d *pGrid) { m_pHeightField = pGrid; }

	/// Enables whether to do terrain following or not.  Default is true.
	void FollowTerrain(bool bFollow);

	/// Set the desired minimum height above the terrain.
	void SetHeight(float fMinHeight) { m_fMinHeight = fMinHeight; }

	/// Get the minimum height about the terrain.
	float GetHeight() { return m_fMinHeight; }

	/// If true, the current height above the terrain is maintained.  Default is false.
	void SetMaintain(bool bMaintain) { m_bMaintain = bMaintain; }
	bool GetMaintain() { return m_bMaintain; }

	// If true, exaggerate the speed of the view by height above ground
	void SetExag(bool bDo) { m_bExag = bDo; }
	bool GetExag() { return m_bExag; }

	/// Set the height about the terrain to maintain, or pass 0 to use the current value.
	void SetMaintainHeight(float fheight) { m_fMaintainHeight = fheight; }

	// override
	void Eval();

	/// Current speed (meters/second as of the most recent frame)
	float GetCurrentSpeed() { return m_fCurrentSpeed; }

protected:
	void KeepAboveGround();

protected:
	vtHeightField3d *m_pHeightField;
	float	m_fAboveGround;	// height above the ground as of last frame
	float	m_fMinHeight;
	bool	m_bMin;
	bool	m_bFollow;
	float	m_fMaintainHeight;
	bool	m_bMaintain;
	float	m_fCurrentSpeed; // current speed (most recent frame)
	bool	m_bExag;		// exaggerate speed based on height
};


/**
 *  This engine looks the viewpoint around using the mouse position.
 *  Movement is only done when left mouse button is pressed.
 *  Right mouse button is a holdover from vtFlyer.
 *
 *	No buttons: pitch, yaw
 *  Left button: forward
 *  Right button: up/dn, left/right
 *
 *  Position is considered relative to the center of the window.
 */
class vtPanoFlyer : public vtTerrainFlyer
{
public:
	vtPanoFlyer(float fSpeed, float fMinHeight, bool bMin);

	void Eval();

protected:
	float	m_Velocity;
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
	void SetHeight(float fMinHeight) { m_fMinHeight = fMinHeight; }

	/// Get the minimum height about the terrain.
	float GetHeight() { return m_fMinHeight; }

	// implementation override
	void Eval();

protected:
	float	m_fSpeed;	// max units per frame of movement
	vtTin3d	*m_pTin;
	float	m_fMinHeight;
	float	m_fPitch;

	void KeepAboveGround();
};


/**
 * Similar to vtTerrainFlyer, but a velocity is maintained.
 * Viewpoint moves even after mouse button is released.
 */
class VFlyer : public vtTerrainFlyer
{
public:
	VFlyer(float scale, float fMinHeight, bool bMin);

	void SetVerticalVelocity(float velocity);

	void Eval();	// overrides

protected:
	FPoint3	m_Velocity;
	float	m_last_time;
};


/**
 * Implementation of a Quake-like navigation engine.  Mouse position
 * rotates the view direction, and keyboard input moves the view
 * position.
 */
class QuakeFlyer : public vtTerrainFlyer
{
public:
	QuakeFlyer(float scale, float fMinHeight, bool bMin);
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

/**
 * A Trackball is a navigation engine which can move a Camera (or any
 * other Transform target) and move it around a center point based on
 * mouse input.
 */
class vtTrackball : public vtLastMouse
{
public:
	vtTrackball(float fRadius);

	void SetRadius(float fRadius);
	float GetRadius();
	void SetZoomScale(float s);
	void SetRotateButton(int button, int modifier = 0, bool bExact = true);
	void SetZoomButton(int button, int modifier = 0, bool bExact = true);
	void SetTranslateButton(int button, int modifier = 0, bool bExact = true);
	void SetDirection(float fTheta, float fPhi);

	void OnMouse(vtMouseEvent &event);
	void Eval();

protected:
	bool _IsRotate();
	bool _IsZoom();
	bool _IsTranslate();

	FPoint3	m_Pos, m_Start, m_Trans;
	bool	m_bRotate;
	bool	m_bZoom;
	bool	m_bTrans;

	IPoint2	m_MouseStart;
	float	m_fZoomScale;

	int		m_rotate_button;
	int		m_rotate_modifier;
	bool	m_rotate_exact;
	int		m_zoom_button;
	int		m_zoom_modifier;
	bool	m_zoom_exact;
	int		m_trans_button;
	int		m_trans_modifier;
	bool	m_trans_exact;
};

/*@}*/	// Group nav

#endif

