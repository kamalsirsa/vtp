//
// Engine.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTENGINEH
#define VTENGINEH

#include "vtdata/vtString.h"

class vtEngine : public vtEnabledBase
{
public:
	vtEngine();

	void *GetTarget() { return m_pTarget; }
	void SetTarget(void *ptr) { m_pTarget = ptr; }

	void SetName2(const char *str) { m_strName = str; }
	const char *GetName2() { return m_strName; }

	virtual void OnMouse(vtMouseEvent &event);
	virtual void OnKey(int key, int flags);

	virtual void Eval();

protected:
	void	 *m_pTarget;
	vtString m_strName;
};

class vtLastMouse : public vtEngine
{
public:
	vtLastMouse();

	void OnMouse(vtMouseEvent &event);

	int m_buttons;
	IPoint2 m_pos;
};

#endif
