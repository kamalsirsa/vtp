
#ifndef CBIMAGE_H
#define CBIMAGE_H

class CGBM;
class CDib;

class CBImage
{
public:
	CBImage();
	~CBImage();

	bool LoadFromFile(const char *szPathName);
	bool LoadTFW(const char *szPathName);

	CGBM	*m_pSourceGBM;
//	CGBM	*m_pMonoGBM;
	CDib	*m_pSourceDIB, *m_pMonoDIB, *m_pCurrentDIB;
	CBitmap m_bmp;
	bool	m_initialized;

	CPoint	m_PixelSize;			// size of bitmap from the file
	float	m_xUTMoffset;			//data obtained from image world file (.tfw)
	float	m_yUTMoffset;
	float	m_fImageWidth;		// in UTM meters
	float	m_fImageHeight;		// same
	float	m_xMetersPerPixel;
	float	m_yMetersPerPixel;
};

#endif CBIMAGE_H
