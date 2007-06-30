//
// Charset.cpp : help with CString and character sets
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"
#include "Charset.h"
#include <wchar.h>

#ifdef _UNICODE

CString FromUTF8(const char *input)
{
	int len = strlen(input);
	wchar_t *unicode = new wchar_t[len+1];
	int  wcharlen = MultiByteToWideChar(
		CP_UTF8,
		0,
		input,
		len+1,		// include NULL terminator
		unicode,
		len+1);
	CString str(unicode);
	delete [] unicode;
	return str;
}

vtString ToUTF8(const CString &cstr)
{
	int len = cstr.GetLength();
	char *mbs = new char[len+1];

	int mblen = WideCharToMultiByte(
		CP_UTF8,
		0,
		cstr,
		len+1,
		mbs,
		len+1,
		NULL,
		NULL);

	vtString str = mbs;
	delete [] mbs;
	return str;
}

#else	// MBCS version (which mean ANSI, apparently)

CString FromUTF8(const char *input)
{
	int len = strlen(input);

	wchar_t *unicode = new wchar_t[len+1];
	char *mbs = new char[len+1];

	int  wcharlen = MultiByteToWideChar(
		CP_UTF8,
		0,
		input,
		len+1,		// include NULL terminator
		unicode,
		len+1);

	int mblen = WideCharToMultiByte(
		CP_ACP,
		0,
		unicode,
		wcharlen+1,
		mbs,
		len+1,
		"?",
		NULL);

	CString str(mbs);
	delete [] mbs;
	delete [] unicode;
	return str;
}

vtString ToUTF8(const CString &cstr)
{
	int len = cstr.GetLength();

	wchar_t *unicode = new wchar_t[len+1];
	char *mbs = new char[len+1];

	int  wcharlen = MultiByteToWideChar(
		CP_ACP,
		0,
		cstr,
		len+1,		// include NULL terminator
		unicode,
		len+1);

	int mblen = WideCharToMultiByte(
		CP_UTF8,
		0,
		unicode,
		-1,
		mbs,
		len+1,
		NULL,
		NULL);

	vtString str(mbs);
	delete [] mbs;
	delete [] unicode;
	return str;
}

#endif
