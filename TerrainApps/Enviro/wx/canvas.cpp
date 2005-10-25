//
// Name:	 canvas.cpp
// Purpose: Implements the canvas class for the Enviro wxWidgets application.
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/Event.h"
#include "vtui/wxString2.h"
#include "vtdata/vtLog.h"
#include "EnviroGUI.h"			// for g_App, GetTerrainScene
#include "canvas.h"
#include "EnviroFrame.h"		// for UpdateStatus and OnChar
#include "EnviroApp.h"

DECLARE_APP(EnviroApp)

/*
 * vtGLCanvas implementation
 */
BEGIN_EVENT_TABLE(vtGLCanvas, wxGLCanvas)
EVT_CLOSE(vtGLCanvas::OnClose)
EVT_SIZE(vtGLCanvas::OnSize)
EVT_PAINT(vtGLCanvas::OnPaint)
EVT_CHAR(vtGLCanvas::OnChar)
EVT_KEY_DOWN(vtGLCanvas::OnKeyDown)
EVT_KEY_UP(vtGLCanvas::OnKeyUp)
EVT_MOUSE_EVENTS(vtGLCanvas::OnMouseEvent)
EVT_ERASE_BACKGROUND(vtGLCanvas::OnEraseBackground)
EVT_IDLE(vtGLCanvas::OnIdle)
END_EVENT_TABLE()

static vtGLCanvas *s_canvas = NULL;

vtGLCanvas::vtGLCanvas(wxWindow *parent, wxWindowID id,
	const wxPoint& pos, const wxSize& size, long style, const wxString& name, int* gl_attrib):
		wxGLCanvas(parent, id, pos, size, style, name, gl_attrib)
{
	VTLOG("vtGLCanvas constructor\n");
	VTLOG("  parent %lx, id %d, pos %d %d, size %d %d, style %0x\n",
		parent, id, pos.x, pos.y, size.x, size.y, style);

	VTLOG("vtGLCanvas: calling Show on parent\n");
	parent->Show();

	// Documentation says about SetCurrent:
	// "Note that this function may only be called after the window has been shown."
	VTLOG("vtGLCanvas: calling SetCurrent\n");
	SetCurrent();

        wxGLContext *context = GetContext();
        if (context)
		VTLOG("OpenGL context: %lx\n", context);
	else
        {
                VTLOG("No OpenGL context, quitting app.\n");
                exit(0);
        }

        VTLOG("OpenGL version: ");
        VTLOG1((const char *) glGetString(GL_VERSION));

	m_bPainting = false;
	m_bRunning = true;
	m_bShowFrameRateChart = false;

	for (int i = 0; i < 512; i++)
		m_pbKeyState[i] = false;
	vtGetScene()->SetKeyStates(m_pbKeyState);
	m_iConsecutiveMousemoves = 0;

	s_canvas = this;
	VTLOG("vtGLCanvas, leaving constructor\n");
}

vtGLCanvas::~vtGLCanvas(void)
{
	VTLOG("Deleting Canvas\n");
}


void EnableContinuousRendering(bool bTrue)
{
	VTLOG("EnableContinuousRendering %d\n", bTrue);
	if (!s_canvas)
		return;

	bool bNeedRefresh = (s_canvas->m_bRunning == false && bTrue == true);
	s_canvas->m_bRunning = bTrue;
	if (bNeedRefresh)
		s_canvas->Refresh(FALSE);
}

