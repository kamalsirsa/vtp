
#ifndef FILEPATHH
#define FILEPATHH

#include "vtString.h"
#include "Array.h"
#include "Array.inl"

class StringArray : public Array<vtString*>
{
public:
	// assignment
	StringArray &operator=(const class StringArray &v);
};

vtString FindFileOnPaths(const StringArray &paths, const char *filename);

#endif // FILEPATHH