//
// Helper.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef HELPERH
#define HELPERH

#include "vtdata/Projections.h"
#include "vtdata/MathTypes.h"

extern void progress_callback(int amount);
void OpenProgressDialog(const char *title);
void CloseProgressDialog();
void UpdateProgressDialog(int amount, const wxString& newmsg = wxT(""));

/////

wxString FormatCoord(bool bGeo, double val, bool minsec = false);

/////

void IncreaseRect(wxRect &rect, int adjust);

/////

int GuessZoneFromLongitude(double longitude);

#endif
