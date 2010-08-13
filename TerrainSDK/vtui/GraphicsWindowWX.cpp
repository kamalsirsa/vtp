#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef USE_OSG_VIEWER

#include "wx/glcanvas.h"
#include "GraphicsWindowWX.h"

GraphicsWindowWX::GraphicsWindowWX(wxGLCanvas* pCanvas)
{
	m_pCanvas = pCanvas;
	wxPoint pos = m_pCanvas->GetPosition();
	wxSize  size = m_pCanvas->GetSize();

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

bool GraphicsWindowWX::makeCurrentImplementation()
{
    m_pCanvas->SetCurrent();
    return true;
}

void GraphicsWindowWX::swapBuffersImplementation()
{
    m_pCanvas->SwapBuffers();
}
#endif
