//
// Helper.cpp - various helper functions used by the classes
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/progdlg.h"
#include "vtdata/MathTypes.h"
#include "vtdata/vtLog.h"
#include "Helper.h"

//////////////////////////////////////////////////////

static bool s_bOpen = false;
wxProgressDialog *g_pProg = NULL;
		
void progress_callback(int amount)
{
	if (g_pProg)
		g_pProg->Update(amount);
}

void OpenProgressDialog(const wxString &title)
{
	if (s_bOpen)
		return;

	// force the window to be wider by giving a dummy string
	wxString message = _T("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
	s_bOpen = true;
	g_pProg = new wxProgressDialog(title, message, 100);
	g_pProg->Show(TRUE);
	g_pProg->Update(0, _T(" "));
}

void CloseProgressDialog()
{
	if (g_pProg)
	{
		g_pProg->Destroy();
		g_pProg = NULL;
		s_bOpen = false;
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

			str.Printf(_T("%d° %d' %.1f\""), (int)degree, (int)min, sec);
		}
		else
			str.Printf(_T("%3.6lf"), val);	// decimal degrees
	}
	else
		str.Printf(_T("%8.2lf"), val);	// meters-based
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

//////////////////////////////////////

#if WIN32

//
// Win32 allows us to do a real StrectBlt operation, although it still won't
// do a StrectBlt with a mask.
//
void wxDC2::StretchBlit(const wxBitmap &bmp, wxCoord x, wxCoord y,
						wxCoord width, wxCoord height)
{
    wxCHECK_RET( bmp.Ok(), _T("invalid bitmap in wxDC::DrawBitmap") );

	HDC cdc = ((HDC)GetHDC());
    HDC memdc = ::CreateCompatibleDC( cdc );
    HBITMAP hbitmap = (HBITMAP) bmp.GetHBITMAP( );

    COLORREF old_textground = ::GetTextColor(cdc);
    COLORREF old_background = ::GetBkColor(cdc);
    if (m_textForegroundColour.Ok())
    {
        ::SetTextColor(cdc, m_textForegroundColour.GetPixel() );
    }
    if (m_textBackgroundColour.Ok())
    {
        ::SetBkColor(cdc, m_textBackgroundColour.GetPixel() );
    }

    HGDIOBJ hOldBitmap = ::SelectObject( memdc, hbitmap );

	int bwidth = bmp.GetWidth(), bheight = bmp.GetHeight();
    ::StretchBlt( cdc, x, y, width, height, memdc, 0, 0, bwidth, bheight, SRCCOPY);

    ::SelectObject( memdc, hOldBitmap );
    ::DeleteDC( memdc );

    ::SetTextColor(cdc, old_textground);
    ::SetBkColor(cdc, old_background);
}

#endif // WIN32

