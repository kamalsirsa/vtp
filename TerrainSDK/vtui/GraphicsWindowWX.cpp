#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef USE_OSG_VIEWER

#include "wx/glcanvas.h"
#include "GraphicsWindowWX.h"
#include <osgViewer/Viewer>

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

GraphicsWindowWX::GraphicsWindowWX(wxGLCanvas* pCanvas)
{
	m_pCanvas = pCanvas;
	m_pGLContext = NULL;
	m_bValid = true;
	m_bIsRealized = false;
}

bool GraphicsWindowWX::realizeImplementation()
{
	m_pGLContext = new LocalGLContext(m_pCanvas);
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

bool GraphicsWindowWX::makeCurrentImplementation()
{
    m_pCanvas->SetCurrent(*m_pGLContext);
    return true;
}

bool GraphicsWindowWX::releaseContextImplementation()
{
	m_pGLContext->ReleaseContext(*m_pCanvas);
	return true;
}

void GraphicsWindowWX::swapBuffersImplementation()
{
    m_pCanvas->SwapBuffers();
}

bool GraphicsWindowWX::isRealizedImplementation() const  
{
	return m_bIsRealized;
}

void GraphicsWindowWX::closeImplementation()
{
	m_bValid = false;
}

bool GraphicsWindowWX::valid() const
{
	return m_bValid;
}

void GraphicsWindowWX::CloseOsgContext()
{
	getEventQueue()->closeWindow(0);
	// Force handling of event before the idle loop can call frame();
	dynamic_cast<osgViewer::View*>(getCameras().front()->getView())->getViewerBase()->eventTraversal();
}

#endif
