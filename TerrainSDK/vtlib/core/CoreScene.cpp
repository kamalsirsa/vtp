//
// CoreScene.cpp
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

/**
\mainpage vtlib library documentation

\section Overview Overview
The <b>vtlib</b> library is built on top of the <a href="../vtdata.html">vtdata</a> library which
handles many kinds of geospatial data.  It extends <b>vtdata</b> with the
ability to create 3D geometry of the data for interactive visualization.

It is part of the <a href="http://vterrain.org/">Virtual Terrain Project</a>
and distributed under a completely free <a href="../license.txt">open
source license</a>.

<b>vtlib</b> contains an abstraction of a Scene Graph API, built on top of
OpenGL and other lower-level APIs such as OSG and SGL.
Because it is higher-level, <b>vtlib</b> lets you construct 3D geometry
and simulations much faster and easier than using the lower-level libraries
directly.

In addition to these automatically generated pages, programmer documentation
is also <a href="../vtlib.html">available online</a>.
*/

#include "vtlib/vtlib.h"
#include "Engine.h"

vtWindow::vtWindow()
{
	m_BgColor.Set(0.2f, 0.2f, 0.4f);
	m_Size.Set(0, 0);
}

void vtWindow::SetSize(int w, int h)
{
	m_Size.Set(w,h);
}


///////////////////////////////////////////////////////////////////////

vtSceneBase::vtSceneBase()
{
	m_pCamera = NULL;
	m_pRoot = NULL;
	m_pRootEngine = NULL;
	m_piKeyState = NULL;
	m_pDefaultCamera = NULL;
	m_pDefaultWindow = NULL;
}

vtSceneBase::~vtSceneBase()
{
	// cleanup engines
	vtEngineArray list(m_pRootEngine, false);	// ALL engines
	for (unsigned int i = 0; i < list.GetSize(); i++)
		delete list[i];
	m_pRootEngine = NULL;
}

void vtSceneBase::OnMouse(vtMouseEvent &event, vtWindow *pWindow)
{
	// Pass event to Engines
	vtEngineArray list(m_pRootEngine);
	for (unsigned int i = 0; i < list.GetSize(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled() &&
			(pEng->GetWindow() == NULL || pEng->GetWindow() == pWindow))
			pEng->OnMouse(event);
	}
}

void vtSceneBase::OnKey(int key, int flags, vtWindow *pWindow)
{
	// Pass event to Engines
	vtEngineArray list(m_pRootEngine);
	for (unsigned int i = 0; i < list.GetSize(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled() &&
			(pEng->GetWindow() == NULL || pEng->GetWindow() == pWindow))
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

void vtSceneBase::SetWindowSize(int w, int h, vtWindow *pWindow)
{
	if (!pWindow)
		pWindow = GetWindow(0);
	pWindow->SetSize(w, h);

	// Pass event to Engines
	vtEngineArray list(m_pRootEngine);
	for (unsigned int i = 0; i < list.GetSize(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled() &&
			(pEng->GetWindow() == NULL || pEng->GetWindow() == pWindow))
			pEng->OnWindowSize(w, h);
	}
}

IPoint2 vtSceneBase::GetWindowSize(vtWindow *pWindow)
{
	if (!pWindow)
		pWindow = GetWindow(0);
	return pWindow->GetSize();
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

