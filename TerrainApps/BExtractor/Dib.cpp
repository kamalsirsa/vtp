////////////////////////////////////////////////////////////////
// Copyright 1996 Microsoft Systems Journal. 
// If this program works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
// CDib - Device Independent Bitmap.
// This implementation draws bitmaps using normal Win32 API functions,
// not DrawDib. CDib is derived from CBitmap, so you can use it with
// any other MFC functions that use bitmaps.
//
// Changes 8/99: added constructor to convert from DIB to CDib
//
#include "StdAfx.h"
#include "Dib.h"

// GBM
#include "GBMWrapper.h"

// GDAL
#include "gdal_priv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const  int		MAXPALCOLORS = 256;

IMPLEMENT_DYNAMIC(CDib, CObject)

CDib::CDib()
{
	memset(&m_bm, 0, sizeof(m_bm));
	m_hdd = NULL;
}

// 203 0 23
// 137 51 128
// 218 179 214

CDib::CDib(CDC* pDC, CGBM *pGBM, HDRAWDIB hdd)
{
	m_hdd = hdd;

	m_bmi = pGBM->GetDIBHeader();
	m_colors = (RGBQUAD *) (m_bmi + 1);
	m_stride = ( ((m_bmi->biWidth * m_bmi->biBitCount + 31)/32) * 4 );

	HBITMAP hbm = CreateDIBSection(pDC->GetSafeHdc(),
							(BITMAPINFO *) m_bmi, 
							DIB_RGB_COLORS,
							&m_data,
							NULL, 
							0);
	//copy the data to the new location
	memcpy(m_data,pGBM->m_data, pGBM->m_datasize);

	BOOL result = Attach(hbm);
}

CDib::CDib(CDC* pDC, CSize &size, HDRAWDIB hdd)
{
	m_hdd = hdd;

	int width = size.cx;
	int height = size.cy;
	int bpp = 8;
	m_stride = ( ((width * bpp + 31)/32) * 4 );
	int datasize = (size_t) (m_stride * height);

	void *buf = malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
	m_bmi = (BITMAPINFOHEADER *)buf;
	m_colors = (RGBQUAD *) (m_bmi + 1);
	for (int i = 0; i < 256; i++)
	{
		m_colors[i].rgbBlue = i;
		m_colors[i].rgbGreen = i;
		m_colors[i].rgbRed = i;
		m_colors[i].rgbReserved = 0;
	}

	m_bmi->biSize = sizeof(BITMAPINFOHEADER);
    m_bmi->biWidth = width;
    m_bmi->biHeight = height;
    m_bmi->biPlanes = 1;
	m_bmi->biBitCount = bpp;
	m_bmi->biCompression = BI_RGB;
	m_bmi->biSizeImage = datasize;
	m_bmi->biXPelsPerMeter = 1;
	m_bmi->biYPelsPerMeter = 1;
	m_bmi->biClrUsed = (bpp == 8) ? 256 : 0;
	m_bmi->biClrImportant = m_bmi->biClrUsed;

	HBITMAP hbm = CreateDIBSection(pDC->GetSafeHdc(),
							(BITMAPINFO *) m_bmi, 
							DIB_RGB_COLORS,
							&m_data,
							NULL, 
							0);
	BOOL result = Attach(hbm);
}

CDib::CDib(CDC* pDC, GDALRasterBand *poBand, HDRAWDIB hdd)
{
	m_hdd = hdd;

	int width = poBand->GetXSize();
	int height = poBand->GetYSize();
	int bpp = 8;
	m_stride = ( ((width * bpp + 31)/32) * 4 );
	int datasize = (size_t) (m_stride * height);

	void *buf = malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
	m_bmi = (BITMAPINFOHEADER *)buf;
	m_colors = (RGBQUAD *) (m_bmi + 1);
	for (int i = 0; i < 256; i++)
	{
		m_colors[i].rgbBlue = i;
		m_colors[i].rgbGreen = i;
		m_colors[i].rgbRed = i;
		m_colors[i].rgbReserved = 0;
	}

	m_bmi->biSize = sizeof(BITMAPINFOHEADER);
    m_bmi->biWidth = width;
    m_bmi->biHeight = height;
    m_bmi->biPlanes = 1;
	m_bmi->biBitCount = bpp;
	m_bmi->biCompression = BI_RGB;
	m_bmi->biSizeImage = datasize;
	m_bmi->biXPelsPerMeter = 1;
	m_bmi->biYPelsPerMeter = 1;
	m_bmi->biClrUsed = bpp == 8 ? 256 : 0;
	m_bmi->biClrImportant = 0;

	HBITMAP hbm = CreateDIBSection(pDC->GetSafeHdc(),
							(BITMAPINFO *) m_bmi, 
							DIB_RGB_COLORS,
							&m_data,
							NULL, 
							0);

	//copy the data to the new location
	char *pasScanline;
	pasScanline = (char *) CPLMalloc(sizeof(char)*width);
	int j;
	for (j = 0; j < height; j++)
	{
		poBand->RasterIO( GF_Read, 0, j, width, 1,
						  pasScanline, width, 1, GDT_Byte,
						  0, 0 );

		char *line = ((char *)m_data) + (j * m_stride);
		memcpy(line, pasScanline, width);
	}

	// Clean up
	CPLFree(pasScanline);

	BOOL result = Attach(hbm);
}

