//
// EnviroGUI.h
// GUI-specific functionality of the Enviro class
//
// Copyright (c) 2003-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "../Enviro.h"

class vtJoystickEngine : public vtEngine
{
public:
	vtJoystickEngine();
	void Eval();

	void SetSpeed(float f) { m_fSpeed = f; };
	float GetSpeed() { return m_fSpeed; }

	class wxJoystick *m_pStick;
	float m_fSpeed, m_fLastTime;
};

class EnviroGUI : public Enviro
{
public:
	EnviroGUI();
	~EnviroGUI();

	// The following are overrides, to handle situations in which the
	//  GUI must be informed of what happens in the Enviro object.
	virtual void ShowPopupMenu(const IPoint2 &pos);
	virtual void SetTerrainToGUI(vtTerrain *pTerrain);
	virtual void ShowLayerView();
	virtual void RefreshLayerView();
	virtual void UpdateLayerView();
	virtual void CameraChanged();
	virtual void EarthPosUpdated();
	virtual void ShowDistance(const DPoint2 &p1, const DPoint2 &p2,
		double fGround, double fVertical);
	virtual vtTagArray *GetInstanceFromGUI();
	virtual bool OnMouseEvent(vtMouseEvent &event);
	virtual void SetupScene3();
	virtual void SetTimeEngineToGUI(class vtTimeEngine *pEngine);
	virtual bool IsAcceptable(vtTerrain *pTerr);
	virtual void ShowMessage(const vtString &str);
	virtual void SetFlightSpeed(float speed);
	virtual void SetState(AppState s);
	virtual vtString GetStringFromUser(const vtString &msg);

	// The following are useful methods
	void SaveVegetation(bool bAskFilename);
	void SaveStructures(bool bAskFilename);

	// navigation engines
	vtJoystickEngine	*m_pJFlyer;
};

// global singleton
extern EnviroGUI g_App;

