//
// vtDIB.h
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_DIBH
#define VTDATA_DIBH

#include "MathTypes.h"

class vtProjection;

/**
 * An abstract class which defines the basic functionality that any bitmap must expose.
 */
class vtBitmapBase
{
public:
	virtual ~vtBitmapBase() {}

	virtual unsigned char GetPixel8(int x, int y) const = 0;
	virtual void GetPixel24(int x, int y, RGBi &rgb) const = 0;
	virtual void GetPixel32(int x, int y, RGBAi &rgba) const = 0;

	virtual void SetPixel8(int x, int y, unsigned char color) = 0;
	virtual void SetPixel24(int x, int y, const RGBi &rgb) = 0;
	virtual void SetPixel32(int x, int y, const RGBAi &rgba) = 0;

	virtual unsigned int GetWidth() const = 0;
	virtual unsigned int GetHeight() const = 0;
	virtual unsigned int GetDepth() const = 0;

	void ScalePixel8(int x, int y, float fScale);
	void ScalePixel24(int x, int y, float fScale);
	void ScalePixel32(int x, int y, float fScale);
	void BlitTo(vtBitmapBase &target, int x, int y);
};

/**
 * This virtual bitmap encapsulates a grid of 4x4 bitmap tiles.
 * After it is created, it can be treated just like any other bitmap.
 */
class vtOverlappedTiledBitmap : public vtBitmapBase
{
public:
	unsigned char GetPixel8(int x, int y) const;
	void GetPixel24(int x, int y, RGBi &rgb) const;
	void GetPixel32(int x, int y, RGBAi &rgba) const;

	void SetPixel8(int x, int y, unsigned char color);
	void SetPixel24(int x, int y, const RGBi &rgb);
	void SetPixel32(int x, int y, const RGBAi &rgba);

	unsigned int GetWidth() const;
	unsigned int GetHeight() const;
	unsigned int GetDepth() const;

	virtual vtBitmapBase *GetTile(int i, int j) = 0;
	virtual const vtBitmapBase *GetTile(int i, int j) const = 0;

	int m_iTilesize, m_iSpacing;	// space = size - 1
};

// for non-Win32 systems (or code which doesn't include the Win32 headers),
// define some Microsoft types used by the DIB code
#ifdef _WINGDI_
//  #ifndef _WINDEF_
	typedef DWORD dword;
//  #endif
#else
	typedef unsigned long dword;
	typedef unsigned short word;
	typedef unsigned char byte;
	typedef struct tagBITMAPINFOHEADER BITMAPINFOHEADER;
	#define RGB(r,g,b)		  ((dword)(((byte)(r)|((word)((byte)(g))<<8))|(((dword)(byte)(b))<<16)))
	#define GetRValue(rgb)	  ((byte)(rgb))
	#define GetGValue(rgb)	  ((byte)(((word)(rgb)) >> 8))
	#define GetBValue(rgb)	  ((byte)((rgb)>>16))
#endif

/**
 * A DIB is a Device-Independent Bitmap.  It is a way of representing a
 * bitmap in memory which has its origins in early MS Windows usage, but
 * is entirely applicable to normal bitmap operations.
 */
class vtDIB : public vtBitmapBase
{
public:
	vtDIB();
	vtDIB(void *pDIB);	// wrap an existing DIB
	virtual ~vtDIB();

	bool Create(int width, int height, int bitdepth, bool create_palette = false);
	bool Create24From8bit(const vtDIB &from);

	bool Read(const char *fname, bool progress_callback(int) = NULL);
	bool ReadBMP(const char *fname, bool progress_callback(int) = NULL);
	bool ReadJPEG(const char *fname, bool progress_callback(int) = NULL);
	bool ReadPNG(const char *fname, bool progress_callback(int) = NULL);

	bool WriteBMP(const char *fname);
	bool WriteJPEG(const char *fname, int quality, bool progress_callback(int) = NULL);
	bool WritePNG(const char *fname);
	bool WriteTIF(const char *fname, const DRECT *area = NULL,
		const vtProjection *proj = NULL, bool progress_callback(int) = NULL);

	unsigned int GetPixel24(int x, int y) const;
	void GetPixel24(int x, int y, RGBi &rgb) const;
	void GetPixel24From8bit(int x, int y, RGBi &rgb) const;
	void SetPixel24(int x, int y, dword color);
	void SetPixel24(int x, int y, const RGBi &rgb);

	void GetPixel32(int x, int y, RGBAi &rgba) const;
	void SetPixel32(int x, int y, const RGBAi &rgba);

	unsigned char GetPixel8(int x, int y) const;
	void SetPixel8(int x, int y, unsigned char color);

	bool GetPixel1(int x, int y) const;
	void SetPixel1(int x, int y, bool color);

	void SetColor(const RGBi &rgb);
	void Invert();
	void Blit(vtDIB &target, int x, int y);

	unsigned int GetWidth() const { return m_iWidth; }
	unsigned int GetHeight() const { return m_iHeight; }
	unsigned int GetDepth() const { return m_iBitCount; }

	void *GetHandle() const { return m_pDIB; }
	BITMAPINFOHEADER *GetDIBHeader() const { return m_Hdr; }
	void *GetDIBData() const { return m_Data; }

	void LeaveInternalDIB(bool bLeaveIt);

	bool	m_bLoadedSuccessfully;

private:
	void _ComputeByteWidth();

	bool	m_bLeaveIt;

	// The DIB's header and data
	BITMAPINFOHEADER *m_Hdr;
	void	*m_Data;

	void	*m_pDIB;
	unsigned int	m_iWidth, m_iHeight, m_iBitCount, m_iByteCount;
	unsigned int	m_iByteWidth;
	unsigned int	m_iPaletteSize;
};

#endif	// VTDATA_DIBH