CDib::~CDib()
{
	DeleteObject();
}

//////////////////
// Delete Object. Delete DIB and palette.
//
BOOL CDib::DeleteObject()
{
	m_pal.DeleteObject();
	memset(&m_bm, 0, sizeof(m_bm));
	return CBitmap::DeleteObject();
}

//////////////////
// Read DIB from file.
//
BOOL CDib::Load(LPCTSTR szPathName)
{
	return Attach(::LoadImage(NULL, szPathName, IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE));
}

//////////////////
// Load bitmap resource. Never tested.
//
BOOL CDib::Load(HINSTANCE hInst, LPCTSTR lpResourceName)
{
	return Attach(::LoadImage(hInst, lpResourceName, IMAGE_BITMAP, 0, 0,
		LR_CREATEDIBSECTION | LR_DEFAULTSIZE));
}

//////////////////
// Attach is just like the CGdiObject version,
// except it also creates the palette
//
BOOL CDib::Attach(HGDIOBJ hbm)
{
	if (CBitmap::Attach(hbm)) {
		if (!GetBitmap(&m_bm))			// load BITMAP for speed
			return FALSE;
		m_pal.DeleteObject();			// in case one is already there
		return CreatePalette(m_pal);	// create palette
	}
	return FALSE;	
}

//////////////////
// Get size (width, height) of bitmap.
// extern fn works for ordinary CBitmap objects.
//
CSize GetBitmapSize(CBitmap* pBitmap)
{
	BITMAP bm;
	return pBitmap->GetBitmap(&bm) ?
		CSize(bm.bmWidth, bm.bmHeight) : CSize(0,0);
}

//////////////////
// You can use this static function to draw ordinary
// CBitmaps as well as CDibs
//
BOOL DrawBitmap(CDC& dc, CBitmap* pBitmap,
	const CRect* rcDst, const CRect* rcSrc)
{
	// Compute rectangles where NULL specified
	CRect rc;
	if (!rcSrc) {
		// if no source rect, use whole bitmap
		rc = CRect(CPoint(0,0), GetBitmapSize(pBitmap));
		rcSrc = &rc;
	}
	if (!rcDst) {
		// if no destination rect, use source
		rcDst=rcSrc;
	}

	// Create memory DC
	CDC memdc;
	memdc.CreateCompatibleDC(&dc);
	CBitmap* pOldBm = memdc.SelectObject(pBitmap);

	// Blast bits from memory DC to target DC.
	// Use StretchBlt if size is different.
	//
	BOOL bRet = FALSE;
	if (rcDst->Size()==rcSrc->Size()) {
		bRet = dc.BitBlt(rcDst->left, rcDst->top, 
			rcDst->Width(), rcDst->Height(),
			&memdc, rcSrc->left, rcSrc->top, SRCCOPY);
	} else {
		dc.SetStretchBltMode(COLORONCOLOR);
		bRet = dc.StretchBlt(rcDst->left, rcDst->top, rcDst->Width(),
			rcDst->Height(), &memdc, rcSrc->left, rcSrc->top, rcSrc->Width(),
			rcSrc->Height(), SRCCOPY);
	}
	memdc.SelectObject(pOldBm);

	return bRet;
}

////////////////////////////////////////////////////////////////
// Draw DIB on caller's DC. Does stretching from source to destination
// rectangles. Generally, you can let the following default to zero/NULL:
//
//		bUseDrawDib = whether to use use DrawDib, default TRUE
//		pPal	      = palette, default=NULL, (use DIB's palette)
//		bForeground = realize in foreground (default FALSE)
//
// If you are handling palette messages, you should use bForeground=FALSE,
// since you will realize the foreground palette in WM_QUERYNEWPALETTE.
//
BOOL CDib::Draw(CDC& dc, const CRect* rcDst, const CRect* rcSrc,
	BOOL bUseDrawDib, CPalette* pPal, BOOL bForeground)
{
	if (!m_hObject)
		return FALSE;

	// Select, realize palette
	if (pPal==NULL)				// no palette specified:
		pPal = GetPalette();		// use default
	CPalette* pOldPal = dc.SelectPalette(pPal, !bForeground);
	dc.RealizePalette();

	BOOL bRet = FALSE;
	if (bUseDrawDib) {
		// Compute rectangles where NULL specified
		//
		CRect rc(0,0,-1,-1);	// default for DrawDibDraw
		if (!rcSrc)
			rcSrc = &rc;
		if (!rcDst)
			rcDst=rcSrc;
		if (!m_hdd)				
			VERIFY(m_hdd = DrawDibOpen());	//should never get called here anymore

		GetDIBFromSection();

		// Let DrawDib do the work!
		bRet = DrawDibDraw(m_hdd, dc,
			rcDst->left, rcDst->top, rcDst->Width(), rcDst->Height(),
			m_bmi,			// ptr to BITMAPINFOHEADER + colors
			m_bm.bmBits,	// bits in memory
			rcSrc->left, rcSrc->top, rcSrc->Width(), rcSrc->Height(),
			bForeground ? 0 : DDF_BACKGROUNDPAL);

	} else {
		// use normal draw function
		bRet = DrawBitmap(dc, this, rcDst, rcSrc);
	}
	if (pOldPal)
		dc.SelectPalette(pOldPal, TRUE);
	return bRet;
}


