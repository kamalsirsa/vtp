#pragma once

#include <osgViewer/Viewer>
#include <osgViewer/GraphicsWindow>
#include "wx/glcanvas.h"

class LocalGLContext;
class wxGLCanvas;

#ifndef __WXMAC__
class LocalGLContext : public wxGLContext
{
public:
    LocalGLContext(wxGLCanvas *win, const wxGLContext* other=NULL /* for sharing display lists */ )
	: wxGLContext(win, other)
	{
	}

	void ReleaseContext(const wxGLCanvas& win)
	{
#if defined(__WXMSW__)
		wglMakeCurrent((HDC) win.GetHDC(), NULL);
#endif
	}
};
#endif

class GraphicsWindowWX : public osgViewer::GraphicsWindow
{
public:
	GraphicsWindowWX(wxGLCanvas *pCanvas, osg::DisplaySettings *pSettings);
	~GraphicsWindowWX();

	void SetCanvas(wxGLCanvas *pCanvas)
	{
		m_pCanvas = pCanvas;
	}
	void CloseOsgContext();
    bool makeCurrentImplementation();
    void swapBuffersImplementation();

	virtual bool releaseContextImplementation();
    virtual void closeImplementation();
    virtual bool realizeImplementation();

	virtual bool valid() const
	{
		return m_bValid;
	}
    virtual bool isRealizedImplementation() const
	{
		return m_bIsRealized;
	}

private:
	wxGLCanvas* m_pCanvas;
#ifndef __WXMAC__
	LocalGLContext *m_pGLContext;
#endif
	bool m_bIsRealized;
	bool m_bValid;
};
