
#ifndef FILEPATHH
#define FILEPATHH

#include "vtString.h"
#include "Array.h"

class StringArray : public Array<vtString*>
{
public:
	virtual ~StringArray() { Empty(); free(m_Data); m_Data = NULL; m_MaxSize = 0; }
	virtual	void DestructItems(int first, int last)
	{
		for (int i = first; i <= last; ++i)
			delete GetAt(i);
	}

	// assignment
	StringArray &operator=(const class StringArray &v);
};

vtString FindFileOnPaths(const StringArray &paths, const char *filename);
bool vtCreateDir(const char *dirname);
void vtDestroyDir(const char *dirname);
const char *StartOfFilename(const char *szFullPath);
bool PathIsAbsolute(const char *szPath);

#endif // FILEPATHH

