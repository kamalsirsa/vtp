
#ifndef FILEPATHH
#define FILEPATHH

#include "vtString.h"
#include "Array.h"

class StringArray : public Array<vtString*>
{
public:
	// assignment
	StringArray &operator=(const class StringArray &v);
};

vtString FindFileOnPaths(const StringArray &paths, const char *filename);
bool vtCreateDir(const char *dirname);
void vtDestroyDir(const char *dirname);

#endif // FILEPATHH