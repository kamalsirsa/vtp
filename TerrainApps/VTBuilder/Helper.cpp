//
// Helper.cpp - various helper functions used by the classes
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Frame.h"
#include "BuilderView.h"
#include "Helper.h"
#include "vtdata/vtLog.h"

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

void AddType(wxString &str, const wxString &filter)
{
	// Chop up the input string.  Expected form is "str1|str2|str3"
	wxString str1 = str.BeforeFirst('|');

	wxString str2 = str.AfterFirst('|');
	str2 = str2.BeforeFirst('|');

	wxString str3 = str.AfterFirst('|');
	str3 = str3.AfterFirst('|');

	// Chop up the filter string.  str4 is the wildcard part.
	wxString str4 = filter.AfterFirst('|');

	// Now rebuild the string, with the filter added
	wxString output = str1 + _T("|");
	output += str2;
	if (str2.Len() > 1)
		output += _T(";");
	output += str4;
	output += _T("|");
	if (str3.Len() > 1)
	{
		output += str3;
		output += _T("|");
	}
	output += filter;

	str = output;
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
	VTLOG1(ach);
}

#if SUPPORT_WSTRING
//
// Also wide-character version of the same function.
//
void DisplayAndLog(const wchar_t *pFormat, ...)
{
//#ifdef UNICODE
//	// Try to translate the string
//	wxString2 trans = wxGetTranslation(pFormat);
//	pFormat = trans.c_str();
//#endif

	va_list va;
	va_start(va, pFormat);

	// Use wide characters
	wchar_t ach[2048];
#ifdef _MSC_VER
	vswprintf(ach, pFormat, va);
#else
	// apparently on non-MSVC platforms this takes 4 arguments (safer)
	vswprintf(ach, 2048, pFormat, va);
#endif

	wxString2 msg = ach;
	wxMessageBox(msg);

	VTLOG1(ach);
	VTLOG1("\n");
}
#endif // SUPPORT_WSTRING


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

//	int bwidth = bmp.GetWidth(), bheight = bmp.GetHeight();
	::StretchBlt( cdc, x, y, width, height, memdc, src_x, src_y, src_width, src_height, SRCCOPY);

	::SelectObject( memdc, hOldBitmap );
	::DeleteDC( memdc );

	::SetTextColor(cdc, old_textground);
	::SetBkColor(cdc, old_background);
}

#endif // WIN32

