//
// Options.cpp
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"
#include "xmlhelper/easyxml.hpp"

#include <string.h>
#include <fstream>

#include "Options.h"

using namespace std;

EnviroOptions g_Options;

#define STR_DATAPATH "DataPath"
#define STR_EARTHVIEW "EarthView"
#define STR_EARTHIMAGE "EarthImage"
#define STR_INITTERRAIN "InitialTerrain"
#define STR_FULLSCREEN "FullScreen"
#define STR_STEREO "Stereo"
#define STR_STEREO_MODE "StereoMode"
#define STR_WINLOC "WindowLocation"
#define STR_LOCINSIDE "LocationIsInside"
#define STR_HTMLPANE "HTMLPane"
#define STR_FLOATBAR "FloatingToolBar"
#define STR_TEXTURE_COMPRESSION "TextureCompression"
#define STR_PLANTSIZE "PlantSize"
#define STR_PLANTSHADOWS "PlantShadows"
#define STR_ONLY_AVAILABLE_SPECIES "ShowOnlyAvailableSpecies"
#define STR_SELECTIONCUTOFF "SelectionCutoff"
#define STR_DISABLE_MODEL_MIPMAPS "DisableModelMipmaps"
#define STR_CURSOR_THICKNESS "CursorThickness"
#define STR_CONTENT_FILE "ContentFile"
#define STR_CATENARY_FACTOR "CatenaryFactor"
#define STR_MAX_INST_RADIUS "MaxPickableInstanceRadius"
#define STR_DIRECT_PICKING "DirectPicking"
#define STR_SHOW_PROGRESS "ShowProgress"
#define STR_FLY_IN "FlyIn"
#define STR_TB_CULTURE "TBCulture"
#define STR_TB_SNAPSHOT "TBSnapshot"
#define STR_TB_TIME "TBTime"

EnviroOptions::EnviroOptions()
{
	// Provide defaults
	m_bEarthView = false;
	m_strEarthImage = "ev11656_512";

	m_bStartInNeutral = false;

	m_bFullscreen = false;
	m_bStereo = false;
	m_iStereoMode = 0;
	m_WinPos.Set(50, 50);
	m_WinSize.Set(800, 600);
	m_bLocationInside = false;

	m_bHtmlpane = false;
	m_bFloatingToolbar = false;
	m_bTextureCompression = true;
	m_bDisableModelMipmaps = false;

	m_bDirectPicking = false;
	m_fSelectionCutoff = 10.0f;
	m_fMaxPickableInstanceRadius = 200.0f;
	m_fCursorThickness = 0.012f;

	m_fPlantScale = 1.0f;
	m_bShadows = false;
	m_bOnlyAvailableSpecies = true;
	m_fCatenaryFactor = 140.0f;

	m_bShowProgress = true;
	m_bFlyIn = false;

	m_bShowToolsCulture = true;
	m_bShowToolsSnapshot = true;
	m_bShowToolsTime = true;
}

EnviroOptions::~EnviroOptions()
{
}

void LocalToUTF8(vtString &str)
{
	wstring2 ws((const char *)str);
	str = ws.to_utf8();
}

