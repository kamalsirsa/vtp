//
// vtBitmap.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "wx/image.h"

#include "vtdata/vtLog.h"
#include "vtBitmap.h"


vtBitmap::vtBitmap()
{
	m_pBitmap = NULL;
#if !USE_DIBSECTIONS
	m_pImage = NULL;
#endif
}

vtBitmap::~vtBitmap()
{
	if (m_pBitmap)
		delete m_pBitmap;
#if !USE_DIBSECTIONS
	if (m_pImage)
		delete m_pImage;
#endif
}

bool vtBitmap::Allocate(int iXSize, int iYSize)
{
#if USE_DIBSECTIONS
	BITMAPINFO ScanlineFormat =
	{
		{
			sizeof(BITMAPINFOHEADER), // Sizeof structure
			0, 0,	// width, height
			1,		// number of bit planes (always one)
			24,		// number of bits per pixel
			BI_RGB,	// compression
			0,		// size of image data (if BI_RGB then only for Dib Sections)
			1, 1,	// pixels per meter X  and Y
			0, 0	// colours used & important (0 for 24 bits per pixel)
		}, 0
	};
	ScanlineFormat.bmiHeader.biWidth = iXSize;
	ScanlineFormat.bmiHeader.biHeight = -iYSize;
	ScanlineFormat.bmiHeader.biSizeImage = iXSize * iYSize;

	m_iScanlineWidth = (((iXSize)*(24) + 31) / 32 * 4);

	// The following sets the m_pScanline pointer, which points to a giant
	// block of directly accesible image data!
	HBITMAP hSection;
	hSection = CreateDIBSection(NULL, &ScanlineFormat, DIB_RGB_COLORS,
		(void**)&m_pScanline, NULL, 0);
	if (hSection == NULL)
		return false;

	m_pBitmap = new wxBitmap();
	m_pBitmap->SetHBITMAP((WXHBITMAP)hSection);
	m_pBitmap->SetWidth(ScanlineFormat.bmiHeader.biWidth);
//	m_pBitmap->SetHeight(ScanlineFormat.bmiHeader.biHeight);
	// A little hack to make the call to StretchBlt work: negative height
	//  matches the behavior of a normal BITMAP?
	m_pBitmap->SetHeight(-ScanlineFormat.bmiHeader.biHeight);
	m_pBitmap->SetDepth(24);
#else
	// yes, we could use some error-checking here
	m_pImage = new wxImage(iXSize, iYSize);
	if (!m_pImage->Ok())
	{
		delete m_pImage;
		m_pImage = NULL;
		return false;
	}
	m_pBitmap = new wxBitmap(m_pImage);
	if (!m_pBitmap->Ok())
	{
		delete m_pBitmap;
		delete m_pImage;
		m_pBitmap = NULL;
		m_pImage = NULL;
		return false;
	}
#endif
	return true;
}

void vtBitmap::SetPixel24(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
#if USE_DIBSECTIONS
	*(m_pScanline + (y * m_iScanlineWidth) + (x * 3)) = b;
	*(m_pScanline + (y * m_iScanlineWidth) + (x * 3) + 1) = g;
	*(m_pScanline + (y * m_iScanlineWidth) + (x * 3) + 2) = r;
#else
	if (x == 0 && y == 0)
		VTLOG(" Setting pixel 0 0 to %d %d %d\n", r, g, b);
	m_pImage->SetRGB(x, y, r, g, b);
#endif
}

void vtBitmap::GetPixel24(int x, int y, RGBi &rgb) const
{
#if USE_DIBSECTIONS
	rgb.b = *(m_pScanline + (y * m_iScanlineWidth) + (x * 3));
	rgb.g = *(m_pScanline + (y * m_iScanlineWidth) + (x * 3) + 1);
	rgb.r = *(m_pScanline + (y * m_iScanlineWidth) + (x * 3) + 2);
#else
	rgb.r = m_pImage->GetRed(x, y);
	rgb.g = m_pImage->GetGreen(x, y);
	rgb.b = m_pImage->GetBlue(x, y);
#endif
}

unsigned char vtBitmap::GetPixel8(int x, int y) const
{
	// unimplemented
	return 0;
}

void vtBitmap::SetPixel8(int x, int y, unsigned char color)
{
	// unimplemented
}

unsigned int vtBitmap::GetWidth() const
{
	return m_pBitmap->GetWidth();
}

unsigned int vtBitmap::GetHeight() const
{
	return m_pBitmap->GetHeight();
}

unsigned int vtBitmap::GetDepth() const
{
	// not fully implemented
	return 24;
}

//
// If we aren't using DIBSections, then we don't have direct access to the
// image data, so we must copy from the image to the bitmap when we want
// changes to affect the drawn image.
//
void vtBitmap::ContentsChanged()
{
#if !USE_DIBSECTIONS
	VTLOG("Updating Bitmap Contents from Image Data\n");
	if (!m_pImage)
		return;
	if (m_pBitmap)
		delete m_pBitmap;
	m_pBitmap = new wxBitmap(m_pImage);
#endif
}
