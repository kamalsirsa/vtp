//
// Options.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include <string.h>
#include "Options.h"

#include <fstream>
using namespace std;

extern vtString get_line_from_stream(ifstream &input);

EnviroOptions g_Options;

#define STR_DATAPATH "DataPath"
#define STR_EARTHVIEW "EarthView"
#define STR_EARTHIMAGE "EarthImage"
#define STR_INITTERRAIN "InitialTerrain"
#define STR_FULLSCREEN "FullScreen"
#define STR_GRAVITY "Gravity"
#define STR_HTMLPANE "HTMLPane"
#define STR_FLOATBAR "FloatingToolBar"
#define STR_SOUND "Sound"
#define STR_VCURSOR "VirtualCursor"
#define STR_SPEEDTEST "SpeedTest"
#define STR_QUAKE "QuakeNavigation"
#define STR_PLANTSIZE "PlantSize"
#define STR_PLANTSHADOWS "PlantShadows"

EnviroOptions::EnviroOptions()
{
	m_strDataPath = "Data/";
	m_strImage = "free_512";
}

bool EnviroOptions::Read(const char *szFilename)
{
	m_strFilename = szFilename;

//	ifstream input(m_strFilename, ios::nocreate | ios::binary);
//	ifstream input(m_strFilename, ios_base::in | ios_base::binary);
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
			m_strDataPath = get_line_from_stream(input);
		else if (strcmp(buf, STR_EARTHVIEW) == 0)
			input >> m_bEarthView;
		else if (strcmp(buf, STR_EARTHIMAGE) == 0)
			m_strImage = get_line_from_stream(input);
		else if (strcmp(buf, STR_INITTERRAIN) == 0)
			m_strInitTerrain = get_line_from_stream(input);
		else if (strcmp(buf, STR_FULLSCREEN) == 0)
			input >> m_bFullscreen;
		else if (strcmp(buf, STR_GRAVITY) == 0)
			input >> m_bGravity;
		else if (strcmp(buf, STR_HTMLPANE) == 0)
			input >> m_bHtmlpane;
		else if (strcmp(buf, STR_FLOATBAR) == 0)
			input >> m_bFloatingToolbar;
		else if (strcmp(buf, STR_SOUND) == 0)
			input >> m_bSound;
		else if (strcmp(buf, STR_VCURSOR) == 0)
			input >> m_bVCursor;
		else if (strcmp(buf, STR_SPEEDTEST) == 0)
			input >> m_bSpeedTest;
		else if (strcmp(buf, STR_QUAKE) == 0)
			input >> m_bQuakeNavigation;
		else if (strcmp(buf, STR_PLANTSIZE) == 0)
			input >> m_fPlantScale;
		else if (strcmp(buf, STR_PLANTSHADOWS) == 0)
			input >> m_bShadows;
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
	output << STR_DATAPATH << "\t\t";
	output << ((const char *)m_strDataPath) << endl;
	output << STR_EARTHVIEW << "\t\t";
	output << m_bEarthView << endl;
	output << STR_EARTHIMAGE << "\t\t";
	output << ((const char *)m_strImage) << endl;
	output << STR_INITTERRAIN << "\t";
	output << ((const char *)m_strInitTerrain) << endl;

	output << STR_FULLSCREEN << "\t\t";
	output << m_bFullscreen << endl;
	output << STR_GRAVITY << "\t\t\t";
	output << m_bGravity << endl;
	output << STR_HTMLPANE << "\t\t";
	output << m_bHtmlpane << endl;
	output << STR_FLOATBAR << "\t";
	output << m_bFloatingToolbar << endl;

	output << STR_SOUND << "\t\t\t";
	output << m_bSound << endl;
	output << STR_VCURSOR << "\t";
	output << m_bVCursor << endl;
	output << STR_SPEEDTEST << "\t\t";
	output << m_bSpeedTest << endl;
	output << STR_QUAKE << "\t";
	output << m_bQuakeNavigation << endl;
	output << STR_PLANTSIZE << "\t\t";
	output << m_fPlantScale << endl;
	output << STR_PLANTSHADOWS << "\t";
	output << m_bShadows << endl;

	return true;
}