bool EnviroOptions::ReadINI(const char *szFilename)
{
	VTLOG("Reading options from '%s'\n", szFilename);

	ifstream input(szFilename, ios::in | ios::binary);
	if (!input.is_open())
		return false;

	char buf[80];
	bool bFoundContentFile = false;

	while (!input.eof())
	{
		if (input.peek() == '\n')
			input.ignore();
		input >> buf;

		// data value should been separated by a tab or space
		int next = input.peek();
		if (next != '\t' && next != ' ')
			continue;
		while (input.peek() == '\t' || input.peek() == ' ')
			input.ignore();

		if (strcmp(buf, STR_DATAPATH) == 0)
		{
			m_oldDataPaths.push_back(vtString(get_line_from_stream(input)));
		}
		else if (strcmp(buf, STR_EARTHVIEW) == 0)
			input >> m_bEarthView;
		else if (strcmp(buf, STR_EARTHIMAGE) == 0)
			m_strEarthImage = get_line_from_stream(input);
		else if (strcmp(buf, STR_INITTERRAIN) == 0)
			m_strInitTerrain = get_line_from_stream(input);
		else if (strcmp(buf, STR_FULLSCREEN) == 0)
			input >> m_bFullscreen;
		else if (strcmp(buf, STR_STEREO) == 0)
			input >> m_bStereo;
		else if (strcmp(buf, STR_STEREO_MODE) == 0)
			input >> m_iStereoMode;
		else if (strcmp(buf, STR_WINLOC) == 0)
			input >> m_WinPos.x >> m_WinPos.y >> m_WinSize.x >> m_WinSize.y;
		else if (strcmp(buf, STR_LOCINSIDE) == 0)
			input >> m_bLocationInside;
		else if (strcmp(buf, STR_HTMLPANE) == 0)
			input >> m_bHtmlpane;
		else if (strcmp(buf, STR_FLOATBAR) == 0)
			input >> m_bFloatingToolbar;
		else if (strcmp(buf, STR_TEXTURE_COMPRESSION) == 0)
			input >> m_bTextureCompression;
		else if (strcmp(buf, STR_PLANTSIZE) == 0)
			input >> m_fPlantScale;
		else if (strcmp(buf, STR_PLANTSHADOWS) == 0)
			input >> m_bShadows;
		else if (strcmp(buf, STR_ONLY_AVAILABLE_SPECIES) == 0)
			input >> m_bOnlyAvailableSpecies;
		else if (strcmp(buf, STR_DIRECT_PICKING) == 0)
			input >> m_bDirectPicking;
		else if (strcmp(buf, STR_SELECTIONCUTOFF) == 0)
			input >> m_fSelectionCutoff;
		else if (strcmp(buf, STR_DISABLE_MODEL_MIPMAPS) == 0)
			input >> m_bDisableModelMipmaps;
		else if (strcmp(buf, STR_CURSOR_THICKNESS) == 0)
			input >> m_fCursorThickness;
		else if (strcmp(buf, STR_CATENARY_FACTOR) == 0)
			input >> m_fCatenaryFactor;
		else if (strcmp(buf, STR_CONTENT_FILE) == 0)
		{
			m_strContentFile = get_line_from_stream(input);
			bFoundContentFile = true;
		}
		else if (strcmp(buf, STR_MAX_INST_RADIUS) == 0)
			input >> m_fMaxPickableInstanceRadius;
		else if (strcmp(buf, STR_SHOW_PROGRESS) == 0)
			input >> m_bShowProgress;
		else if (strcmp(buf, STR_FLY_IN) == 0)
			input >> m_bFlyIn;
		else
		{
//			cout << "Input from INI file unrecognized.\n";
			get_line_from_stream(input);
		}
	}

	// if we have an old file, provide the modern default
	if (!bFoundContentFile)
		m_strContentFile = "common_content.vtco";

	// Bad old INI file stored its strings in the local charset, which could
	//  vary from machine to machine.  Nowadays we always encode in utf-8.
	for (unsigned int i = 0; i < m_oldDataPaths.size(); i++)
		LocalToUTF8(m_oldDataPaths[i]);
	LocalToUTF8(m_strEarthImage);
	LocalToUTF8(m_strInitTerrain);
	LocalToUTF8(m_strContentFile);

	m_strFilename = szFilename;
	return true;
}

///////////////////////////////////////////////////////////////////////
// XML format

class EnviroOptionsVisitor : public XMLVisitor
{
public:
	EnviroOptionsVisitor(EnviroOptions &opt) : m_opt(opt) {}
	void startElement(const char *name, const XMLAttributes &atts) { m_data = ""; }
	void endElement (const char *name);
	void data(const char *s, int length) { m_data.append(string(s, length)); }

protected:
	EnviroOptions &m_opt;
	std::string m_data;
};

void s2b(std::string &s, bool &b)
{
	if (s[0] == '0') b = false;
	if (s[0] == '1') b = true;
}

