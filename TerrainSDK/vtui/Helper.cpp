//
// Some useful standalone functions for use with wxWindows.
//
// Copyright (c) 2002-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/progdlg.h"

#include "DoubleProgDlg.h"

#include "vtdata/FilePath.h"	// for dir_iter
#include "vtdata/Projections.h"
#include "vtdata/vtLog.h"
#include "Helper.h"

wxBitmap *MakeColorBitmap(int xsize, int ysize, wxColour color)
{
	wxImage pImage(xsize, ysize);
	int i, j;
	for (i = 0; i < xsize; i++)
		for (j = 0; j < ysize; j++)
		{
			pImage.SetRGB(i, j, color.Red(), color.Green(), color.Blue());
		}

	wxBitmap *pBitmap = new wxBitmap(pImage);
	return pBitmap;
}

void FillWithColor(wxStaticBitmap *pStaticBitmap, const wxColour &color)
{
	const wxBitmap &bm = pStaticBitmap->GetBitmap();

	wxBitmap *pNewBitmap = MakeColorBitmap(bm.GetWidth(), bm.GetHeight(), color);
	pStaticBitmap->SetBitmap(*pNewBitmap);
	delete pNewBitmap;
}

void FillWithColor(wxBitmapButton *pBitmapButton, const wxColour &color)
{
	const wxBitmap &bm = pBitmapButton->GetBitmapLabel();

	wxBitmap *pNewBitmap = MakeColorBitmap(bm.GetWidth(), bm.GetHeight(), color);
	pBitmapButton->SetBitmapLabel(*pNewBitmap);
	delete pNewBitmap;
}

void FillWithColor(wxStaticBitmap *pStaticBitmap, const RGBi &color)
{
	FillWithColor(pStaticBitmap, wxColour(color.r, color.g, color.b));
}

void FillWithColor(wxBitmapButton *pBitmapButton, const RGBi &color)
{
	FillWithColor(pBitmapButton, wxColour(color.r, color.g, color.b));
}

/**
 * This function is used to find all files in a given directory,
 * and if they match a wildcard, add them to a combo box.
 *
 * The wildcard comparison is case-insensitive, so for example "*.vtst"
 * will match "Foo.vtst" and "BAR.VTST".
 */
int AddFilenamesToChoice(wxChoice *choice, const char *directory,
	const char *wildcard, int omit_chars)
{
//  VTLOG(" AddFilenamesToChoice '%s', '%s':", directory, wildcard);

	int entries = 0, matches = 0;

	wxString wildstr(wildcard, wxConvUTF8);
	wildstr.LowerCase();

	for (dir_iter it((const char *)directory); it != dir_iter(); ++it)
	{
		entries++;
		std::string name1 = it.filename();
		//	VTLOG("   entry: '%s'", name1.c_str());
		if (it.is_hidden() || it.is_directory())
			continue;

		wxString name(name1.c_str(), wxConvUTF8);
		wxString name_lower = name;
		name_lower.LowerCase();

		if (name_lower.Matches(wildstr))
		{
			if (omit_chars)
				choice->Append(name.Left(name.Length()-omit_chars));
			else
				choice->Append(name);
			matches++;
		}
	}
//  VTLOG(" %d entries, %d matches\n", entries, matches);
	return matches;
}

/**
 * This function is used to find all files in a given directory,
 * and if they match a wildcard, add them to a combo box.
 *
 * The wildcard comparison is case-insensitive, so for example "*.vtst"
 * will match "Foo.vtst" and "BAR.VTST".
 */
int AddFilenamesToComboBox(wxComboBox *box, const char *directory,
	const char *wildcard, int omit_chars)
{
//  VTLOG(" AddFilenamesToComboBox '%s', '%s':", directory, wildcard);

	// This does not work on all platforms, because wxComboBox is only a subclass
	//  of wxChoice on some wx flavors.
	//return AddFilenamesToChoice(box, directory, wildcard, omit_chars);

	// Instead, we need the same implementation
	int entries = 0, matches = 0;

	wxString wildstr(wildcard, wxConvUTF8);
	wildstr.LowerCase();

	for (dir_iter it((const char *)directory); it != dir_iter(); ++it)
	{
		entries++;
		std::string name1 = it.filename();
		//	VTLOG("   entry: '%s'", name1.c_str());
		if (it.is_hidden() || it.is_directory())
			continue;

		wxString name(name1.c_str(), wxConvUTF8);
		wxString name_lower = name;
		name_lower.LowerCase();

		if (name_lower.Matches(wildstr))
		{
			if (omit_chars)
				box->Append(name.Left(name.Length()-omit_chars));
			else
				box->Append(name);
			matches++;
		}
	}
//  VTLOG(" %d entries, %d matches\n", entries, matches);
	return matches;
}

