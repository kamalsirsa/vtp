//
// TParams.cpp
//
// Defines all the construction parameters for a terrain.
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"
#include "vtdata/vtTime.h"
#include <string.h>
#include "TParams.h"

#include <fstream>
using namespace std;

//---------------------------------------------------------------------------
// Some string for backward compatibility with older (.ini) files
#define STR_BUILDINGFILE "Building_File"
#define STR_FOGCOLORR "Fog_Color_R"
#define STR_FOGCOLORG "Fog_Color_G"
#define STR_FOGCOLORB "Fog_Color_B"
#define STR_16BIT "16_Bit"

// default (currently fixed) number of tiles
#define NTILES	4

TParams::TParams() : vtTagArray()
{
	// Define tags and provide default values
	AddTag(STR_NAME, "");
	AddTag(STR_ELEVFILE, "");
	AddTag(STR_VERTICALEXAG, "1.0");

	AddTag(STR_MINHEIGHT, "20");
	AddTag(STR_NAVSTYLE, "0");
	AddTag(STR_NAVSPEED, "100");
	AddTag(STR_LOCFILE, "");
	AddTag(STR_INITLOCATION, "");
	AddTag(STR_HITHER, "5");
	AddTag(STR_ACCEL, "false");
	AddTag(STR_ALLOW_ROLL, "false");

	AddTag(STR_LODMETHOD, "0");
	AddTag(STR_PIXELERROR, "2.0");
	AddTag(STR_TRICOUNT, "10000");
	AddTag(STR_TRISTRIPS, "true");

	AddTag(STR_TIN, "false");

	AddTag(STR_TIMEON, "false");
	AddTag(STR_INITTIME, "104 2 21 10 0 0");	// 2004, spring, 10am
	AddTag(STR_TIMESPEED, "1");

	AddTag(STR_TEXTURE, "0");
	AddTag(STR_NTILES, "4");
	AddTag(STR_TILESIZE, "512");
	AddTag(STR_TEXTURESINGLE, "true");
	AddTag(STR_TEXTUREBASE, "");
	AddTag(STR_TEXTUREFORMAT, "1");
	AddTag(STR_MIPMAP, "false");
	AddTag(STR_REQUEST16BIT, "true");
	AddTag(STR_PRELIGHT, "true");
	AddTag(STR_PRELIGHTFACTOR, "1.0");
	AddTag(STR_CAST_SHADOWS, "false");
	AddTag(STR_COLOR_MAP, "");

	AddTag(STR_DETAILTEXTURE, "false");
	AddTag(STR_DTEXTURE_NAME, "");
	AddTag(STR_DTEXTURE_SCALE, "1");
	AddTag(STR_DTEXTURE_DISTANCE, "1000");

	AddTag(STR_ROADS, "false");
	AddTag(STR_ROADFILE, "");
	AddTag(STR_HWY, "true");
	AddTag(STR_PAVED, "true");
	AddTag(STR_DIRT, "true");
	AddTag(STR_ROADHEIGHT, "2");
	AddTag(STR_ROADDISTANCE, "2");
	AddTag(STR_TEXROADS, "true");
	AddTag(STR_ROADCULTURE, "false");

	AddTag(STR_TREES, "false");
	AddTag(STR_TREEFILE, "2000");		// 2 km
	AddTag(STR_VEGDISTANCE, "5000");	// 5 km

	AddTag(STR_FOG, "false");
	AddTag(STR_FOGDISTANCE, "50");		// 50 km
	AddTag(STR_FOGCOLOR, "-1 -1 -1");	// unset

	AddTag(STR_STRUCTDIST, "2000");		// 2 km
	AddTag(STR_STRUCT_SHADOWS, "false");
	AddTag(STR_SHADOW_REZ, "1024");
	AddTag(STR_SHADOW_DARKNESS, "0.8");
	AddTag(STR_CONTENT_FILE, "");

	AddTag(STR_TOWERS, "false");
	AddTag(STR_TOWERFILE, "");

	AddTag(STR_VEHICLES, "false");
	AddTag(STR_VEHICLESIZE, "1");
	AddTag(STR_VEHICLESPEED, "1");

	AddTag(STR_SKY, "true");
	AddTag(STR_SKYTEXTURE, "");
	AddTag(STR_OCEANPLANE, "false");
	AddTag(STR_OCEANPLANELEVEL, "-20");
	AddTag(STR_DEPRESSOCEAN, "false");
	AddTag(STR_DEPRESSOCEANLEVEL, "-40");
	AddTag(STR_HORIZON, "false");
	AddTag(STR_BGCOLOR, "40 75 124");	// old blue

	AddTag(STR_ROUTEENABLE, "false");	// not used yet
	AddTag(STR_ROUTEFILE, "");			// not used yet

	AddTag(STR_DIST_TOOL_HEIGHT, "5");
	AddTag(STR_HUD_OVERLAY, "");
}

