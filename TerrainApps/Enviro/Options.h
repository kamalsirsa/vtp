//
// Options.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
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

	bool Read(const char *szFilename);
	bool Write();

	vtStringArray m_DataPaths;
	bool		m_bEarthView;
	vtString	m_strEarthImage;
	vtString	m_strInitTerrain;

	bool	m_bStartInNeutral;

	bool	m_bFullscreen;
	bool	m_bHtmlpane;
	bool	m_bFloatingToolbar;
	bool	m_bTextureCompression;
	bool	m_bDisableModelMipmaps;

	float	m_fSelectionCutoff;
	float	m_fMaxPickableInstanceRadius;
	float	m_fCursorThickness;

	float	m_fPlantScale;
	bool	m_bShadows;
	bool	m_bOnlyAvailableSpecies;

	float	m_fCatenaryFactor;

	vtString	m_strContentFile;

	// filename (with path) from which ini was read
	vtString m_strFilename;

	// look for all data here
};

extern EnviroOptions g_Options;

#endif	// OPTIONSH

