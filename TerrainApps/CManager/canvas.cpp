//
// Name:	 canvas.cpp
// Purpose:	 Implements the canvas class for the wxWindows application.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/NavEngines.h"
#include "vtdata/vtLog.h"
#include "canvas.h"
#include "frame.h"
#include "app.h"

#ifndef WIN32
#define Sleep sleep
#endif

DECLARE_APP(vtApp)

/*
 * vtGLCanvas implementation
 */
BEGIN_EVENT_TABLE(vtGLCanvas, wxGLCanvas)
	EVT_CLOSE(vtGLCanvas::OnClose)
	EVT_SIZE(vtGLCanvas::OnSize)
	EVT_PAINT(vtGLCanvas::OnPaint)
	EVT_CHAR(vtGLCanvas::OnChar)
	EVT_MOUSE_EVENTS(vtGLCanvas::OnMouseEvent)
	EVT_ERASE_BACKGROUND(vtGLCanvas::OnEraseBackground)
	EVT_IDLE(vtGLCanvas::OnIdle)
END_EVENT_TABLE()

vtGLCanvas::vtGLCanvas(wxWindow *parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, long style, const wxString& name, int* gl_attrib) :
  wxGLCanvas(parent, id, pos, size, style, name, gl_attrib)
{
	VTLOG(" constructing Canvas\n");
	parent->Show(TRUE);
	SetCurrent();

	m_bPainting = false;
	m_bRunning = true;
}


vtGLCanvas::~vtGLCanvas(void)
{
	VTLOG(" destructing Canvas\n");
}

void vtGLCanvas::QueueRefresh(bool eraseBackground)
	// A Refresh routine we can call from inside OnPaint.
	//   (queues the events rather than dispatching them immediately).
{
	// With wxGTK, you can't do a Refresh() in OnPaint because it doesn't
	//   queue (post) a Refresh event for later.  Rather it dispatches
	//   (processes) the underlying events immediately via ProcessEvent
	//   (read, recursive call).  See the wxPostEvent docs and Refresh code
	//   for more details.
	if ( eraseBackground ) 
	{
		wxEraseEvent eevent( GetId() );
		eevent.SetEventObject( this );
		wxPostEvent( GetEventHandler(), eevent );
	}

	wxPaintEvent event( GetId() );
	event.SetEventObject( this );
	wxPostEvent( GetEventHandler(), event );
}

void vtGLCanvas::UpdateStatusText()
{
	vtFrame *frame = GetMainFrame();
	if (!frame || !frame->GetStatusBar()) return;

	vtScene *scene = vtGetScene();
	if (!scene) return;

	// get framerate
	float fps = scene->GetFrameRate();

	// get camera distance
	float dist = wxGetApp().m_pTrackball->GetRadius();

	wxString str;
	str.Printf(_T("fps %.3g, camera distance %.2f meters"), fps, dist);

	frame->SetStatusText(str);
}

void vtGLCanvas::OnPaint( wxPaintEvent& event )
{
	static bool bFirst = true;

	if (bFirst)
	{
		VTLOG("first OnPaint\n");
		bFirst = false;
	}
	// place the dc inside a scope, to delete it before the end of function
	if (1)
	{
		// This is a dummy, to avoid an endless succession of paint messages.
		// OnPaint handlers must always create a wxPaintDC.
		wxPaintDC dc(this);
#ifndef __WXMOTIF__
		if (!GetContext()) return;
#endif

		if (m_bPainting) return;

		m_bPainting = true;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render the SSG scene
		vtGetScene()->DoUpdate();

		SwapBuffers();

		// Limit framerate (rough, but sufficient)
		float curr = vtGetFrameTime();
		float max = 1.0f / 60.0f;
		float diff = max - curr;
		if (diff > 0)
			Sleep((int)(diff * 1000));

#ifdef WIN32
		// We use refresh-on-idle on WIN32
#else
		// Queue another refresh for continuous rendering.
		//   (Yield first so we don't starve out keyboard & mouse events.)
		//
		// FIXME: We may want to use a frame timer instead of immediate-
		//   redraw so we don't eat so much CPU on machines that can
		//   easily handle the frame rate.
		wxYield();
		QueueRefresh(FALSE);
#endif

		// update the status bar every 1/10 of a second
		static float last_stat = 0.0f;
		float cur = vtGetTime();
		if (cur - last_stat > 0.1f)
		{
			last_stat = cur;
			UpdateStatusText();
		}

		m_bPainting = false;
	}
}