//
// copy constructor
//
TParams::TParams(const TParams &paramsSrc)
{
	*this = paramsSrc;
}

//
// assignment operator
//
TParams &TParams::operator = (const TParams &rhs)
{
	// copy parent class first
	*((vtTagArray*)this) = rhs;

	// this the elements of this class
	m_Layers = rhs.m_Layers;

	return *this;
}


bool TParams::LoadFrom(const char *fname)
{
	vtString ext = GetExtension(fname, false);

	bool success = false;

	// support old ini file format
	if (ext.CompareNoCase(".ini") == 0)
	{
		success = LoadFromIniFile(fname);

		// Automatically save to the new format for them?
		// vtString name = fname;
		// WriteToXML(name.Left(name.GetLength()-4)+".xml", STR_TPARAMS_FORMAT_NAME);
	}
	else if (ext.CompareNoCase(".xml") == 0)
		success = LoadFromXML(fname);

	return success;
}

bool TParams::LoadFromIniFile(const char *filename)
{
	VTLOG("\tReading TParams from '%s'\n", filename);

	ifstream input(filename, ios::in | ios::binary);
	if (!input.is_open())
		return false;

	// read from file
	char buf[80];

	RGBi fog_color(-1,-1,-1);
	int linenum = 0;
	while (!input.eof())
	{
		linenum++;

		if (input.peek() == '\n')
			input.ignore();
		input >> buf;

		// data value should been separated by a tab or space
		int next = input.peek();
		if (next != '\t' && next != ' ')
			continue;
		while (input.peek() == '\t' || input.peek() == ' ')
			input.ignore();

		if (strcmp(buf, STR_NAME) == 0)
			SetValueString(STR_NAME, get_line_from_stream(input));

		// elevation
		else if (strcmp(buf, STR_ELEVFILE) == 0 ||
				 strcmp(buf, STR_VERTICALEXAG) == 0 ||
				 strcmp(buf, STR_SUPPRESS) == 0 ||
				 strcmp(buf, STR_TIN) == 0)
			SetValueString(buf, get_line_from_stream(input));

		// navigation
		else if (strcmp(buf, STR_MINHEIGHT) == 0 ||
				 strcmp(buf, STR_NAVSTYLE) == 0 ||
				 strcmp(buf, STR_NAVSPEED) == 0 ||
				 strcmp(buf, STR_LOCFILE) == 0 ||
				 strcmp(buf, STR_INITLOCATION) == 0 ||
				 strcmp(buf, STR_HITHER) == 0 ||
				 strcmp(buf, STR_ACCEL) == 0)
			SetValueString(buf, get_line_from_stream(input));

		// LOD
		else if (strcmp(buf, STR_LODMETHOD) == 0 ||
				 strcmp(buf, STR_PIXELERROR) == 0 ||
				 strcmp(buf, STR_TRICOUNT) == 0 ||
				 strcmp(buf, STR_TRISTRIPS) == 0)
			SetValueString(buf, get_line_from_stream(input));

		// time
		else if (strcmp(buf, STR_TIMEON) == 0 ||
				 strcmp(buf, STR_INITTIME) == 0 ||
				 strcmp(buf, STR_TIMESPEED) == 0)
			SetValueString(buf, get_line_from_stream(input));

		// texture
		else if (strcmp(buf, STR_TEXTURE) == 0 ||
				 strcmp(buf, STR_NTILES) == 0 ||
				 strcmp(buf, STR_TILESIZE) == 0 ||
				 strcmp(buf, STR_TEXTURESINGLE) == 0 ||
				 strcmp(buf, STR_TEXTUREBASE) == 0 ||
				 strcmp(buf, STR_TEXTUREFORMAT) == 0 ||
				 strcmp(buf, STR_MIPMAP) == 0 ||
				 strcmp(buf, STR_PRELIGHT) == 0 ||
				 strcmp(buf, STR_PRELIGHTFACTOR) == 0 ||
				 strcmp(buf, STR_CAST_SHADOWS) == 0 ||
				 strcmp(buf, STR_COLOR_MAP) == 0)
			SetValueString(buf, get_line_from_stream(input));
		else if (strcmp(buf, STR_16BIT) == 0)
			SetValueString(STR_REQUEST16BIT, get_line_from_stream(input));

		// detail texture
		else if (strcmp(buf, STR_DETAILTEXTURE) == 0 ||
				 strcmp(buf, STR_DTEXTURE_NAME) == 0 ||
				 strcmp(buf, STR_DTEXTURE_SCALE) == 0 ||
				 strcmp(buf, STR_DTEXTURE_DISTANCE) == 0)
			SetValueString(buf, get_line_from_stream(input));

		// culture
		else if (strcmp(buf, STR_ROADS) == 0 ||
				 strcmp(buf, STR_ROADFILE) == 0 ||
				 strcmp(buf, STR_HWY) == 0 ||
				 strcmp(buf, STR_PAVED) == 0 ||
				 strcmp(buf, STR_DIRT) == 0 ||
				 strcmp(buf, STR_ROADHEIGHT) == 0 ||
				 strcmp(buf, STR_ROADDISTANCE) == 0 ||
				 strcmp(buf, STR_TEXROADS) == 0 ||
				 strcmp(buf, STR_ROADCULTURE) == 0)
			SetValueString(buf, get_line_from_stream(input));

		else if (strcmp(buf, STR_TREES) == 0 ||
				 strcmp(buf, STR_TREEFILE) == 0 ||
				 strcmp(buf, STR_VEGDISTANCE) == 0)
			SetValueString(buf, get_line_from_stream(input));

		else if (strcmp(buf, STR_FOG) == 0 ||
				 strcmp(buf, STR_FOGDISTANCE) == 0)
			SetValueString(buf, get_line_from_stream(input));
		else if (strcmp(buf, STR_FOGCOLORR) == 0)
			fog_color.r = (short) atoi(get_line_from_stream(input));
		else if (strcmp(buf, STR_FOGCOLORG) == 0)
			fog_color.g = (short) atoi(get_line_from_stream(input));
		else if (strcmp(buf, STR_FOGCOLORB) == 0)
			fog_color.b = (short) atoi(get_line_from_stream(input));

		else if (strcmp(buf, STR_BUILDINGFILE) == 0 || strcmp(buf, STR_STRUCTFILE) == 0)
		{
			vtString strFile(get_line_from_stream(input));
			if (strFile != "")
			{
				vtTagArray lay;
				lay.SetValueString("Type", "Structure");
				lay.SetValueString("Filename", strFile);
				lay.SetValueBool("Visible", true);
				m_Layers.push_back(lay);
			}
		}
		else if (strcmp(buf, STR_STRUCTDIST) == 0 ||
				 strcmp(buf, STR_STRUCT_SHADOWS) == 0 ||
				 strcmp(buf, STR_SHADOW_REZ) == 0 ||
				 strcmp(buf, STR_CONTENT_FILE) == 0)
			SetValueString(buf, get_line_from_stream(input));

		// sky and ocean
		else if (strcmp(buf, STR_SKY) == 0 ||
				 strcmp(buf, STR_SKYTEXTURE) == 0 ||
				 strcmp(buf, STR_OCEANPLANE) == 0 ||
				 strcmp(buf, STR_OCEANPLANELEVEL) == 0 ||
				 strcmp(buf, STR_DEPRESSOCEAN) == 0 ||
				 strcmp(buf, STR_DEPRESSOCEANLEVEL) == 0 ||
				 strcmp(buf, STR_HORIZON) == 0)
			SetValueString(buf, get_line_from_stream(input));

		// vehicles
		else if (strcmp(buf, STR_VEHICLES) == 0 ||
				 strcmp(buf, STR_VEHICLESIZE) == 0 ||
				 strcmp(buf, STR_VEHICLESPEED) == 0)
			SetValueString(buf, get_line_from_stream(input));

		// utilities
		else if (strcmp(buf, STR_TOWERS) == 0 ||
				 strcmp(buf,STR_TOWERFILE) == 0 ||
				 strcmp(buf, STR_ROUTEFILE) == 0 ||
				 strcmp(buf, STR_ROUTEENABLE) == 0)
			SetValueString(buf, get_line_from_stream(input));
		else if (buf[0] == ';' || buf[0] == 0)
		{
			// ignore comments and empty lines
		}
		else
		{
			VTLOG("\t Ignoring line %d from INI file: '%s'\n", linenum, buf);
			get_line_from_stream(input);
		}
	}
	input.close();

	//
	// Do some value cleanup
	//
	int iVegDistance = GetValueInt(STR_VEGDISTANCE);
	if (iVegDistance > 0 && iVegDistance < 20)	// surely an old km value
		SetValueInt(STR_VEGDISTANCE, iVegDistance * 1000);
	ConvertOldTimeValue();

	SetValueRGBi(STR_FOGCOLOR, fog_color);

	return true;
}

