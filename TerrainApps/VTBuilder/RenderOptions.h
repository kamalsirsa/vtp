
#ifndef __RenderOptions_H__
#define __RenderOptions_H__

class RenderOptions
{
public:
	int  m_iSizeX;
	int  m_iSizeY;
	wxString   m_strColorMap;
	bool m_bToFile;
	RGBi m_ColorNODATA;
	bool m_bShading;
	wxString m_strToFile;
	bool m_bJPEG;
};

#endif


