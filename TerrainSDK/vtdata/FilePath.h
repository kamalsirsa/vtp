
#ifndef FILEPATHH
#define FILEPATHH

#include "vtString.h"
#include "Array.h"

vtString FindFileOnPaths(const StringArray &paths, const char *filename);
bool vtCreateDir(const char *dirname);
void vtDestroyDir(const char *dirname);
const char *StartOfFilename(const char *szFullPath);
bool PathIsAbsolute(const char *szPath);

#endif // FILEPATHH

