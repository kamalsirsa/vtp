//
// FilePath.cpp
//
// Functions for helping with management of directories and file paths.
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include <errno.h>

#include "FilePath.h"

#ifdef unix
# include <unistd.h>
# include <sys/stat.h>
#else
# include <direct.h>
# include <io.h>
#endif

#if WIN32
#  ifdef _MSC_VER
#	undef mkdir		// replace the one in direct.h that takes 1 param
#	define mkdir(dirname,mode)	_mkdir(dirname)
#	define rmdir(dirname)		_rmdir(dirname)
#	define strdup(str)			_strdup(str)
#	define unlink(fname)		_unlink(fname)
#	define access(path,mode)	_access(path,mode)
#	define vsnprintf			_vsnprintf
#  else
#	define mkdir(dirname,mode) _mkdir(dirname)
#  endif
#else
#  include <utime.h>
#endif

/**
 * The dir_iter class provides a cross-platform way to read directories.
 */
#if WIN32

dir_iter::dir_iter()
{
	m_handle = -1;
}

dir_iter::dir_iter(std::string const &dirname)
{
	m_handle = _findfirst((dirname + "\\*").c_str(), &m_data);
}

dir_iter::~dir_iter()
{
	if (m_handle != -1)
		_findclose(m_handle);
}

bool dir_iter::is_directory()
{
	return (m_data.attrib & _A_SUBDIR) != 0;
}

bool dir_iter::is_hidden()
{
	return (m_data.attrib & _A_HIDDEN) != 0;
}

std::string dir_iter::filename()
{
	return m_data.name;
}

void dir_iter::operator++()
{
	if (m_handle != -1)
	{
		if (_findnext(m_handle, &m_data) == -1)
		{
			_findclose(m_handle);
			m_handle = -1;
		}
	}
}

bool dir_iter::operator!=(const dir_iter &it)
{
	return (m_handle == -1) != (it.m_handle == -1);
}

///////////////////////////////////////////////////////////////////////
#else	// non-WIN32 platforms, i.e. generally Unix

dir_iter::dir_iter()
{
	m_handle = 0;
	m_stat_p = false;
}

dir_iter::dir_iter(std::string const &dirname)
{
	m_handle = opendir(dirname.c_str());
	m_directory = dirname;
	m_stat_p = false;

	if (m_directory[m_directory.size() - 1] != '/')
		m_directory += '/';
	operator++ ();
}

dir_iter::~dir_iter()
{
	if (m_handle)
		closedir(m_handle);
}

bool dir_iter::is_directory()
{
	return S_ISDIR(get_stat().st_mode);
}

bool dir_iter::is_hidden()
{
	return (m_current[0] == '.');
}

std::string dir_iter::filename()
{
	return m_current;
}

void dir_iter::operator++()
{
	if (!m_handle)
		return;

	m_stat_p = false;
	dirent *rc = readdir(m_handle);
	if (rc != 0)
		m_current = rc->d_name;
	else
	{
		m_current = "";
		closedir(m_handle);
		m_handle = 0;
	}
}

bool dir_iter::operator!=(const dir_iter &it)
{
	return (m_handle == 0) != (it.m_handle == 0);
}

#endif	// !WIN32


/**
 * This function will search for a given file on the given paths, returning
 * the full path to the first file which is found (file exists and can be
 * read from).
 *
 * \param paths An array of strings containing the directories to search.
 * Each directory should end with a the trailing slash ("/" or "\")
 *
 * \param filename A filename, which can optionally contain a partial path
 * as well.  Examples: "foo.txt" or "Stuff/foo.txt"
 */
vtString FindFileOnPaths(const vtStringArray &paths, const char *filename)
{
	FILE *fp;

	if (!strcmp(filename, ""))
		return vtString("");

	// it's possible that the filename is already resolvable without
	// searching the data paths
	fp = fopen(filename, "r");
	if (fp != NULL)
	{
		fclose(fp);
		return vtString(filename);
	}

	for (unsigned int i = 0; i < paths.size(); i++)
	{
		vtString fname = paths[i];
		fname += filename;
		fp = fopen((const char *)fname, "r");
		if (fp != NULL)
		{
			fclose(fp);
			return fname;
		}
	}
	return vtString("");
}


/**
 * Recursive make directory.
 * Aborts if there is an ENOENT error somewhere in the middle.
 *
 * \return true if OK, falso on error
 */
bool vtCreateDir(const char *dirname)
{
	char *buffer = strdup(dirname);
	char *p;
	int  len = strlen(buffer);

	if (len <= 0) {
		free(buffer);
		return false;
	}
	if (buffer[len-1] == '/') {
		buffer[len-1] = '\0';
	}
	if (mkdir(buffer, 0775) == 0)
	{
		free(buffer);
		return 1;
	}

	p = buffer+1;
	while (1)
	{
		char hold;

		while(*p && *p != '\\' && *p != '/')
			p++;
		hold = *p;
		*p = 0;
		if ((mkdir(buffer, 0775) == -1) && (errno == ENOENT))
		{
//			fprintf(stderr,"%s: couldn't create directory %s\n",prog,buffer);
			free(buffer);
			return false;
		}
		if (hold == 0)
		break;
		*p++ = hold;
	}
	free(buffer);
	return true;
}

