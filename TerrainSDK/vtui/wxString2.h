//
// wxString2.h
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __wxString2_h__
#define __wxString2_h__

#include "vtdata/vtString.h"

/**
 * The purpose of this class is that wxString does not seem to always return
 * correct C strings when asked, with the mb_str() method.  In particular,
 * with VC++ and Unicode mode, it definitely gives bogus results.  The
 * wxString2 class replaces this one method with an implementation that
 * works reliably.  It also has other benefits including being able to
 * do assignment to and from a normal C string and vtStrings.
 */
class wxString2 : public wxString
{
public:
	// Construction
	wxString2() : wxString() {}
#if wxUSE_UNICODE
	wxString2(const char *psz);
#endif
	wxString2(const wxChar *psz);
	wxString2(const wxString &in) : wxString(in) {}
	wxString2(const vtString &vtstr);

	// Assignment
#if wxUSE_UNICODE
	wxString2& operator=(const char *psz);
#endif
	wxString2& operator=(const wxChar *psz);
	wxString2& operator=(const wxString &str);
	wxString2& operator=(const vtString &vtstr);

    // implicit conversion to vtString
	operator vtString() const;

	// implicit conversion to C string
	operator const char*() const;

#if wxUSE_UNICODE
	// explicit conversion to C string
	const char *mb_str() const;

private:
#define MAX_WXSTRING2_SIZE 2048
	static char s_buffer[MAX_WXSTRING2_SIZE];
#endif
};

#endif // __wxString2_h__