/**
 * Read a directory and add all the filenames which match a given wildcard
 * to a string array.
 */
int AddFilenamesToStringArray(vtStringArray &array, const char *directory,
	const char *wildcard, int omit_chars)
{
	int matches = 0;
	for (dir_iter it(directory); it != dir_iter(); ++it)
	{
		if (it.is_hidden() || it.is_directory())
			continue;

		vtString name = it.filename().c_str();
		if (name.Matches(wildcard))
		{
			if (omit_chars)
				array.push_back(name.Left(name.GetLength()-omit_chars));
			else
				array.push_back(name);
			matches++;
		}
	}
	return matches;
}

///////////////////////////////////////////////////////////////////////

#if WIN32

#define BUFSIZE 80

//
// This code comes from Microsoft; it's the gnarly way of finding out
//  at runtime exactly what version of Windows we are running on.
//
bool LogWindowsVersion()
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )
			return false;
	}

	switch (osvi.dwPlatformId)
	{
		// Test for the Windows NT product family.
		case VER_PLATFORM_WIN32_NT:
			// Test for the specific product family.
			if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
				VTLOG("Windows Server&nbsp;2003 family, ");

			if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
				VTLOG("Windows XP ");

			if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
				VTLOG("Windows 2000 ");

			if ( osvi.dwMajorVersion <= 4 )
				VTLOG("Windows NT ");

			// Test for specific product on Windows NT 4.0 SP6 and later.
			if( bOsVersionInfoEx )
			{
				// Test for the workstation type.
				if ( osvi.wProductType == VER_NT_WORKSTATION )
				{
					if( osvi.dwMajorVersion == 4 )
						VTLOG("Workstation 4.0 ");
					else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
						VTLOG("Home Edition ");
					else
						VTLOG("Professional ");
				}

				// Test for the server type.
				else if ( osvi.wProductType == VER_NT_SERVER )
				{
					if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
					{
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
							VTLOG("Datacenter Edition ");
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
							VTLOG("Enterprise Edition ");
						else if ( osvi.wSuiteMask == VER_SUITE_BLADE )
							VTLOG("Web Edition ");
						else
							VTLOG("Standard Edition ");
					}

					else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
					{
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
							VTLOG("Datacenter Server ");
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
							VTLOG("Advanced Server ");
						else
							VTLOG("Server ");
					}

					else  // Windows NT 4.0
					{
						if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
							VTLOG("Server 4.0, Enterprise Edition ");
						else
							VTLOG("Server 4.0 ");
					}
				}
			}
			else  // Test for specific product on Windows NT 4.0 SP5 and earlier
			{
				HKEY hKey;
				TCHAR szProductType[BUFSIZE];
				DWORD dwBufLen=BUFSIZE;
				LONG lRet;

				lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
					_T("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
					0, KEY_QUERY_VALUE, &hKey );
				if( lRet != ERROR_SUCCESS )
					return false;

				lRet = RegQueryValueEx( hKey, _T("ProductType"), NULL, NULL,
					(LPBYTE) szProductType, &dwBufLen);
				if( (lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE) )
					return false;

				RegCloseKey( hKey );

				if ( lstrcmpi( _T("WINNT"), szProductType) == 0 )
					VTLOG( "Workstation ");
				if ( lstrcmpi( _T("LANMANNT"), szProductType) == 0 )
					VTLOG( "Server ");
				if ( lstrcmpi( _T("SERVERNT"), szProductType) == 0 )
					VTLOG( "Advanced Server ");

				VTLOG( "%d.%d ", osvi.dwMajorVersion, osvi.dwMinorVersion );
			}

			// Display service pack (if any) and build number.
			if (osvi.dwMajorVersion == 4 &&
				 lstrcmpi( osvi.szCSDVersion, _T("Service Pack 6") ) == 0 )
			{
				HKEY hKey;
				LONG lRet;

				// Test for SP6 versus SP6a.
				lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
					_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009"),
					0, KEY_QUERY_VALUE, &hKey );
				if( lRet == ERROR_SUCCESS )
					VTLOG( "Service Pack 6a (Build %d)\n", osvi.dwBuildNumber & 0xFFFF );
				else // Windows NT 4.0 prior to SP6a
				{
					VTLOG( "%s (Build %d)\n",
						osvi.szCSDVersion,
						osvi.dwBuildNumber & 0xFFFF);
				}

				RegCloseKey( hKey );
			}
			else // Windows NT 3.51 and earlier or Windows 2000 and later
			{
				VTLOG( "%s (Build %d)\n",
					osvi.szCSDVersion,
					osvi.dwBuildNumber & 0xFFFF);
			}
			break;

		// Test for the Windows 95 product family.
		case VER_PLATFORM_WIN32_WINDOWS:
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
			{
				 VTLOG("Windows 95 ");
				 if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
					 VTLOG("OSR2 ");
			}
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
			{
				 VTLOG("Windows 98 ");
				 if ( osvi.szCSDVersion[1] == 'A' )
					 VTLOG("SE ");
			}
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
			{
				 VTLOG("Windows Millennium Edition\n");
			}
			break;

		case VER_PLATFORM_WIN32s:
			VTLOG("Win32s\n");
			break;
	}
	return true;
}

