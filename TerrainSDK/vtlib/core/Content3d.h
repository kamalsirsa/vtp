//
// Content3d.h
//
// Header for the Content Management classes.
//
// Copyright (c) 2003-2004 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_CONTENT3DH
#define VTLIB_CONTENT3DH

/** \defgroup content Content Management
 * These classes are used for managing 3D model content - generally,
 * collections of 3D models with metadata.
 */
/*@{*/

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

	bool LoadModels();
	void UpdateExtents();

	osg::Node *m_pNode;
};

/**
 * Subclass of vtContentManager which is capable of instantiating the
 * corresponding 3D geometry for the content items.
 */
class vtContentManager3d : public vtContentManager
{
public:
	vtContentManager3d();
	~vtContentManager3d();

	// implementation
	osg::Node *CreateNodeFromItemname(const char *itemname);
	virtual vtItem *NewItem() { return new vtItem3d; }
	void ReleaseContents();

	vtGroupPtr m_pGroup;
};

/*@}*/	// group content

#endif // VTLIB_CONTENT3DH

