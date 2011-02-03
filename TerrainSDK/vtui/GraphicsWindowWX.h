#pragma once

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
#ifndef __WXMAC__
	LocalGLContext *m_pGLContext;
#endif
	bool m_bIsRealized;
	bool m_bValid;
};