#endif // WIN32

/////////////////////////////////

vtString FormatCoord(bool bGeo, double val, bool minsec)
{
	vtString str;
	if (bGeo)
	{
		if (minsec)
		{
			// show minutes and seconds
			double degree = val;
			double min = (degree - (int)degree) * 60.0f;
			double sec = (min - (int)min) * 60.0f;

			str.Format("%d° %d' %.1f\"", (int)degree, (int)min, sec);
		}
		else
			str.Format("%3.6lf", val);	// decimal degrees
	}
	else
		str.Format("%.2lf", val);	// meters-based
	return str;
}

/////////////////////////////////

//
// Given a string name of a language like "de" or "German", return the
// enumerated language id, e.g. wxLANGUAGE_GERMAN.
//
enum wxLanguage GetLangFromName(const wxString &name)
{
	int lang;
	for (lang = wxLANGUAGE_ABKHAZIAN; lang < wxLANGUAGE_USER_DEFINED; lang++)
	{
		const wxLanguageInfo *info = wxLocale::GetLanguageInfo(lang);
		if (name.CmpNoCase(info->Description) == 0)
			return (enum wxLanguage) lang;
		if (name.Length() == 2)
		{
			wxString shortname = info->CanonicalName.Left(2);
			if (name.CmpNoCase(shortname) == 0)
				return (enum wxLanguage) lang;
		}
		else if (name.Length() == 5 && name[2] == '_')
		{
			if (name.CmpNoCase(info->CanonicalName) == 0)
				return (enum wxLanguage) lang;
		}
		else if (name.CmpNoCase(info->Description) == 0)
			return (enum wxLanguage) lang;
	}
	return wxLANGUAGE_UNKNOWN;
}


///////////////////////////////////////////////////////////////////////
// Shared Progress Dialog Functionality
//

static bool s_bOpen = false;
wxProgressDialog *g_pProg = NULL;

bool progress_callback(int amount)
{
	bool value = false;
	// Update() returns false if the Cancel button has been pushed
	// but this functions return _true_ if user wants to cancel
	if (g_pProg)
		value = (g_pProg->Update(amount) == false);
	return value;
}