static void Reshape(int width, int height)
{
	glViewport(0, 0, (GLint)width, (GLint)height);
}


void vtGLCanvas::OnClose(wxCloseEvent& event)
{
	VTLOG("Canvas OnClose\n");
	m_bRunning = false;
}

void vtGLCanvas::OnSize(wxSizeEvent& event)
{
#ifndef __WXMOTIF__
	if (!GetContext()) return;
#endif

	SetCurrent();
	int width, height;
	GetClientSize(& width, & height);
	Reshape(width, height);

	vtGetScene()->SetWindowSize(width, height);
}

void vtGLCanvas::OnChar(wxKeyEvent& event)
{
	long key = event.KeyCode();

	// pass the char to the frame for it to do "accelerator" shortcuts
	vtFrame *frame = GetMainFrame();
	frame->OnChar(event);

	int flags = 0;

	if (event.ControlDown())
		flags |= VT_CONTROL;

	if (event.ShiftDown())
		flags |= VT_SHIFT;

	if (event.AltDown())
		flags |= VT_ALT;

	// pass the char to the vtlib Scene
	vtGetScene()->OnKey(key, flags);

	// Allow wxWindows to pass the event along to other code
	event.Skip();
}

void vtGLCanvas::OnMouseEvent(wxMouseEvent& event1)
{
	static bool bCapture = false;

	// turn WX mouse event into a VT mouse event
	vtMouseEvent event;
	wxEventType  ev = event1.GetEventType();
	if (ev == wxEVT_LEFT_DOWN) {
		event.type = VT_DOWN;
		event.button = VT_LEFT;
	} else if (ev == wxEVT_LEFT_UP) {
		event.type = VT_UP;
		event.button = VT_LEFT;
	} else if (ev == wxEVT_MIDDLE_DOWN) {
		event.type = VT_DOWN;
		event.button = VT_MIDDLE;
	} else if (ev == wxEVT_MIDDLE_UP) {
		event.type = VT_UP;
		event.button = VT_MIDDLE;
	} else if (ev == wxEVT_RIGHT_DOWN) {
		event.type = VT_DOWN;
		event.button = VT_RIGHT;
	} else if (ev == wxEVT_RIGHT_UP) {
		event.type = VT_UP;
		event.button = VT_RIGHT;
	} else if (ev == wxEVT_MOTION) {
		event.type = VT_MOVE;
		event.button = VT_NONE;
	} else {
		// ignored mouse events, such as wxEVT_LEAVE_WINDOW
		return;
	}

	if (ev == wxEVT_LEFT_DOWN ||
		ev == wxEVT_MIDDLE_DOWN ||
		ev == wxEVT_RIGHT_DOWN)
	{
		if (!bCapture)
		{
			CaptureMouse();
			bCapture = true;
		}
	}
	if (ev == wxEVT_LEFT_UP ||
		ev == wxEVT_MIDDLE_UP ||
		ev == wxEVT_RIGHT_UP)
	{
		if (bCapture)
		{
			ReleaseMouse();
			bCapture = false;
		}
	}

	event.flags = 0;
	wxCoord xpos, ypos;
    event1.GetPosition(&xpos, &ypos);
	event.pos.Set(xpos, ypos);

	if (event1.ControlDown())
		event.flags |= VT_CONTROL;

	if (event1.ShiftDown())
		event.flags |= VT_SHIFT;

	vtGetScene()->OnMouse(event);
}

void vtGLCanvas::OnEraseBackground(wxEraseEvent& event)
{
	// Do nothing, to avoid flashing.
}

void vtGLCanvas::OnIdle(wxIdleEvent &event)
{
	// We use the "Refresh on Idle" approach to continuous rendering.
	if (m_bRunning)
		Refresh(FALSE);
}