void CDib::GetDIBFromSection()
{
	// Get BITMAPINFOHEADER/color table. I copy into stack object each time.
	// This doesn't seem to slow things down visibly.
	//
	DIBSECTION ds;

	VERIFY(GetObject(sizeof(ds), &ds)==sizeof(ds));
//	char buf[sizeof(BITMAPINFOHEADER) + MAXPALCOLORS*sizeof(RGBQUAD)];
//	BITMAPINFOHEADER& bmih = *(BITMAPINFOHEADER*)buf;
//	RGBQUAD* colors = (RGBQUAD*)(m_bmi+1);
	memcpy(m_bmi, &ds.dsBmih, sizeof(BITMAPINFOHEADER));
	GetColorTable(m_colors, MAXPALCOLORS);
}

#define PALVERSION 0x300	// magic number for LOGPALETTE

//////////////////
// Create the palette. Use halftone palette for hi-color bitmaps.
//
BOOL CDib::CreatePalette(CPalette& pal)
{ 
	// should not already have palette
	ASSERT(pal.m_hObject==NULL);

	BOOL bRet = FALSE;
	RGBQUAD* colors = new RGBQUAD[MAXPALCOLORS];
	UINT nColors = GetColorTable(colors, MAXPALCOLORS);
	if (nColors > 0) {
		// Allocate memory for logical palette 
		int len = sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * nColors;
		LOGPALETTE* pLogPal = (LOGPALETTE*)new char[len];
		if (!pLogPal)
			return NULL;

		// set version and number of palette entries
		pLogPal->palVersion = PALVERSION;
		pLogPal->palNumEntries = nColors;

		// copy color entries 
		for (UINT i = 0; i < nColors; i++) {
			pLogPal->palPalEntry[i].peRed   = colors[i].rgbRed;
			pLogPal->palPalEntry[i].peGreen = colors[i].rgbGreen;
			pLogPal->palPalEntry[i].peBlue  = colors[i].rgbBlue;
			pLogPal->palPalEntry[i].peFlags = 0;
		}

		// create the palette and destroy LOGPAL
		bRet = pal.CreatePalette(pLogPal);
		delete [] (char*)pLogPal;
	} else {
		CWindowDC dcScreen(NULL);
		bRet = pal.CreateHalftonePalette(&dcScreen);
	}
	delete colors;
	return bRet;
}

//////////////////
// Helper to get color table. Does all the mem DC voodoo.
//
UINT CDib::GetColorTable(RGBQUAD* colorTab, UINT nColors)
{
	CWindowDC dcScreen(NULL);
	CDC memdc;
	memdc.CreateCompatibleDC(&dcScreen);
	CBitmap* pOldBm = memdc.SelectObject(this);
	nColors = GetDIBColorTable(memdc, 0, nColors, colorTab);
	memdc.SelectObject(pOldBm);
	return nColors;
}

void CDib::GetPixel24(int x, int y, RGBQUAD &rgb)
{
	byte i = ((char *)m_data)[y * m_stride + x];
	rgb = m_colors[i];
}

void CDib::SetPixel8(int x, int y, byte val)
{
	((char *)m_data)[y * m_stride + x] = val;
}

byte CDib::GetPixel8(int x, int y)
{
	return ((char *)m_data)[y * m_stride + x];
}

///////////////////////////////////////////

CDib *CreateMonoDib(CDC *pDC, CDib *pDib, HDRAWDIB hdd)
{
	pDib->GetDIBFromSection();

	CSize size = pDib->GetSize();
	CDib *pNew = new CDib(pDC, size, hdd);

	BITMAPINFOHEADER *header = pNew->GetDIBHeader();
	RGBQUAD* colors = (RGBQUAD*) (header + 1);
	int i;
	for (i = 0; i < 256; i++)
	{
		colors[i].rgbRed = i;
		colors[i].rgbGreen = i;
		colors[i].rgbBlue = i;
		colors[i].rgbReserved = 0;
	}

	int x, y;
	RGBQUAD rgb;
	byte color8;

	for (y = 0; y < size.cy; y++)
	{
		for (x = 0; x < size.cx; x++)
		{
			pDib->GetPixel24(x, y, rgb);
			 //if color is black, white, or gray, don't change it
			if (rgb.rgbRed == rgb.rgbGreen && rgb.rgbGreen == rgb.rgbBlue)
				color8 = rgb.rgbBlue;
			else
				color8 = 0xff;
			pNew->SetPixel8(x, y, color8);
		}
	}
	return pNew;
}


