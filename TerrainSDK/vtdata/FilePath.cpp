//
// FilePath.cpp
//
// Functions for helping with management of file paths
//

#include <stdio.h>
#include <errno.h>

#include "FilePath.h"
#include "boost/directory.h"

#ifdef unix
# include <unistd.h>
# include <sys/stat.h>
#else
# include <direct.h>
# include <io.h>
#endif

#ifdef WIN32
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
vtString FindFileOnPaths(const StringArray &paths, const char *filename)
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

	for (int i = 0; i < paths.GetSize(); i++)
	{
		vtString fname = *(paths[i]);
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
	using namespace boost::filesystem;

	StringArray con;

	char fullname[1024];
	dir_it it(dirname);
	for (; it != dir_it(); ++it)
	{
		std::string name1 = *it;
		if (name1 == "." || name1 == "..")
			continue;
		con.Append(new vtString(name1.c_str()));
	}

	for (int i = 0; i < con.GetSize(); i++)
	{
		vtString *item = con.GetAt(i);

		strcpy(fullname, dirname);
		strcat(fullname, "/");
		strcat(fullname, (const char *) *item );

		if (get<is_directory>(it))
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

		delete item;
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

