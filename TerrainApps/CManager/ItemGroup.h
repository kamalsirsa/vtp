//
// Name:		ItemGroup.h
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ITEMGROUPH
#define ITEMGROUPH

///////////////////////////////////////////////////////////////////////
// ItemGroup
//
class ItemGroup
{
public:
	ItemGroup(vtItem *pItem);
	void CreateNodes();
	void AttemptToLoadModels();
	void AttachModels();
	void ShowLOD(bool bTrue);
	void SetRanges();
	vtGroup	*GetTop() { return m_pTop; }
	void ShowOrigin(bool bShow);
	void ShowRulers(bool bShow);

protected:
	vtItem	*m_pItem;
	vtGroup	*m_pTop;
	vtGroup *m_pGroup;
	vtLOD	*m_pLOD;
	vtGeom	*m_pAxes;
	vtGeom	*m_pRulers;
	float m_ranges[20];		// One LOD distance for each child
};

// helper
vtGeom *CreateRulers(float fSize);

#endif // ITEMGROUPH

