#pragma once

#ifdef USE_OSG_VIEWER

#include <osgViewer/GraphicsWindow>

class wxGLCanvas;
class GraphicsWindowWX : public osgViewer::GraphicsWindow
{
public:
	GraphicsWindowWX(wxGLCanvas* pCanvas);


	void InvalidateCanvas() { m_bCanvasValid = false; }

    bool makeCurrentImplementation();
    void swapBuffersImplementation();

	// No portable way to do this on wxWidgets
    virtual bool releaseContextImplementation() { return true; }
	
    // Cannot think of anything to do here at the moment
    virtual void closeImplementation() {}

	// We pass in an already realized context on construction so can just return true here
    virtual bool realizeImplementation() { return true; }
	// and here
    virtual bool valid() const { return true; }

    virtual bool isRealizedImplementation() const  { return m_bCanvasValid; }
private:
	wxGLCanvas* m_pCanvas;
	bool m_bCanvasValid;
};
#endif
