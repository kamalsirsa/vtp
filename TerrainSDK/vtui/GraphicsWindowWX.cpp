#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef USE_OSG_VIEWER

#include "wx/glcanvas.h"
#include "GraphicsWindowWX.h"

GraphicsWindowWX::GraphicsWindowWX(wxGLCanvas* pCanvas, LocalGLContext *pContext)
{
	m_pCanvas = pCanvas;
	m_pGLContext = pContext;
	wxPoint pos = m_pCanvas->GetPosition();
	wxSize  size = m_pCanvas->GetSize();
	m_bCanvasValid = true;
	m_bIsRealized = false;

	// Set up traits to match the passed in window
	_traits = new GraphicsContext::Traits;
	_traits->x = pos.x;
	_traits->y = pos.y;
	_traits->width = size.x;
	_traits->height = size.y;

	// Set up a new context ID - I don't think we make use of this at the moment
	setState( new osg::State );
	getState()->setGraphicsContext(this);
	getState()->setContextID( osg::GraphicsContext::createNewContextID() );
}

bool GraphicsWindowWX::realizeImplementation()
{
	m_bIsRealized = true;
	// We pass in an already realized context on construction so can just return true here
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

LocalGLContext::LocalGLContext(wxGLCanvas *win, const wxGLContext* other /* for sharing display lists */ )
	: wxGLContext(win, other)
{
}

void LocalGLContext::ReleaseContext(const wxGLCanvas& win)
{
#if defined(__WXMSW__)
    wglMakeCurrent((HDC) win.GetHDC(), NULL);
#endif
}

#endif
