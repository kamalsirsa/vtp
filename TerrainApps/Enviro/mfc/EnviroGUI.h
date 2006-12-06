//
// EnviroGUI.h
// GUI-specific functionality of the Enviro class
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "../Enviro.h"

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
	virtual void EarthPosUpdated();
	virtual vtString GetPathFromGUI();
	virtual vtString GetStringFromUser(const vtString &title, const vtString &msg);
};

// global singleton
extern EnviroGUI g_App;

