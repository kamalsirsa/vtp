//
// FilePath.cpp
//
// Functions for helping with management of directories and file paths
//
// Copyright (c) 2002-2003 Virtual Terrain Project
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
#  else
#	define mkdir(dirname,mode) _mkdir(dirname)
#  endif
#else
#  include <utime.h>
#endif

dir_iter::dir_iter()
{
#if WIN32
	m_handle = -1;
#else
	m_handle = 0;
	m_stat_p = false;
#endif
	m_refcount = 1;
}

dir_iter::dir_iter(std::string const &dirname)
{
#if WIN32
	m_handle = _findfirst((dirname + "\\*").c_str(), &m_data);
#else
	m_handle = opendir(dirname.c_str());
	m_directory = dirname;
	m_stat_p = false;

	if (m_directory[m_directory.size() - 1] != '/')
		m_directory += '/';
	operator++ ();
#endif
	m_refcount = 1;
}

dir_iter::~dir_iter()
{
#if WIN32
	if (m_handle != -1)
		_findclose(m_handle);
#else
	if (m_handle)
		closedir(m_handle);
#endif
}

bool dir_iter::is_directory()
{
#if WIN32
	return (m_data.attrib & _A_SUBDIR) != 0;
#else
	return S_ISDIR(get_stat().st_mode);
#endif
}

bool dir_iter::is_hidden()
{
#if WIN32
	return (m_data.attrib & _A_SUBDIR) != 0;
#else
	return S_ISDIR(get_stat().st_mode);
#endif
}

std::string dir_iter::filename()
{
#if WIN32
	return m_data.name;
#else
	return m_current;
#endif
}

void dir_iter::operator++()
{
#if WIN32
	if (m_handle != -1)
	{
		if (_findnext(m_handle, &m_data) == -1)
		{
			_findclose(m_handle);
			m_handle = -1;
		}
	}
#else
	if (m_handle)
	{
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
#endif
}

bool dir_iter::operator!=(const dir_iter &it)
{
#if WIN32
	return (m_handle == -1) != (it.m_handle == -1);
#else
	return (m_handle == 0) != (it.m_handle == 0);
#endif
}


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
