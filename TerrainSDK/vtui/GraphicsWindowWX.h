#pragma once

#ifdef USE_OSG_VIEWER

#include <osgViewer/GraphicsWindow>


class LocalGLContext;
class wxGLCanvas;

class GraphicsWindowWX : public osgViewer::GraphicsWindow
{
public:
	GraphicsWindowWX(wxGLCanvas* pCanvas);

	void CloseOsgContext();

    bool makeCurrentImplementation();
    void swapBuffersImplementation();

    virtual bool releaseContextImplementation();
	
    virtual void closeImplementation();

    virtual bool realizeImplementation();

	virtual bool valid() const;

    virtual bool isRealizedImplementation() const;
private:
	wxGLCanvas* m_pCanvas;
	LocalGLContext *m_pGLContext;
	bool m_bIsRealized;
	bool m_bValid;
};
#endif
