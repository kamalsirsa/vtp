//
// wxString2.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxString2.h"

// Construction
#if wxUSE_UNICODE
wxString2::wxString2(const char *psz)
{
	*this = psz;
}
#endif

wxString2::wxString2(const wxChar *psz)
{
	(void) wxString::operator=(psz);
}

wxString2::wxString2(const vtString &vtstr)
{
	*this = (const char *) vtstr;
}

// Assignment
#if wxUSE_UNICODE
wxString2& wxString2::operator=(const char *psz)
{
	int len = strlen(psz);
	wxChar *buf = new wchar_t[len+1];
	int result = wxMB2WC(buf, psz, len+1);
	(void) wxString::operator=(buf);
	delete buf;
	return *this;
}
#endif

wxString2& wxString2::operator=(const wxChar *psz)
{
	(void) wxString::operator=(psz);
	return *this;
}

wxString2& wxString2::operator=(const vtString &vtstr)
{
	*this = (const char *) vtstr;
	return *this;
}

wxString2::operator vtString() const
{
	return vtString(mb_str());
}

wxString2::operator const char*() const
{
	return mb_str();
}

#if wxUSE_UNICODE
char wxString2::s_buffer[MAX_WXSTRING2_SIZE];
const char *wxString2::mb_str() const
{
	int result = wxWC2MB(s_buffer, c_str(), MAX_WXSTRING2_SIZE);
	return s_buffer;
}
#endif

