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
#include "Frame.h"
#include "BuilderView.h"
#include "Helper.h"
#include "vtdata/vtLog.h"

//////////////////////////////////////////////////////

static bool s_bOpen = false;
wxProgressDialog *g_pProg = NULL;

void progress_callback(int amount)
{
	if (g_pProg)
		g_pProg->Update(amount);
}

void OpenProgressDialog(const wxString &title, bool bCancellable)
{
	if (s_bOpen)
		return;

	BuilderView *pView = GetMainFrame()->GetView();

	// force the window to be wider by giving a dummy string
	wxString message = _T("___________________________________");
	int style = wxPD_AUTO_HIDE | wxPD_APP_MODAL;
	if (bCancellable)
		style |= wxPD_CAN_ABORT;

	s_bOpen = true;
	g_pProg = new wxProgressDialog(title, message, 100, pView, style);
	g_pProg->Show(true);
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

void ResumeProgressDialog()
{
	if (g_pProg)
		g_pProg->Resume();
}

//
// returns true if the user pressed the "Cancel" button
//
bool UpdateProgressDialog(int amount, const wxString& newmsg)
{
	bool value = false;
	if (g_pProg)
		value = (g_pProg->Update(amount, newmsg) == false);
	return value;
}


//////////////////////////////////

void IncreaseRect(wxRect &rect, int adjust)
{
	rect.y -= adjust;
	rect.height += (adjust<<1);
	rect.x -= adjust;
	rect.width += (adjust<<1);
}

void DrawRectangle(wxDC* pDC, const wxRect &rect)
{
	int left = rect.x;
	int right = rect.x + rect.GetWidth();
	int top = rect.y;
	int bottom = rect.y + rect.GetHeight();
	wxPoint p[5];
	p[0].x = left;
	p[0].y = bottom;

	p[1].x = left;
	p[1].y = top;

	p[2].x = right;
	p[2].y = top;

	p[3].x = right;
	p[3].y = bottom;

	p[4].x = left;
	p[4].y = bottom;
	pDC->DrawLines(5, p);

	pDC->DrawLine(left, bottom, right, top);
	pDC->DrawLine(left, top, right, bottom);
}

//////////////////////////////////////

int GuessZoneFromLongitude(double longitude)
{
	return (int) (((longitude + 180.0) / 6.0) + 1.0);
}

//////////////////////////////////////

//
// Display a message to the user, and also send it to the log file.
//
void DisplayAndLog(const char *pFormat, ...)
{
	va_list va;
	va_start(va, pFormat);

	char ach[2048];
	vsprintf(ach, pFormat, va);

	wxString2 msg = ach;
	wxMessageBox(msg);

	strcat(ach, "\n");
	g_Log._Log(ach);
}

//////////////////////////////////////

#if WIN32

//
// Win32 allows us to do a real StrectBlt operation, although it still won't
// do a StretchBlt with a mask.
//
void wxDC2::StretchBlit(const wxBitmap &bmp,
						wxCoord x, wxCoord y,
						wxCoord width, wxCoord height,
						wxCoord src_x, wxCoord src_y,
						wxCoord src_width, wxCoord src_height)
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
	::StretchBlt( cdc, x, y, width, height, memdc, src_x, src_y, src_width, src_height, SRCCOPY);

	::SelectObject( memdc, hOldBitmap );
	::DeleteDC( memdc );

	::SetTextColor(cdc, old_textground);
	::SetBkColor(cdc, old_background);
}

#endif // WIN32

