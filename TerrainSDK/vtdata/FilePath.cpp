//
// FilePath.cpp
//
// Functions for helping with management of file paths
//

#include <stdio.h>
#include "FilePath.h"

//
// assignment operator
//
StringArray &StringArray::operator=(const class StringArray &v)
{
	int size = v.GetSize();
	SetSize(size);
	for (int i = 0; i < size; i++)
		SetAt(i, v.GetAt(i));

	return *this;
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
vtString FindFileOnPaths(const StringArray &paths, const char *filename)
{
	FILE *fp;
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