void TParams::ConvertOldTimeValue()
{
	// Convert old time values to new values
	const char *str = GetValueString(STR_INITTIME);
	if (str)
	{
		int one, two;
		int num = sscanf(str, "%d %d", &one, &two);
		if (num == 1)
		{
			vtTime time;
			time.SetTimeOfDay(one, 0, 0);
			SetValueString(STR_INITTIME, time.GetAsString());
		}
	}
}

bool TParams::LoadFromXML(const char *fname)
{
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	VTLOG("\tReading TParams from '%s'\n", fname);

	TParamsVisitor visitor(this);
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

	// Convert old time values to new values
	ConvertOldTimeValue();

	// Remove some obsolete stuff
	RemoveTag("Labels");
	RemoveTag("LabelFile");
	RemoveTag("Label_Field");
	RemoveTag("Label_Height");
	RemoveTag("Label_Size");
	RemoveTag("Overlay");

	return true;
}

void TParamsVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	TagVisitor::startElement(name, atts);
	if (m_level == 2 && !strcmp(name, STR_STRUCTFILE))
	{
		m_bViz = true;
		const char *attval = atts.getValue("Visible");
		if (attval && !strcmp(attval, "false"))
			m_bViz = false;
	}
	else if (m_level == 2 && !strcmp(name, "Layer"))
	{
		m_layer.Clear();
	}
}

