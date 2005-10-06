//
// FilePath.h
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
/** \file FilePath.h */

#ifndef FILEPATHH
#define FILEPATHH

#include "vtString.h"
#include "Array.h"

#include "zlib.h"
#include <fstream>

#if WIN32
  #include <io.h>
#else
  #include <sys/stat.h>
  #include <dirent.h>
  #include <unistd.h>
  #include <pwd.h>
  #include <grp.h>
#endif

/**
 * A portable class for reading directory contents.
 */
class dir_iter
{
public:
	dir_iter();
	dir_iter(std::string const &dirname);
	~dir_iter();

	/// Returns true if the current object is a directory.
	bool is_directory();

	/// Returns true if the current object is hidden.
	bool is_hidden();

	/// Get the filename fo the current object.
	std::string filename();

	// Iterate the object to the next file/directory.
	void operator++();

	// Test for inequality useful to test when iteration is finished.
	bool operator!=(const dir_iter &it);

private:
#ifdef WIN32
	struct _finddata_t m_data;
	long               m_handle;
#else
	DIR         *m_handle;
	std::string m_current;
	struct stat m_stat;
	bool        m_stat_p;
	struct stat &get_stat()
	{
		if (!m_stat_p)
		{
			stat(m_current.c_str(), &m_stat);
			m_stat_p = true;
		}
		return m_stat;
	}
#endif
};

vtString FindFileOnPaths(const vtStringArray &paths, const char *filename);
bool vtCreateDir(const char *dirname);
void vtDestroyDir(const char *dirname);
void vtDeleteFile(const char *filename);
const char *StartOfFilename(const char *szFullPath);
bool PathIsAbsolute(const char *szPath);
vtString get_line_from_stream(std::ifstream &input);
void RemoveFileExtensions(vtString &fname, bool bAll = true);
vtString GetExtension(const vtString &fname, bool bFull = true);
vtString ChangeFileExtension(const char *input, const char *extension);
bool FileExists(const char *fname);

// Encapsulation for Zlib's gzip output functions.
class GZOutput
{
public:
	GZOutput(bool bCompressed);
	bool bGZip;
	FILE *fp;
	gzFile gfp;
};
bool gfopen(GZOutput &out, const char *fname);
int gfprintf(GZOutput &out, const char *pFormat, ...);
void gfclose(GZOutput &out);

#endif // FILEPATHH

