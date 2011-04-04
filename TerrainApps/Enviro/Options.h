//
// Options.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef OPTIONSH
#define OPTIONSH

#include "vtdata/FilePath.h"

class EnviroOptions
{
public:
	EnviroOptions();
	~EnviroOptions();

	bool ReadXML(const char *szFilename);
	bool WriteXML();

	// older method was .ini files
	bool ReadINI(const char *szFilename);

	vtStringArray m_oldDataPaths;
	bool		m_bEarthView;
	vtString	m_strEarthImage;
	vtString	m_strInitTerrain;
	vtString	m_strInitLocation;
	vtString	m_strUseElevation;

	bool	m_bStartInNeutral;

	// display options, window location and size
	bool	m_bFullscreen;
	bool	m_bStereo;
	int		m_iStereoMode;
	IPoint2	m_WinPos, m_WinSize;
	bool	m_bLocationInside;

	bool	m_bHtmlpane;
	bool	m_bFloatingToolbar;
	bool	m_bTextureCompression;
	bool	m_bDisableModelMipmaps;

	bool	m_bDirectPicking;
	float	m_fSelectionCutoff;
	float	m_fMaxPickableInstanceRadius;
	float	m_fCursorThickness;

	float	m_fPlantScale;
	bool	m_bShadows;
	bool	m_bOnlyAvailableSpecies;

	float	m_fCatenaryFactor;

	vtString	m_strContentFile;

	bool	m_bShowProgress;
	bool	m_bFlyIn;

	// toolbar options
	bool m_bShowToolsCulture;
	bool m_bShowToolsSnapshot;
	bool m_bShowToolsTime;

	// filename (with path) from which ini was read
	vtString m_strFilename;

	// look for all data here
};

extern EnviroOptions g_Options;

#endif	// OPTIONSH

