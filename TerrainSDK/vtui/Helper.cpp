//
// Some useful standalone functions for use with wxWindows.
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxString2.h"
#include "vtdata/FilePath.h"	// for dir_iter
#include "vtdata/vtLog.h"

//
// This function is used to find all files in a given directory,
// and if they match a wildcard, add them to a combo box.
//
int AddFilenamesToComboBox(wxComboBox *box, const char *directory,
	const char *wildcard, int omit_chars)
{
//  VTLOG(" AddFilenamesToComboBox '%s', '%s':", directory, wildcard);

	int entries = 0, matches = 0;

	wxString2 wildstr = wildcard;
	for (dir_iter it((const char *)directory); it != dir_iter(); ++it)
	{
		entries++;
		std::string name1 = it.filename();
		//	VTLOG("   entry: '%s'", name1.c_str());
		if (it.is_hidden() || it.is_directory())
			continue;

		wxString2 name = name1.c_str();
		if (name.Matches(wildstr))
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

//
// This function is used to find all files in a given directory,
// and if they match a wildcard, add them to a combo box.
//
int AddFilenamesToChoice(wxChoice *choice, const char *directory,
	const char *wildcard, int omit_chars)
{
//  VTLOG(" AddFilenamesToComboBox '%s', '%s':", directory, wildcard);

	int entries = 0, matches = 0;

	wxString2 wildstr = wildcard;
	for (dir_iter it((const char *)directory); it != dir_iter(); ++it)
	{
		entries++;
		std::string name1 = it.filename();
		//	VTLOG("   entry: '%s'", name1.c_str());
		if (it.is_hidden() || it.is_directory())
			continue;

		wxString2 name = name1.c_str();
		if (name.Matches(wildstr))
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