void EnviroOptionsVisitor::endElement(const char *name)
{
	const char *str = m_data.c_str();

	if (strcmp(name, STR_DATAPATH) == 0)
		m_opt.m_oldDataPaths.push_back(str);

	else if (strcmp(name, STR_EARTHVIEW) == 0)
		s2b(m_data, m_opt.m_bEarthView);
	else if (strcmp(name, STR_EARTHIMAGE) == 0)
		m_opt.m_strEarthImage = str;
	else if (strcmp(name, STR_INITTERRAIN) == 0)
		m_opt.m_strInitTerrain = str;

	else if (strcmp(name, STR_FULLSCREEN) == 0)
		s2b(m_data, m_opt.m_bFullscreen);
	else if (strcmp(name, STR_STEREO) == 0)
		s2b(m_data, m_opt.m_bStereo);
	else if (strcmp(name, STR_STEREO_MODE) == 0)
		m_opt.m_iStereoMode = atoi(str);
	else if (strcmp(name, STR_WINLOC) == 0)
	{
		sscanf(str, "%d %d %d %d", &m_opt.m_WinPos.x, &m_opt.m_WinPos.y,
			&m_opt.m_WinSize.x, &m_opt.m_WinSize.y);
	}
	else if (strcmp(name, STR_LOCINSIDE) == 0)
		s2b(m_data, m_opt.m_bLocationInside);
	else if (strcmp(name, STR_HTMLPANE) == 0)
		s2b(m_data, m_opt.m_bHtmlpane);
	else if (strcmp(name, STR_FLOATBAR) == 0)
		s2b(m_data, m_opt.m_bFloatingToolbar);
	else if (strcmp(name, STR_TEXTURE_COMPRESSION) == 0)
		s2b(m_data, m_opt.m_bTextureCompression);
	else if (strcmp(name, STR_DISABLE_MODEL_MIPMAPS) == 0)
		s2b(m_data, m_opt.m_bDisableModelMipmaps);

	else if (strcmp(name, STR_PLANTSIZE) == 0)
		m_opt.m_fPlantScale = (float) atof(str);
	else if (strcmp(name, STR_PLANTSHADOWS) == 0)
		s2b(m_data, m_opt.m_bShadows);
	else if (strcmp(name, STR_ONLY_AVAILABLE_SPECIES) == 0)
		s2b(m_data, m_opt.m_bOnlyAvailableSpecies);

	else if (strcmp(name, STR_DIRECT_PICKING) == 0)
		s2b(m_data, m_opt.m_bDirectPicking);
	else if (strcmp(name, STR_SELECTIONCUTOFF) == 0)
		m_opt.m_fSelectionCutoff = (float) atof(str);
	else if (strcmp(name, STR_MAX_INST_RADIUS) == 0)
		m_opt.m_fMaxPickableInstanceRadius = (float) atof(str);
	else if (strcmp(name, STR_CURSOR_THICKNESS) == 0)
		m_opt.m_fCursorThickness = (float) atof(str);
	else if (strcmp(name, STR_CATENARY_FACTOR) == 0)
		m_opt.m_fCatenaryFactor = (float) atof(str);

	else if (strcmp(name, STR_CONTENT_FILE) == 0)
		m_opt.m_strContentFile = str;
	else if (strcmp(name, STR_SHOW_PROGRESS) == 0)
		s2b(m_data, m_opt.m_bShowProgress);
	else if (strcmp(name, STR_FLY_IN) == 0)
		s2b(m_data, m_opt.m_bFlyIn);

	else if (strcmp(name, STR_TB_CULTURE) == 0)
		s2b(m_data, m_opt.m_bShowToolsCulture);
	else if (strcmp(name, STR_TB_SNAPSHOT) == 0)
		s2b(m_data, m_opt.m_bShowToolsSnapshot);
	else if (strcmp(name, STR_TB_TIME) == 0)
		s2b(m_data, m_opt.m_bShowToolsTime);
}

bool EnviroOptions::ReadXML(const char *fname)
{
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	VTLOG("\tReading options from '%s'\n", fname);

	EnviroOptionsVisitor visitor(*this);
	try
	{
		std::string fname2(fname);
		readXML(fname2, visitor);
	}
	catch (xh_io_exception &ex)
	{
		const string msg = ex.getFormattedMessage();
		VTLOG(" XML problem: %s\n", msg.c_str());
		return false;
	}
	m_strFilename = fname;
	return true;
}

