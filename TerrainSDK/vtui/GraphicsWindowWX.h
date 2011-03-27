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
	GraphicsWindowWX(wxGLCanvas* pCanvas)
	{
		m_pCanvas = pCanvas;
#ifndef __WXMAC__
		m_pGLContext = NULL;
#endif
		m_bValid = true;
		m_bIsRealized = false;
	}

	~GraphicsWindowWX()
	{
#ifndef __WXMAC__
		delete m_pGLContext;
#endif
	}

	void CloseOsgContext()
	{
		getEventQueue()->closeWindow(0);
		// Force handling of event before the idle loop can call frame();
		dynamic_cast<osgViewer::View*>(getCameras().front()->getView())->getViewerBase()->eventTraversal();
	}

    bool makeCurrentImplementation()
	{
#ifdef __WXMAC__
		m_pCanvas->SetCurrent();
#else
		m_pCanvas->SetCurrent(*m_pGLContext);
#endif
		return true;
	}

    void swapBuffersImplementation()
	{
		m_pCanvas->SwapBuffers();
	}

    virtual bool releaseContextImplementation()
	{
#ifndef __WXMAC__
		m_pGLContext->ReleaseContext(*m_pCanvas);
#endif
		return true;
	}
	
    virtual void closeImplementation()
	{
		m_bValid = false;
	}


    virtual bool realizeImplementation()
	{
#ifndef __WXMAC__
		m_pGLContext = new LocalGLContext(m_pCanvas);
#endif
		wxPoint pos = m_pCanvas->GetPosition();
		wxSize  size = m_pCanvas->GetSize();

		// Set up traits to match the canvas
		_traits = new GraphicsContext::Traits;
		_traits->x = pos.x;
		_traits->y = pos.y;
		_traits->width = size.x;
		_traits->height = size.y;

		// Set up a new context ID - I don't think we make use of this at the moment
		setState( new osg::State );
		getState()->setGraphicsContext(this);
		getState()->setContextID( osg::GraphicsContext::createNewContextID() );
		m_bIsRealized = true;
		return true;
	}


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
