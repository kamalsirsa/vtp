//
// Name: canvas.h
//
// Copyright (c) 2001-2008 Virtual Terrain Project
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
// A Canvas for the main view area.
//
class vtGLCanvas: public wxGLCanvas
{
public:
	vtGLCanvas(wxWindow *parent, const wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition,
	  const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = _T("vtGLCanvas"),
	  int* gl_attrib = NULL);
	~vtGLCanvas(void);

#if WIN32
    // Hook into the default window procedure
    virtual WXLRESULT MSWDefWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif

#ifndef __WXMAC__
	void OnPaint(wxPaintEvent& event);
#endif
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void OnChar(wxKeyEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnIdle(wxIdleEvent &event);

#ifndef __WXMAC__
	bool m_bPainting;
#endif
	bool m_bRunning;

protected:
	DECLARE_EVENT_TABLE()
};

#endif	// CANVASH