void WriteElem(ofstream &output, const char *elem, const char *value)
{
	output << "\t<" << elem << ">";
	output << value;
	output << "</" << elem << ">\n";
}

void WriteElemB(ofstream &output, const char *elem, bool value)
{
	output << "\t<" << elem << ">";
	output << value;
	output << "</" << elem << ">\n";
}

void WriteElemI(ofstream &output, const char *elem, int value)
{
	output << "\t<" << elem << ">";
	output << value;
	output << "</" << elem << ">\n";
}

void WriteElemF(ofstream &output, const char *elem, float value)
{
	output << "\t<" << elem << ">";
	output << value;
	output << "</" << elem << ">\n";
}

bool EnviroOptions::WriteXML()
{
	VTLOG("Writing options to '%s'\n", (const char *) m_strFilename);

	ofstream output(m_strFilename, ios::binary);
	if (!output.is_open())
	{
		vtString msg;
		msg = "Couldn't write settings to file \"";
		msg += m_strFilename;
		msg += "\"\nPlease make sure it's writable.\n";
		VTLOG(msg);
		return false;
	}

	// write to file
	output << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
	output << "<EnviroOptions>" << std::endl;

	// Don't write data paths here, they are now written to vtp.xml
	//for (unsigned int i = 0; i < m_oldDataPaths.size(); i++)
	//	WriteElem(output, STR_DATAPATH, m_oldDataPaths[i]);

	WriteElemB(output, STR_EARTHVIEW, m_bEarthView);
	WriteElem(output, STR_EARTHIMAGE, m_strEarthImage);
	WriteElem(output, STR_INITTERRAIN, m_strInitTerrain);
	WriteElemB(output, STR_FULLSCREEN, m_bFullscreen);
	WriteElemB(output, STR_STEREO, m_bStereo);
	WriteElemI(output, STR_STEREO_MODE, m_iStereoMode);

	vtString winpos;
	winpos.Format("%d %d %d %d", m_WinPos.x, m_WinPos.y, m_WinSize.x, m_WinSize.y);
	WriteElem(output, STR_WINLOC, winpos);

	WriteElemB(output, STR_LOCINSIDE, m_bLocationInside);
	WriteElemB(output, STR_HTMLPANE, m_bHtmlpane);
	WriteElemB(output, STR_FLOATBAR, m_bFloatingToolbar);
	WriteElemB(output, STR_TEXTURE_COMPRESSION, m_bTextureCompression);
	WriteElemF(output, STR_PLANTSIZE, m_fPlantScale);
	WriteElemB(output, STR_PLANTSHADOWS, m_bShadows);
	WriteElemB(output, STR_ONLY_AVAILABLE_SPECIES, m_bOnlyAvailableSpecies);
	WriteElemB(output, STR_DIRECT_PICKING, m_bDirectPicking);
	WriteElemF(output, STR_SELECTIONCUTOFF, m_fSelectionCutoff);
	WriteElemB(output, STR_DISABLE_MODEL_MIPMAPS, m_bDisableModelMipmaps);
	WriteElemF(output, STR_CURSOR_THICKNESS, m_fCursorThickness);
	WriteElemF(output, STR_CATENARY_FACTOR, m_fCatenaryFactor);
	WriteElem(output, STR_CONTENT_FILE, m_strContentFile);
	WriteElemF(output, STR_MAX_INST_RADIUS, m_fMaxPickableInstanceRadius);
	WriteElemB(output, STR_SHOW_PROGRESS, m_bShowProgress);
	WriteElemB(output, STR_FLY_IN, m_bFlyIn);
	WriteElemB(output, STR_TB_CULTURE, m_bShowToolsCulture);
	WriteElemB(output, STR_TB_SNAPSHOT, m_bShowToolsSnapshot);
	WriteElemB(output, STR_TB_TIME, m_bShowToolsTime);

	output << "</EnviroOptions>" << std::endl;

	return true;
}

