//
// Content3d.h
//
// Header for the Content Management classes.
//
// Copyright (c) 2003 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_CONTENT3DH
#define VTLIB_CONTENT3DH

#include "vtdata/Content.h"

/**
 * Subclass of vtItem which is capable of instantiating the corresponding
 * 3D geometry for the item.
 */
class vtItem3d : public vtItem
{
public:
	vtItem3d();
	~vtItem3d();

	bool LoadModels(vtStringArray *pDataPaths);
	void UpdateExtents();

	vtGroup *m_pGroup;
};

/**
 * Subclass of vtContentManager which is capable of instantiating the
 * corresponding 3D geometry for the content items.
 */
class vtContentManager3d : public vtContentManager
{
public:
	vtContentManager3d();

	void SetDataPaths(vtStringArray *pDataPaths) { m_pDataPaths = pDataPaths; }
	vtGroup *CreateGroupFromItemname(const char *itemname);

	// implementation
	virtual vtItem *NewItem() { return new vtItem3d; }

protected:
	vtStringArray *m_pDataPaths;
};

#endif // VTLIB_CONTENT3DH

