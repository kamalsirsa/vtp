//
// Name:        canvas.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef CANVASH
#define CANVASH

#if !wxUSE_GLCANVAS
#error Please set wxUSE_GLCANVAS to 1 in setup.h.
#endif
#include "wx/glcanvas.h"

class vtGLCanvas;

//
// A timer used to refresh the status bar at a constant rate,
// asynchronously with framerate.
//
class StatusTimer : public wxTimer
{
public:
	StatusTimer() { m_pFrame = NULL; }
	void SetFrame(wxFrame *pFrame) { m_pFrame = pFrame; }
	void Notify();
	class wxFrame *m_pFrame;
};

//
// A Canvas for the main view area.
//
class vtGLCanvas: public wxGLCanvas
{
public:
	vtGLCanvas(wxWindow *parent, const wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition,
	  const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = "vtGLCanvas",
	  int* gl_attrib = NULL);
	~vtGLCanvas(void);

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void OnChar(wxKeyEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnClose(wxCloseEvent& event);
	void QueueRefresh(bool eraseBackground);

	bool m_bPainting;
	bool m_bRunning;
	bool m_bShowFrameRateChart;

protected:
	StatusTimer	m_StatusTimer;

	DECLARE_EVENT_TABLE()
};

void EnableContinuousRendering(bool bTrue);

#endif
