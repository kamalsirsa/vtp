//
// Options.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include <string.h>
#include "Options.h"

#include <fstream>
using namespace std;

EnviroOptions g_Options;

#define STR_DATAPATH "DataPath"
#define STR_EARTHVIEW "EarthView"
#define STR_EARTHIMAGE "EarthImage"
#define STR_INITTERRAIN "InitialTerrain"
#define STR_FULLSCREEN "FullScreen"
#define STR_HTMLPANE "HTMLPane"
#define STR_FLOATBAR "FloatingToolBar"
#define STR_TEXTURE_COMPRESSION "TextureCompression"
#define STR_SPEEDTEST "SpeedTest"
#define STR_PLANTSIZE "PlantSize"
#define STR_PLANTSHADOWS "PlantShadows"
#define STR_SELECTIONCUTOFF "SelectionCutoff"
#define STR_DISABLE_MODEL_MIPMAPS "DisableModelMipmaps"

EnviroOptions::EnviroOptions()
{
	m_strImage = "ev11656_512";
	m_fSelectionCutoff = 15.0f;
	m_bStartInNeutral = false;
	m_bDisableModelMipmaps = false;
	m_bTextureCompression = true;
}

EnviroOptions::~EnviroOptions()
{
}

bool EnviroOptions::Read(const char *szFilename)
{
	m_strFilename = szFilename;

	ifstream input(m_strFilename, ios::in | ios::binary);
	if (!input.is_open())
		return false;

	char buf[80];

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
			m_DataPaths.push_back(vtString(get_line_from_stream(input)));
		}
		else if (strcmp(buf, STR_EARTHVIEW) == 0)
			input >> m_bEarthView;
		else if (strcmp(buf, STR_EARTHIMAGE) == 0)
			m_strImage = get_line_from_stream(input);
		else if (strcmp(buf, STR_INITTERRAIN) == 0)
			m_strInitTerrain = get_line_from_stream(input);
		else if (strcmp(buf, STR_FULLSCREEN) == 0)
			input >> m_bFullscreen;
		else if (strcmp(buf, STR_HTMLPANE) == 0)
			input >> m_bHtmlpane;
		else if (strcmp(buf, STR_FLOATBAR) == 0)
			input >> m_bFloatingToolbar;
		else if (strcmp(buf, STR_TEXTURE_COMPRESSION) == 0)
			input >> m_bTextureCompression;
		else if (strcmp(buf, STR_SPEEDTEST) == 0)
			input >> m_bSpeedTest;
		else if (strcmp(buf, STR_PLANTSIZE) == 0)
			input >> m_fPlantScale;
		else if (strcmp(buf, STR_PLANTSHADOWS) == 0)
			input >> m_bShadows;
		else if (strcmp(buf, STR_SELECTIONCUTOFF) == 0)
			input >> m_fSelectionCutoff;
		else if (strcmp(buf, STR_DISABLE_MODEL_MIPMAPS) == 0)
			input >> m_bDisableModelMipmaps;
		else
		{
//			cout << "Input from INI file unrecognized.\n";
			get_line_from_stream(input);
		}
	}
	return true;
}

bool EnviroOptions::Write()
{
	ofstream output(m_strFilename, ios::binary);
	if (!output.is_open())
	{
		vtString msg;
		msg = "Couldn't write settings to file \"";
		msg += m_strFilename;
		msg += "\"\nPlease make sure it's writable.";
//		AfxMessageBox(msg);
//		MessageBox(NULL, msg, "Note", MB_OK);
		return false;
	}

	// write to file
	for (unsigned int i = 0; i < m_DataPaths.size(); i++)
	{
		output << STR_DATAPATH << "\t\t";
		output << (const char *)(m_DataPaths[i]) << endl;
	}
	output << STR_EARTHVIEW << "\t\t";
	output << m_bEarthView << endl;
	output << STR_EARTHIMAGE << "\t\t";
	output << (const char *)m_strImage << endl;
	output << STR_INITTERRAIN << "\t";
	output << (const char *)m_strInitTerrain << endl;

	output << STR_FULLSCREEN << "\t\t";
	output << m_bFullscreen << endl;
	output << STR_HTMLPANE << "\t\t";
	output << m_bHtmlpane << endl;
	output << STR_FLOATBAR << "\t";
	output << m_bFloatingToolbar << endl;

	output << STR_TEXTURE_COMPRESSION << "\t";
	output << m_bTextureCompression << endl;
	output << STR_SPEEDTEST << "\t\t";
	output << m_bSpeedTest << endl;
	output << STR_PLANTSIZE << "\t\t";
	output << m_fPlantScale << endl;
	output << STR_PLANTSHADOWS << "\t";
	output << m_bShadows << endl;
	output << STR_SELECTIONCUTOFF << "\t";
	output << m_fSelectionCutoff << endl;
	output << STR_DISABLE_MODEL_MIPMAPS << "\t";
	output << m_bDisableModelMipmaps << endl;

	return true;
}

