//
// Helper.cpp - various helper functions used by the classes
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/progdlg.h"
#include "vtdata/MathTypes.h"
#include "Helper.h"

//////////////////////////////////////////////////////

wxProgressDialog *g_pProg = NULL;
		
void progress_callback(int amount)
{
	if (g_pProg)
		g_pProg->Update(amount);
}

void OpenProgressDialog(const char *title1)
{
	// force the window to be wider by giving a dummy string
	wxString message = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
	wxString title = title1;
	g_pProg = new wxProgressDialog(title, message, 100);
	g_pProg->Show(TRUE);
	g_pProg->Update(0, " ");
}

void CloseProgressDialog()
{
	if (g_pProg)
	{
		g_pProg->Destroy();
		g_pProg = NULL;
	}
}

void UpdateProgressDialog(int amount, const wxString& newmsg)
{
	if (g_pProg)
		g_pProg->Update(amount, newmsg);
}


/////////////////////////////////

wxString FormatCoord(bool bGeo, double val, bool minsec)
{
	wxString str;
	if (bGeo)
	{
		if (minsec)
		{
			// show minutes and seconds
			double degree = val;
			double min = (degree - (int)degree) * 60.0f;
			double sec = (min - (int)min) * 60.0f;

			str.Printf("%d° %d' %.1f\"", (int)degree, (int)min, sec);
		}
		else
			str.Printf("%3.6lf", val);	// decimal degrees
	}
	else
		str.Printf("%8.2lf", val);	// meters-based
	return str;
}


//////////////////////////////////

void IncreaseRect(wxRect &rect, int adjust)
{
	rect.y -= adjust;
	rect.height += (adjust<<1);
	rect.x -= adjust;
	rect.width += (adjust<<1);
}

//////////////////////////////////////

int GuessZoneFromLongitude(double longitude)
{
	return (int) (((longitude + 180.0) / 6.0) + 1.0);
}

