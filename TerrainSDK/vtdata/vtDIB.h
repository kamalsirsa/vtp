//
// vtDIB.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_DIBH
#define VTDATA_DIBH

// for non-Win32 systems (or code which does include the Win32 headers),
// define some Microsoft types used by the DIB code
#ifdef _WINGDI_

typedef DWORD dword;

#else

typedef unsigned long dword;
typedef unsigned short word;
typedef unsigned char byte;

#ifdef _MSC_VER
#include <pshpack2.h>
#endif

#ifdef __GNUC__
#  define PACKED __attribute__((packed))
#else
#  define PACKED
#endif

typedef struct tagBITMAPFILEHEADER {
	word	bfType;
	dword   bfSize;
	word	bfReserved1;
	word	bfReserved2;
	dword   bfOffBits;
} PACKED BITMAPFILEHEADER;

#ifdef _MSC_VER
#include <poppack.h>
#endif

typedef struct tagBITMAPINFOHEADER{
	dword	biSize;
	long	biWidth;
	long	biHeight;
	word	biPlanes;
	word	biBitCount;
	dword	biCompression;
	dword	biSizeImage;
	long	biXPelsPerMeter;
	long	biYPelsPerMeter;
	dword	biClrUsed;
	dword	biClrImportant;
} PACKED BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
	byte	rgbBlue;
	byte	rgbGreen;
	byte	rgbRed;
	byte	rgbReserved;
} PACKED RGBQUAD;

/* constants for the biCompression field */
#define BI_RGB			0L
#define BI_RLE8			1L
#define BI_RLE4			2L
#define BI_BITFIELDS	3L

#define RGB(r,g,b)		  ((dword)(((byte)(r)|((word)((byte)(g))<<8))|(((dword)(byte)(b))<<16)))

#define GetRValue(rgb)	  ((byte)(rgb))
#define GetGValue(rgb)	  ((byte)(((word)(rgb)) >> 8))
#define GetBValue(rgb)	  ((byte)((rgb)>>16))

#endif // #ifndef _WINGDI_


/**
 * A DIB is a Device-Independent Bitmap.  It is a way of representing a
 * bitmap in memory which has its origins in early MS Windows usage, but
 * is entirely applicable to normal bitmap operations.
 */
class vtDIB
{
public:
	vtDIB();
	vtDIB(const char *fname);		// load new DIB from file
	vtDIB(int width, int height, int bitdepth, bool create_palette);	// create new DIB
	vtDIB(void *pDIB);				// wrap an existing DIB
	~vtDIB();

	bool ReadBMP(const char *fname);
	bool WriteBMP(const char *fname);
//	bool ReadPNG(const char *filename);

	unsigned long GetPixel24(int x, int y);
	void SetPixel24(int x, int y, dword color);

	unsigned char GetPixel8(int x, int y);
	void SetPixel8(int x, int y, byte color);

	bool GetPixel1(int x, int y);
	void SetPixel1(int x, int y, bool color);

	int GetWidth() { return m_iWidth; }
	int GetHeight() { return m_iHeight; }
	int GetDepth() { return m_iBitCount; }

	void *GetHandle() { return m_pDIB; }
	BITMAPINFOHEADER *GetDIBHeader() { return m_Hdr; }
	void *GetDIBData() { return m_Data; }

	void Lock();
	void Unlock();
	void LeaveInternalDIB(bool bLeaveIt);

	bool	m_bLoadedSuccessfully;

private:
	void _ComputeByteWidth();

	bool	m_bLeaveIt;

	// When locked, these two fields point to the header and data
	BITMAPINFOHEADER *m_Hdr;
	void	*m_Data;

	void	*m_pDIB;
	int		m_iWidth, m_iHeight, m_iBitCount;
	int		m_iByteWidth;
	int		m_iPaletteSize;
};

#endif
