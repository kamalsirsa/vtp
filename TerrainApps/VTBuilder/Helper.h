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
void OpenProgressDialog(const wxString &title);
void CloseProgressDialog();
void UpdateProgressDialog(int amount, const wxString& newmsg = wxT(""));

/////

wxString FormatCoord(bool bGeo, double val, bool minsec = false);

/////

void IncreaseRect(wxRect &rect, int adjust);

/////

int GuessZoneFromLongitude(double longitude);

#if WIN32
/**
 * Win32 allows us to do a real StrectBlt operation, although it still won't
 * do a StrectBlt with a mask.
 */
class wxDC2 : public wxDC
{
public:
	void StretchBlit(const wxBitmap &bmp, wxCoord x, wxCoord y,
		wxCoord width, wxCoord height);
};
#endif	// WIN32

#endif	// HELPERH

