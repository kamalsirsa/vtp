//
// Some useful standalone functions for use with wxWindows.
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

int AddFilenamesToComboBox(wxComboBox *box, const char *directory,
	const char *wildcard, int omit_chars = 0);
int AddFilenamesToChoice(wxChoice *choice, const char *directory,
	const char *wildcard, int omit_chars = 0);
bool LogWindowsVersion();
vtString FormatCoord(bool bGeo, double val, bool minsec = false);

/////

