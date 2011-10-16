//
// DataPath.cpp
//
// Copyright (c) 2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "DataPath.h"
#include "FilePath.h"
#include "vtLog.h"

// globals
static vtStringArray s_datapath;
static vtString s_configfile;

///////////////////////////////////////////////////////////////////////
// XML format

#include "xmlhelper/easyxml.hpp"
#define STR_DATAPATH "DataPath"

class PathsVisitor : public XMLVisitor
{
public:
	PathsVisitor() {}
	void startElement(const char *name, const XMLAttributes &atts) { m_data = ""; }
	void endElement (const char *name)
	{
		const char *str = m_data.c_str();
		if (strcmp(name, STR_DATAPATH) == 0)
			s_datapath.push_back(str);
	}
	void data(const char *s, int length) { m_data.append(string(s, length)); }

protected:
	std::string m_data;
};


////////////////////////////////////////////////

void vtSetDataPath(const vtStringArray &paths)
{
	s_datapath = paths;
}

vtStringArray &vtGetDataPath()
{
	return s_datapath;
}

bool vtLoadDataPath(const char *user_config_dir, const char *config_dir)
{
	// Look for the config file which has the datapaths.
	// There are two supported places for it.
	//  1. In the same directory as the executable.
	//  2. On Windows, in the user's "Application Data" folder.
	vtString BaseName = "vtp.xml";
	vtString FileName = BaseName;

	vtString AppDataUser(user_config_dir ? user_config_dir : "");
	vtString AppDataCommon(config_dir ? config_dir : "");

	VTLOG("Looking for '%s'\n", (const char *) FileName);
	bool bFound = vtFileExists(FileName);
	if (!bFound)
	{
		VTLOG1("Not found.\n");
		FileName = AppDataUser + "/" + BaseName;
		VTLOG("Looking for '%s'\n", (const char *) FileName);
		bFound = vtFileExists(FileName);
	}
	if (!bFound)
	{
		VTLOG1("Not found.\n");
		FileName = AppDataCommon + "/" + BaseName;
		VTLOG("Looking for '%s'\n", (const char *) FileName);
		bFound = vtFileExists(FileName);
	}

	// Not found anywhere.  Default to default data path
	if (!bFound)
	{
		VTLOG1("Not found.\n");
		return false;
	}

	VTLOG("\tReading datapaths from '%s'\n", (const char*)FileName);

	PathsVisitor visitor;
	try
	{
		std::string fname2(FileName);
		readXML(fname2, visitor);
	}
	catch (xh_io_exception &ex)
	{
		const string msg = ex.getFormattedMessage();
		VTLOG(" XML problem: %s\n", msg.c_str());
		return false;
	}
	VTLOG("\tSuccessfully read.\n");

	// Remember where we loaded it from
	s_configfile = FileName;

	// Supply the special symbols {appdata} and {appdatacommon}
	for (unsigned int i = 0; i < s_datapath.size(); i++)
	{
		s_datapath[i].Replace("{appdata}", AppDataUser);
		s_datapath[i].Replace("{appdatacommon}", AppDataCommon);
	}

	VTLOG1("Loaded Datapaths:\n");
	unsigned int n = s_datapath.size();
	if (n == 0)
		VTLOG("   none.\n");
	for (unsigned int i = 0; i < n; i++)
		VTLOG("   %s\n", (const char *) s_datapath[i]);
	VTLOG1("\n");
	return true;
}

bool vtSaveDataPath(const char *fname)
{
	vtString filename;
	if (fname)
	{
		filename = fname;

		// Remember where we're saving it to
		s_configfile = fname;
	}
	else
		filename = s_configfile;

	VTLOG("Writing datapaths to '%s'\n", (const char *) filename);

	FILE *fp = vtFileOpen(filename, "wb");
	if (!fp)
	{
		vtString msg;
		msg = "Couldn't write settings to file \"";
		msg += filename;
		msg += "\"\nPlease make sure it's writable.\n";
		VTLOG(msg);
		return false;
	}

	// write to file
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	fprintf(fp, "<VTP>\n");
	for (unsigned int i = 0; i < s_datapath.size(); i++)
	{
		fprintf(fp, "\t<" STR_DATAPATH ">");
		fputs(s_datapath[i], fp);
		fprintf(fp, "</" STR_DATAPATH ">\n");
	}
	fprintf(fp, "</VTP>\n");
	fclose(fp);
	return true;
}
