
#ifndef CBIMAGE_H
#define CBIMAGE_H

class CDib;
class OGRSpatialReference;

class CBImage
{
public:
	CBImage();
	~CBImage();

	bool LoadFromFile(const char *szPathName, CDC *pDC, HDRAWDIB hdd);
	bool LoadTFW(const char *szPathName);

private:
	bool LoadGDAL(const char *szPathName, CDC *pDC, HDRAWDIB hdd);

public:
	OGRSpatialReference *m_pSpatialReference;
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