void OpenProgressDialog(const wxString &title, bool bCancellable, wxWindow *pParent)
{
	if (s_bOpen)
		return;

	// force the window to be wider by giving a dummy string
	wxString message = _T("___________________________________");
	int style = wxPD_AUTO_HIDE | wxPD_APP_MODAL;
	if (bCancellable)
		style |= wxPD_CAN_ABORT;
	style |= wxPD_SMOOTH;

	s_bOpen = true;
	g_pProg = new wxProgressDialog(title, message, 100, pParent, style);
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


///////////////////////////////////////////////////////////////////////
// Shared Double Progress Dialog Functionality
//

static bool s_bOpen2 = false;
DoubleProgressDialog *g_pProg2 = NULL;

bool progress_callback2(int amount1, int amount2)
{
	bool value = false;
	// Update() returns false if the Cancel button has been pushed
	// but this functions return _true_ if user wants to cancel
	if (g_pProg2)
		value = (g_pProg2->Update(amount1, amount2) == false);
	return value;
}

bool progress_callback_major(int amount)
{
	bool value = false;
	// Update() returns false if the Cancel button has been pushed
	// but this functions return _true_ if user wants to cancel
	if (g_pProg2)
		value = (g_pProg2->Update(amount, -1) == false);
	return value;
}

bool progress_callback_minor(int amount)
{
	bool value = false;
	// Update() returns false if the Cancel button has been pushed
	// but this functions return _true_ if user wants to cancel
	if (g_pProg2)
		value = (g_pProg2->Update(-1, amount) == false);
	return value;
}

void OpenProgressDialog2(const wxString &title, bool bCancellable, wxWindow *pParent)
{
	if (s_bOpen2)
		return;

	// force the window to be wider by giving a dummy string
	wxString message = _T("___________________________________");
	int style = wxPD_AUTO_HIDE | wxPD_APP_MODAL;
	if (bCancellable)
		style |= wxPD_CAN_ABORT;
	style |= wxPD_SMOOTH;

	s_bOpen2 = true;
	g_pProg2 = new DoubleProgressDialog(title, message, pParent, style);
	g_pProg2->Show(true);
	g_pProg2->Update(0, 0, _T(" "));
}

void CloseProgressDialog2()
{
	if (g_pProg2)
	{
		g_pProg2->Destroy();
		g_pProg2 = NULL;
		s_bOpen2 = false;
	}
}

void ResumeProgressDialog2()
{
	if (g_pProg2)
		g_pProg2->Resume();
}

//
// returns true if the user pressed the "Cancel" button
//
bool UpdateProgressDialog2(int amount1, int amount2, const wxString& newmsg)
{
	bool value = false;
	if (g_pProg2)
		value = (g_pProg2->Update(amount1, amount2, newmsg) == false);
	return value;
}


//////////////////////////////////////

int GuessZoneFromGeo(const DPoint2 &pos)
{
	int zone = (int) (((pos.x + 180.0) / 6.0) + 1.0);
	if (pos.y < 0)
		zone = -zone;
	return zone;
}

//////////////////////////////////////

// Assemble the filepath for a libMini .db file
vtString MakeFilenameDB(const vtString &base, int col, int row, int relative_lod)
{
	vtString fname = base, str;
	fname += '/';
	if (relative_lod == 0)
		str.Format("tile.%d-%d.db", col, row);
	else
		str.Format("tile.%d-%d.db%d", col, row, relative_lod);
	fname += str;
	return fname;
}

/**
 * Given a full path containing a filename, return a pointer to
 * the filename portion of the string.
 */
wxString StartOfFilename(const wxString &strFullPath)
{
	int index = 0;

	int tmp1 = strFullPath.Find('/', true);
	if (tmp1 > index)
		index = tmp1+1;
	int tmp2 = strFullPath.Find('\\', true);
	if (tmp2 > index)
		index = tmp2+1;
	int tmp3 = strFullPath.Find(':', true);
	if (tmp3 > index)
		index = tmp3+1;

	return strFullPath.Mid(index);
}

/**
 * Return a copy of the string that has forward slashes converted to backslashes.
 * This is useful for passing paths and filenames to the file dialog on WIN32.
 */
wxString ToBackslash(const wxString &path)
{
	wxString 	result;
	size_t 	i, len = path.length();
	result.reserve(len);
	for ( i = 0; i < len; i++ )
	{
		wxChar ch = path.GetChar(i);
		switch ( ch )
		{
		case _T('/'):
			ch = _T('\\');	// convert to backslash
			// fall through
		default:
			result += ch;	// normal char
		}
	}
	return result;
}

/**
 * Given a filename (which may include a path), remove any file extension(s)
 * which it may have.
 */
void RemoveFileExtensions(wxString &fname, bool bAll)
{
	for (int i = fname.Length()-1; i >= 0; i--)
	{
		wxChar ch = fname[i];

		// If we hit a path divider, stop
		if (ch == ':' || ch == '\\' || ch == '/')
			break;

		// If we hit a period which indicates an extension, snip
		if (ch == '.')
		{
			fname = fname.Left(i);

			// if we're not snipping all the extensions, stop now
			if (!bAll)
				return;
		}
	}
}

//
// Initialize GDAL/OGR.  If there are problems, report them with a message box and VTLOG.
//
void CheckForGDALAndWarn()
{
	// check for correctly set up environment variables and locatable files
	g_GDALWrapper.Init();
	GDALInitResult *gr = g_GDALWrapper.GetInitResult();

	if (!gr->hasGDAL_DATA)
	{
		vtString msg = "Unable to locate the necessary GDAL files for full coordinate\n"
			" system support. Without these files, many operations won't work.\n";
		DisplayAndLog(msg);
	}
	if (!gr->hasPROJ_LIB)
	{
		vtString msg = "Unable to locate the necessary PROJ.4 files for full coordinate\n"
			" system support. Without these files, many operations won't work.\n";
		DisplayAndLog(msg);
	}
	if (!gr->hasPROJSO)
	{
		vtString msg = "Unable to locate the PROJ.4 shared library for full coordinate\n"
			" system support. Without the file, many operations won't work.\n";
		DisplayAndLog(msg);
	}
	else
	{
		// Test that PROJ4 is actually working.
		if (!g_GDALWrapper.TestPROJ4())
			DisplayAndLog("Unable to transform coordinates.  This may be because the shared\n"
				"library for PROJ.4 is not found.  Without this, many operations won't work.");
	}
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

	wxString msg(ach, wxConvUTF8);
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
//	wxString trans = wxGetTranslation(pFormat);
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

	wxString msg(ach);
	wxMessageBox(msg);

	VTLOG1(ach);
	VTLOG1("\n");
}
#endif // SUPPORT_WSTRING