void TParamsVisitor::endElement(const char *name)
{
	// Detect and support old "Structure_File" parameter
	if (m_level == 2 && !strcmp(name, STR_STRUCTFILE))
	{
		vtTagArray lay;
		lay.SetValueString("Type", TERR_LTYPE_STRUCTURE, true);
		lay.SetValueString("Filename", m_data.c_str(), true);
		lay.SetValueBool("Visible", m_bViz, true);
		m_pParams->m_Layers.push_back(lay);
		m_level--;
	}
	else if (m_level == 2 && !strcmp(name, "Layer"))
	{
		m_pParams->m_Layers.push_back(m_layer);
		m_level--;
	}
	else if (m_level == 3)
	{
		// Layer properties
		m_layer.SetValueString(name, m_data.c_str(), true);
		m_level--;
	}
	else
		TagVisitor::endElement(name);
}

void TParams::SetLodMethod(LodMethodEnum method)
{
	SetValueInt(STR_LODMETHOD, (int) method);
}

LodMethodEnum TParams::GetLodMethod() const
{
	return (LodMethodEnum) GetValueInt(STR_LODMETHOD);
}

void TParams::SetTextureEnum(TextureEnum tex)
{
	SetValueInt(STR_TEXTURE, (int) tex);
}

TextureEnum TParams::GetTextureEnum() const
{
	return (TextureEnum) GetValueInt(STR_TEXTURE);
}

vtString TParams::CookTextureFilename() const
{
	vtString str = GetValueString(STR_TEXTUREBASE, true);
	vtString str2;
	str2.Format("%d", NTILES * (GetValueInt(STR_TILESIZE)-1) + 1);
	str += str2;

	if (GetValueBool(STR_TEXTUREFORMAT) == 1)
		str += ".jpg";
	else
		str += ".bmp";
	return str;
}

void TParams::SetOverlay(const vtString &fname, int x, int y)
{
	vtString str;
	str.Format("%s,%d,%d", (const char *)fname, x, y);
	SetValueString(STR_HUD_OVERLAY, str, true);
}

bool TParams::GetOverlay(vtString &fname, int &x, int &y) const
{
	vtString ovstring = GetValueString(STR_HUD_OVERLAY);
	if (ovstring == "") return false;
	char buf[256];
	strcpy(buf, ovstring);
	const char *name = strtok(buf, ",");
	const char *xstr = strtok(NULL, ",");
	const char *ystr = strtok(NULL, ",");
	if (!fname || !xstr || !ystr)
		return false;
	fname = name;
	x = atoi(xstr);
	y = atoi(ystr);
	return true;
}

void TParams::WriteOverridesToXML(FILE *fp) const
{
	for (unsigned int i = 0; i < m_Layers.size(); i++)
	{
		const vtTagArray &lay = m_Layers[i];

		fprintf(fp, "\t<Layer>\n");
		lay.WriteToXMLBody(fp, 2);
		fprintf(fp, "\t</Layer>\n");
	}
}

