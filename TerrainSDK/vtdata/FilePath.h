
#ifndef FILEPATHH
#define FILEPATHH

#include "vtString.h"
#include "Array.h"

vtString FindFileOnPaths(const vtStringArray &paths, const char *filename);
bool vtCreateDir(const char *dirname);
void vtDestroyDir(const char *dirname);
const char *StartOfFilename(const char *szFullPath);
bool PathIsAbsolute(const char *szPath);
vtString get_line_from_stream(std::ifstream &input);

#endif // FILEPATHH

