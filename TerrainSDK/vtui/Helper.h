//
// Some useful standalone functions for use with wxWindows.
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtdata/vtString.h"
#include "vtdata/MathTypes.h"

wxBitmap *MakeColorBitmap(int xsize, int ysize, wxColour color);
void FillWithColor(wxStaticBitmap *pStaticBitmap, const RGBi &color);
void FillWithColor(wxBitmapButton *pBitmapButton, const RGBi &color);
void FillWithColor(wxStaticBitmap *pStaticBitmap, const wxColour &color);
void FillWithColor(wxBitmapButton *pBitmapButton, const wxColour &color);

int AddFilenamesToComboBox(wxComboBox *box, const char *directory,
	const char *wildcard, int omit_chars = 0);
int AddFilenamesToChoice(wxChoice *choice, const char *directory,
	const char *wildcard, int omit_chars = 0);
int AddFilenamesToStringArray(vtStringArray &array, const char *directory,
	const char *wildcard, int omit_chars = 0);
bool LogWindowsVersion();
vtString FormatCoord(bool bGeo, double val, bool minsec = false);
enum wxLanguage GetLangFromName(const wxString &name);

////////

extern bool progress_callback(int amount);
void OpenProgressDialog(const wxString &title, bool bCancellable = false, wxWindow *pParent = NULL);
void CloseProgressDialog();
bool UpdateProgressDialog(int amount, const wxString& newmsg = wxT(""));
void ResumeProgressDialog();

/////

