//
// Name:	 frame.cpp
// Purpose:  The frame class for a wxWindows application.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Header for the vtlib library
#include "vtlib/vtlib.h"

#include "app.h"
#include "frame.h"
#include "canvas.h"

DECLARE_APP(vtApp)

BEGIN_EVENT_TABLE(vtFrame, wxFrame)
	EVT_CLOSE(vtFrame::OnClose)
END_EVENT_TABLE()

// My frame constructor
vtFrame::vtFrame(wxFrame *parent, const wxString& title, const wxPoint& pos,
	const wxSize& size, long style):
	wxFrame(parent, -1, title, pos, size, style)
{
	// Make a vtGLCanvas
  //   FIXME:  Can remove this special case once wxMotif 2.3 is released
#ifdef __WXMOTIF__
	int gl_attrib[20] = { GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
			GLX_BLUE_SIZE, 1, GLX_DEPTH_SIZE, 1,
			GLX_DOUBLEBUFFER, None };
#else
	int *gl_attrib = NULL;
#endif

	m_canvas = new vtGLCanvas(this, -1, wxPoint(0, 0), wxSize(-1, -1), 0,
		"vtGLCanvas", gl_attrib);

	// Show the frame
	Show(TRUE);

	m_canvas->SetCurrent();
}

vtFrame::~vtFrame()
{
	delete m_canvas;
}

//
// Intercept commands
//

void vtFrame::OnClose(wxCloseEvent &event)
{
	m_canvas->m_bRunning = false;
	delete m_canvas;
	m_canvas = NULL;
	event.Skip();
}

