#pragma once

#ifdef USE_OSG_VIEWER

#include "wx/glcanvas.h"
#include <osgViewer/GraphicsWindow>


class LocalGLContext : public wxGLContext
{
public:
    LocalGLContext(wxGLCanvas *win, const wxGLContext* other=NULL /* for sharing display lists */ );
	void ReleaseContext(const wxGLCanvas& win);
};

class wxGLCanvas;
class GraphicsWindowWX : public osgViewer::GraphicsWindow
{
public:
	GraphicsWindowWX(wxGLCanvas* pCanvas, LocalGLContext *pContext);


	void InvalidateCanvas() { m_bCanvasValid = false; }

    bool makeCurrentImplementation();
    void swapBuffersImplementation();

    virtual bool releaseContextImplementation();
	
    // Cannot think of anything to do here at the moment
    virtual void closeImplementation() {}

    virtual bool realizeImplementation();

	virtual bool valid() const { return true; }

    virtual bool isRealizedImplementation() const  { return m_bCanvasValid && m_bIsRealized; }
private:
	wxGLCanvas* m_pCanvas;
	LocalGLContext *m_pGLContext;
	bool m_bCanvasValid;
	bool m_bIsRealized;
};
#endif
