//
// CoreScene.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

/**
\mainpage vtlib library documentation

\section overview Overview
The <b>vtlib</b> library is built on top of the <b>vtdata</b> library which
handles many kinds of geospatial data.  It extends <b>vtdata</b> with the
ability to create 3D geometry of the data for interactive visualization.

It is part of the <a href="http://vterrain.org/">Virtual Terrain Project</a>
and distributed under a completely free <a href="../../license.txt">open
source license</a>.

<b>vtlib</b> contains an abstraction of a Scene Graph API, built on top of
OpenGL and other lower-level APIs such as OSG and SGL.
Because it is higher-level, <b>vtlib</b> lets you construct 3D geometry
and simulations much faster and easier than using the lower-level libraries
directly.

Some documentation for vtlib, and important discussion of how to use the
library, is <a href="http://vterrain.org/Implementation/Libs/vtlib.html">
online</a>.

This folder contains documentation of the classes and methods - see the
<a href="hierarchy.html">Class Hierarchy</a> or <a href="inherits.html">
graphical class hierarchy</a>.

*/

#include "vtlib/vtlib.h"
#include "vtlib/core/Engine.h"


vtSceneBase::vtSceneBase()
{
	m_pCamera = NULL;
	m_WindowSize.Set(0, 0);
	m_pRoot = NULL;
	m_pRootEngine = NULL;
	m_piKeyState = NULL;
}

vtSceneBase::~vtSceneBase()
{
	// cleanup engines
	vtEngineArray list(m_pRootEngine);
	for (unsigned int i = 0; i < list.GetSize(); i++)
		delete list[i];
	m_pRootEngine = NULL;
}

void vtSceneBase::OnMouse(vtMouseEvent &event)
{
	// Pass event to Engines
	vtEngineArray list(m_pRootEngine);
	for (unsigned int i = 0; i < list.GetSize(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled())
			pEng->OnMouse(event);
	}
}

void vtSceneBase::OnKey(int key, int flags)
{
	// Pass event to Engines
	vtEngineArray list(m_pRootEngine);
	for (unsigned int i = 0; i < list.GetSize(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled())
			pEng->OnKey(key, flags);
	}
}

bool vtSceneBase::GetKeyState(int key)
{
	if (m_piKeyState)
		return m_piKeyState[key];
	else
		return false;
}

void vtSceneBase::SetWindowSize(int w, int h)
{
	m_WindowSize.Set(w, h);

	// Pass event to Engines
	vtEngineArray list(m_pRootEngine);
	for (unsigned int i = 0; i < list.GetSize(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled())
			pEng->OnWindowSize(w, h);
	}
}

void vtSceneBase::DoEngines()
{
	// Evaluate Engines
	vtEngineArray list(m_pRootEngine);
	for (unsigned int i = 0; i < list.GetSize(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled())
			pEng->Eval();
	}
}

// (for backward compatibility only)
void vtSceneBase::AddEngine(vtEngine *ptr)
{
	if (m_pRootEngine)
		m_pRootEngine->AddChild(ptr);
	else
		m_pRootEngine = ptr;
}

