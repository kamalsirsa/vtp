//
// Helper.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef HELPERH
#define HELPERH

#include "vtdata/Projections.h"
#include "vtdata/MathTypes.h"

extern void progress_callback(int amount);
void OpenProgressDialog(const wxString &title, bool bCancellable = false);
void CloseProgressDialog();
bool UpdateProgressDialog(int amount, const wxString& newmsg = wxT(""));
void ResumeProgressDialog();

/////

void IncreaseRect(wxRect &rect, int adjust);
void DrawRectangle(wxDC* pDC, const wxRect &rect);

/////

int GuessZoneFromLongitude(double longitude);

/////

// Display a message to the user, and also send it to the log file.
void DisplayAndLog(const char *pFormat, ...);

/////

#if WIN32
/**
 * Win32 allows us to do a real StrectBlt operation, although it still won't
 * do a StrectBlt with a mask.
 */
class wxDC2 : public wxDC
{
public:
	void StretchBlit(const wxBitmap &bmp, wxCoord x, wxCoord y,
		wxCoord width, wxCoord height, wxCoord src_x, wxCoord src_y,
		wxCoord src_width, wxCoord src_height);
};
#endif	// WIN32

#endif	// HELPERH

