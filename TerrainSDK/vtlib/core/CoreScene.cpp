//
// CoreScene.cpp
//
// Copyright (c) 2001-2007 Virtual Terrain Project
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