/**
 * Destroy a directory and all its contents (recusively if needed).
 */
void vtDestroyDir(const char *dirname)
{
	int result;

	vtStringArray con;

	char fullname[1024];
	dir_iter it(dirname);
	for (; it != dir_iter(); ++it)
	{
		std::string name1 = it.filename();
		if (name1 == "." || name1 == "..")
			continue;
		con.push_back(vtString(name1.c_str()));
	}

	for (unsigned int i = 0; i < con.size(); i++)
	{
		vtString item = con[i];

		strcpy(fullname, dirname);
		strcat(fullname, "/");
		strcat(fullname, (const char *) item );

		if (it.is_directory())
		{
			vtDestroyDir(fullname);
		}
		else
		{
			result = unlink(fullname);
			if (result == -1)
			{
				// failed
				if (errno == ENOENT)	// not found
					result = 0;
				if (errno == EACCES)	// found but can't delete
					result = 0;
			}
		}
	}
	rmdir(dirname);
}

/**
 * Delete a file.
 */
void vtDeleteFile(const char *filename)
{
	unlink(filename);
}


/**
 * Given a full path containing a filename, return a pointer to
 * the filename portion of the string.
 */
const char *StartOfFilename(const char *szFullPath)
{
	const char *tmp = szFullPath;
	const char *tmp1 = strrchr(szFullPath, '/');
	if (tmp1)
		tmp = tmp1+1;
	const char *tmp2 = strrchr(szFullPath, '\\');
	if (tmp2 && tmp2 > tmp)
		tmp = tmp2+1;
	const char *tmp3 = strrchr(szFullPath, ':');
	if (tmp3 && tmp3 > tmp)
		tmp = tmp3+1;
	return tmp;
}

/**
 * Return whether a path is absolute or relative.
 */
bool PathIsAbsolute(const char *szPath)
{
	int len = strlen(szPath);
	if (len >= 2 && szPath[1] == ':')
		return true;
	if (len >= 1 && (szPath[0] == '/' || szPath[0] == '\\'))
		return true;
	return false;
}

/**
 * Given a filename (which may include a path), remove any file extension(s)
 * which it may have.
 */
void RemoveFileExtensions(vtString &fname, bool bAll)
{
	for (int i = fname.GetLength()-1; i >= 0; i--)
	{
		char ch = fname[i];

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

/**
 * Get the full file extension(s) from a filename.
 */
vtString GetExtension(const vtString &fname, bool bFull)
{
	int chop = -1;
	for (int i = fname.GetLength()-1; i >= 0; i--)
	{
		char ch = fname[i];

		// If we hit a path divider, stop
		if (ch == ':' || ch == '\\' || ch == '/')
			break;

		// If we hit a period which indicates an extension, note it.
		if (ch == '.')
		{
			chop = i;
			if (!bFull)
				break;
		}
	}
	if (chop == -1)
		return vtString("");
	else
		return fname.Right(fname.GetLength() - chop);
}

#include <fstream>
using namespace std;
//
// helper
//
vtString get_line_from_stream(ifstream &input)
{
	char buf[80];
	input.getline(buf, 80);
	int len = strlen(buf);

	// trim trailing CR and LF characters
	while (len > 0 && (buf[len-1] == '\r' || buf[len-1] == '\n'))
	{
		buf[len-1] = '\0';
		len--;
	}
	return vtString(buf);
}

/* alternate version
vtString get_line_from_stream(ifstream &input)
{
	char buf[80];
	// eat leading LF
	if (input.peek() == '\n') {
		input.ignore();
		buf[0] = '\0';
	} else {
		input >> buf;
	}
	return vtString(buf);
}
*/


//
// Encapsulation for Zlib's gzip output functions.  These wrappers allow
//  you to do stdio file output to a compressed _or_ uncompressed file
//  with one set of functions.
//
GZOutput::GZOutput(bool bCompressed)
{
	fp = NULL;
	gfp = NULL;
	bGZip = bCompressed;
}

bool gfopen(GZOutput &out, const char *fname)
{
	if (out.bGZip)
	{
		out.gfp = gzopen(fname, "wb");
		return (out.gfp != NULL);
	}
	else
	{
		out.fp = fopen(fname, "wb");
		return out.fp != NULL;
	}
}

int gfprintf(GZOutput &out, const char *pFormat, ...)
{
	va_list va;
	va_start(va, pFormat);

	if (out.bGZip)
	{
		// For unknown reasons, gzprintf sometimes fails to write strings,
		//  instead writing a handful of bogus bytes.
//		return gzprintf(out.gfp, pFormat, va);

		// Work around the mysterious failure in gzprintf
		char buf[4096];
		int chars = vsnprintf(buf, 4096, pFormat, va);
		gzwrite(out.gfp, buf, chars);
		return chars;
	}
	else
		return vfprintf(out.fp, pFormat, va);
}

void gfclose(GZOutput &out)
{
	if (out.bGZip)
		gzclose(out.gfp);
	else
		fclose(out.fp);
}