void vtGLCanvas::OnPaint( wxPaintEvent& event )
{
	static bool bFirstPaint = true;
	if (bFirstPaint) VTLOG("vtGLCanvas: first OnPaint\n");

	// Safety check
	if (!s_canvas)
	{
		VTLOG("OnPaint: Canvas not yet constructed, returning\n");
		return;
	}

	// place the dc inside a scope, to delete it before the end of function
	if (1)
	{
		// This is a dummy, to avoid an endless succession of paint messages.
		// OnPaint handlers must always create a wxPaintDC.
		if (bFirstPaint) VTLOG("vtGLCanvas: creating a wxPaintDC on the stack\n");
		wxPaintDC dc(this);

		// Safety check
		if (!GetContext())
		{
			VTLOG("OnPaint: No context yet, exiting OnPaint\n");
			return;
		}

		// Avoid reentrance
		if (m_bPainting) return;

		m_bPainting = true;

		// Render the Scene Graph
		if (bFirstPaint) VTLOG("vtGLCanvas: DoUpdate\n");
		vtGetScene()->DoUpdate();

		if (m_bShowFrameRateChart)
			vtGetScene()->DrawFrameRateChart();

		if (bFirstPaint) VTLOG("vtGLCanvas: SwapBuffers\n");
		SwapBuffers();

		if (bFirstPaint) VTLOG("vtGLCanvas: update status bar\n");
		EnviroFrame *frame = (EnviroFrame*) GetParent();

		// update the status bar every 1/10 of a second
		static float last_stat = 0.0f;
		static vtString last_msg;
		float cur = vtGetTime();
		if (cur - last_stat > 0.1f || g_App.GetMessage() != last_msg)
		{
			last_msg = g_App.GetMessage();
			last_stat = cur;
			frame->UpdateStatus();
		}

		frame->UpdateLODInfo();

		m_bPainting = false;
	}

	// Reset the number of mousemoves we've gotten since last redraw
	m_iConsecutiveMousemoves = 0;

	if (bFirstPaint)
		bFirstPaint = false;
}

void vtGLCanvas::OnClose(wxCloseEvent& event)
{
	m_bRunning = false;
}

void vtGLCanvas::OnSize(wxSizeEvent& event)
{
  // Presumably this is a wxMSWism.
  // For wxGTK & wxMotif, all canvas resize events occur before the context
  //   is set.  So ignore this context check and grab the window width/height
  //   when we get it so it (and derived values such as aspect ratio and
  //   viewport parms) are computed correctly.
#ifdef __WXMSW__
	if (!GetContext()) return;
#endif

	SetCurrent();
	int width, height;
	GetClientSize(& width, & height);

	vtGetScene()->SetWindowSize(width, height);

	wxGLCanvas::OnSize(event);
}

void vtGLCanvas::OnChar(wxKeyEvent& event)
{
	long key = event.GetKeyCode();

	// pass the char to the frame for it to do "accelerator" shortcuts
	EnviroFrame *frame = (EnviroFrame*) GetParent();
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

void vtGLCanvas::OnKeyDown(wxKeyEvent& event)
{
	m_pbKeyState[event.m_keyCode] = true;
	event.Skip();
}

void vtGLCanvas::OnKeyUp(wxKeyEvent& event)
{
	m_pbKeyState[event.m_keyCode] = false;
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
		m_iConsecutiveMousemoves++;		// Increment
	} else {
		// ignored mouse events, such as wxEVT_LEAVE_WINDOW
		return;
	}

	if (ev == wxEVT_LEFT_DOWN || ev == wxEVT_MIDDLE_DOWN || ev == wxEVT_RIGHT_DOWN)
	{
//		VTLOG("DOWN: capture %d", bCapture);
		if (!bCapture)
		{
			CaptureMouse();
			bCapture = true;
//			VTLOG(" -> true");
		}
//		VTLOG("\n");
	}
	if (ev == wxEVT_LEFT_UP || ev == wxEVT_MIDDLE_UP || ev == wxEVT_RIGHT_UP)
	{
//		VTLOG("  UP: capture %d", bCapture);
		if (bCapture)
		{
			ReleaseMouse();
			bCapture = false;
//			VTLOG(" -> false");
		}
//		VTLOG("\n");
	}

	// Because of the way the event pump works, if it takes too long to
	//  handle a MouseMove event, then we might get the next MouseMove
	//  event without ever seeing a Redraw or Idle.  That's because the
	//  MouseMove events are considered higher priority in the queue.
	// So, to keep Enviro response smooth, we effectively ignore all but
	//  one MouseMove event per Draw event.
	if (ev == wxEVT_MOTION && m_iConsecutiveMousemoves > 1)
		return;

	event.flags = 0;
	wxCoord xpos, ypos;
	event1.GetPosition(&xpos, &ypos);
	event.pos.Set(xpos, ypos);

	if (event1.ControlDown())
		event.flags |= VT_CONTROL;

	if (event1.ShiftDown())
		event.flags |= VT_SHIFT;

	if (event1.AltDown())
		event.flags |= VT_ALT;

	// inform vtlib scene, which informs the engines
	vtGetScene()->OnMouse(event);

	// inform Enviro app
	g_App.OnMouse(event);
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

