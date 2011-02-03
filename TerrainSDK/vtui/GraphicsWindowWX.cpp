#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "wx/glcanvas.h"
#include "GraphicsWindowWX.h"
#include <osgViewer/Viewer>


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

GraphicsWindowWX::GraphicsWindowWX(wxGLCanvas* pCanvas)
{
	m_pCanvas = pCanvas;
#ifndef __WXMAC__
	m_pGLContext = NULL;
#endif
	m_bValid = true;
	m_bIsRealized = false;
}

bool GraphicsWindowWX::realizeImplementation()
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

bool GraphicsWindowWX::makeCurrentImplementation()
{
#ifdef __WXMAC__
    m_pCanvas->SetCurrent();
#else
    m_pCanvas->SetCurrent(*m_pGLContext);
#endif
    return true;
}

bool GraphicsWindowWX::releaseContextImplementation()
{
#ifndef __WXMAC__
	m_pGLContext->ReleaseContext(*m_pCanvas);
#endif
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
