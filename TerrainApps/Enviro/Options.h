//
// Options.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef OPTIONSH
#define OPTIONSH

class EnviroOptions
{
public:
	EnviroOptions();

	bool Read(const char *szFilename);
	bool Write();

	vtString m_strDataPath;
	bool	m_bEarthView;
	vtString m_strImage;
	vtString m_strInitTerrain;
	bool	m_bFullscreen;
	bool	m_bGravity;
	bool	m_bHtmlpane;
	bool	m_bFloatingToolbar;
	bool	m_bSound;
	bool	m_bVCursor;
	bool	m_bSpeedTest;
	bool	m_bQuakeNavigation;
	float	m_fPlantScale;
	bool	m_bShadows;

	// filename (with path) from which ini was read
	vtString m_strFilename;

	// look for all data here
};

extern EnviroOptions g_Options;

#endif
