//
// vtui Helper.h
//  Some useful standalone functions for use with wxWindows.
//
// Copyright (c) 2002-2007 Virtual Terrain Project
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

///////////
// Helper methods which provide a progress dialog.

extern bool progress_callback(int amount);
void OpenProgressDialog(const wxString &title, bool bCancellable = false, wxWindow *pParent = NULL);
void CloseProgressDialog();
bool UpdateProgressDialog(int amount, const wxString& newmsg = wxT(""));
void ResumeProgressDialog();

///////////
// Helper methods which provide a 'double' progress dialog.

extern bool progress_callback2(int amount1, int amount2);
extern bool progress_callback_minor(int amount);
void OpenProgressDialog2(const wxString &title, bool bCancellable = false, wxWindow *pParent = NULL);
void CloseProgressDialog2();
bool UpdateProgressDialog2(int amount1, int amount2, const wxString& newmsg = wxT(""));
void ResumeProgressDialog2();

/////

int GuessZoneFromGeo(const DPoint2 &pos);

/////

vtString MakeFilenameDB(const vtString &base, int col, int row, int relative_lod);

/////
wxString StartOfFilename(const wxString &strFullPath);
wxString ToBackslash(const wxString &path);
void RemoveFileExtensions(wxString &fname, bool bAll = true);

/////

// Display a message to the user, and also send it to the log file.
void DisplayAndLog(const char *pFormat, ...);
#if SUPPORT_WSTRING
void DisplayAndLog(const wchar_t *pFormat, ...);
#endif

// Initialize GDAL/OGR.  If there are problems, report them with a message box and VTLOG.
void CheckForGDALAndWarn();
