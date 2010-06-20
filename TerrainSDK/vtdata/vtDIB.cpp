//
// vtDIB.cpp
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include <stdlib.h>

// Headers for PNG support, which uses the library "libpng"
#include "png.h"

// compatibility with libpng 1.4
#if ((PNG_LIBPNG_VER_MAJOR==1 && PNG_LIBPNG_VER_MINOR>=4) || PNG_LIBPNG_VER_MAJOR>1)
#define png_infopp_NULL NULL
#endif

// Headers for JPEG support, which uses the library "libjpeg"
extern "C" {
#include "jpeglib.h"
}

#include "vtDIB.h"
#include "vtLog.h"
#include "ByteOrder.h"
#include "FilePath.h"

// GDAL for TIFF support
#include "gdal_priv.h"
#include "Projections.h"

///////////////////////////////////////////////////////////////////////
// Base class vtBitmapBase
//

void vtBitmapBase::ScalePixel8(int x, int y, float fScale)
{
	unsigned int texel = GetPixel8(x, y);
	texel = (int) (texel * fScale);
	if (texel > 255)
		texel = 255;
	SetPixel8(x, y, (unsigned char) texel);
}

void vtBitmapBase::ScalePixel24(int x, int y, float fScale)
{
	RGBi rgb;
	GetPixel24(x, y, rgb);
	rgb *= fScale;
	if (rgb.r > 255) rgb.r = 255;
	if (rgb.g > 255) rgb.g = 255;
	if (rgb.b > 255) rgb.b = 255;
	SetPixel24(x, y, rgb);
}

void vtBitmapBase::ScalePixel32(int x, int y, float fScale)
{
	RGBAi rgba;
	GetPixel32(x, y, rgba);
	rgba.MultRGB(fScale);
	if (rgba.r > 255) rgba.r = 255;
	if (rgba.g > 255) rgba.g = 255;
	if (rgba.b > 255) rgba.b = 255;
	SetPixel32(x, y, rgba);
}

void vtBitmapBase::BlitTo(vtBitmapBase &target, int x, int y)
{
	int depth = GetDepth();
	int tdepth = target.GetDepth();

	unsigned int w = GetWidth();
	unsigned int h = GetHeight();
	int tw = target.GetWidth();
	int th = target.GetHeight();

	unsigned int i, j;
	RGBi rgb;
	RGBAi rgba;
	for (i = 0; i < w; i++)
	{
		for (j = 0; j < h; j++)
		{
			int tx = i+x, ty = j+y;
			if (tx < 0 || tx > tw-1 || ty < 0 || ty > th-1)
				continue;

			if (depth == 8 && tdepth == 8)
			{
				unsigned char value = GetPixel8(i, j);
				target.SetPixel8(tx, ty, value);
			}
			else if (tdepth == 24)
			{
				GetPixel24(i, j, rgb);
				target.SetPixel24(tx, ty, rgb);
			}
			else if (tdepth == 32)
			{
				GetPixel32(i, j, rgba);
				target.SetPixel32(tx, ty, rgba);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////
// Tiled class vtOverlappedTiledBitmap
//
unsigned char vtOverlappedTiledBitmap::GetPixel8(int x, int y) const
{
	int c = x / m_iSpacing;
	int i = x % m_iSpacing;
	int r = y / m_iSpacing;
	int j = y % m_iSpacing;
	return GetTile(c, r)->GetPixel8(i, j);
}
void vtOverlappedTiledBitmap::GetPixel24(int x, int y, RGBi &rgb) const
{
	int c = x / m_iSpacing;
	if (c == 4) c = 3;
	int i = x - (c * m_iSpacing);
	int r = y / m_iSpacing;
	if (r == 4) r = 3;
	int j = y - (r * m_iSpacing);

	GetTile(c, r)->GetPixel24(i, j, rgb);
}
void vtOverlappedTiledBitmap::GetPixel32(int x, int y, RGBAi &rgba) const
{
	int c = x / m_iSpacing;
	int i = x % m_iSpacing;
	int r = y / m_iSpacing;
	int j = y % m_iSpacing;
	GetTile(c, r)->GetPixel32(i, j, rgba);
}

void vtOverlappedTiledBitmap::SetPixel8(int x, int y, unsigned char color)
{
	// The array of images has edges which are shared.
	// When an internal edge of one image is modified, the matching edge
	// must also be modified, or there will be discontinuities on the seams.
	// This means one call to SetPixel might modify more than one destination
	// pixel: in fact, up to four pixels in the case of a tile corner.
	int c = x / m_iSpacing;
	int i = x % m_iSpacing;
	int r = y / m_iSpacing;
	int j = y % m_iSpacing;
	GetTile(c, r)->SetPixel8(i, j, color);

	if (c > 0 && i == 0)
		GetTile(c-1, r)->SetPixel8(m_iSpacing, j, color);
	if (r > 0 && j == 0)
		GetTile(c, r-1)->SetPixel8(i, m_iSpacing, color);
	if (c > 0 && i == 0 && r > 0 && j == 0)
		GetTile(c-1, r-1)->SetPixel8(m_iSpacing, m_iSpacing, color);
}
void vtOverlappedTiledBitmap::SetPixel24(int x, int y, const RGBi &rgb)
{
	int c = x / m_iSpacing;
	if (c == 4) c = 3;
	int i = x - (c * m_iSpacing);
	int r = y / m_iSpacing;
	if (r == 4) r = 3;
	int j = y - (r * m_iSpacing);

	GetTile(c, r)->SetPixel24(i, j, rgb);

	if (c > 0 && i == 0)
		GetTile(c-1, r)->SetPixel24(m_iSpacing, j, rgb);
	if (r > 0 && j == 0)
		GetTile(c, r-1)->SetPixel24(i, m_iSpacing, rgb);
	if (c > 0 && i == 0 && r > 0 && j == 0)
		GetTile(c-1, r-1)->SetPixel24(m_iSpacing, m_iSpacing, rgb);
}
void vtOverlappedTiledBitmap::SetPixel32(int x, int y, const RGBAi &rgba)
{
	int c = x / m_iSpacing;
	int i = x % m_iSpacing;
	int r = y / m_iSpacing;
	int j = y % m_iSpacing;
	GetTile(c, r)->SetPixel32(i, j, rgba);

	if (c > 0 && i == 0)
		GetTile(c-1, r)->SetPixel32(m_iSpacing, j, rgba);
	if (r > 0 && j == 0)
		GetTile(c, r-1)->SetPixel32(i, m_iSpacing, rgba);
	if (c > 0 && i == 0 && r > 0 && j == 0)
		GetTile(c-1, r-1)->SetPixel32(m_iSpacing, m_iSpacing, rgba);
}
unsigned int vtOverlappedTiledBitmap::GetWidth() const
{
	return m_iTilesize*4-3;
}
unsigned int vtOverlappedTiledBitmap::GetHeight() const
{
	return m_iTilesize*4-3;
}
unsigned int vtOverlappedTiledBitmap::GetDepth() const
{
	return GetTile(0, 0)->GetDepth();
}

///////////////////////////////////////////////////////////////////////
// Class vtDIB
//

#ifndef _WINGDI_

#ifdef _MSC_VER
#include <pshpack2.h>
#endif

#ifdef __GNUC__
#  define PACKED __attribute__((packed))
#else
#  define PACKED
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS

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

#endif	// DOXYGEN_SHOULD_SKIP_THIS

/* constants for the biCompression field */
#define BI_RGB			0L
#define BI_RLE8			1L
#define BI_RLE4			2L
#define BI_BITFIELDS	3L

#endif // #ifndef _WINGDI_

/**
 * Create a new empty DIB wrapper.
 */
vtDIB::vtDIB()
{
	m_bLoadedSuccessfully = false;
	m_bLeaveIt = false;
	m_pDIB = NULL;
}


vtDIB::vtDIB(void *pDIB)
{
	m_pDIB = pDIB;

	m_Hdr = (BITMAPINFOHEADER *) m_pDIB;
	m_Data = ((byte *)m_Hdr) + sizeof(BITMAPINFOHEADER) + m_iPaletteSize;

	m_iWidth = m_Hdr->biWidth;
	m_iHeight = m_Hdr->biHeight;
	m_iBitCount = m_Hdr->biBitCount;
	m_iByteCount = m_iBitCount/8;

	_ComputeByteWidth();
}


vtDIB::~vtDIB()
{
	// Only free the encapsulated DIB if we're allowed to
	if (!m_bLeaveIt)
	{
		if (m_pDIB != NULL)
			free(m_pDIB);
		m_pDIB = NULL;
	}
}


/**
 * Create a new DIB in memory.
 */
bool vtDIB::Create(int xsize, int ysize, int bitdepth, bool create_palette)
{
	m_iWidth = xsize;
	m_iHeight = ysize;
	m_iBitCount = bitdepth;
	m_iByteCount = m_iBitCount/8;

	_ComputeByteWidth();

	int ImageSize = m_iByteWidth * m_iHeight;

	int PaletteColors = create_palette ? 256 : 0;
	m_iPaletteSize = sizeof(RGBQUAD) * PaletteColors;

	int total_size = sizeof(BITMAPINFOHEADER) + m_iPaletteSize + ImageSize;
	m_pDIB = malloc(total_size);
	if (!m_pDIB)
	{
		VTLOG("Could not allocate %d bytes\n", total_size);
		return false;
	}

	m_Hdr = (BITMAPINFOHEADER *) m_pDIB;
	m_Data = ((byte *)m_Hdr) + sizeof(BITMAPINFOHEADER) + m_iPaletteSize;

	m_Hdr->biSize = sizeof(BITMAPINFOHEADER);
	m_Hdr->biWidth = xsize;
	m_Hdr->biHeight = ysize;
	m_Hdr->biPlanes = 1;
	m_Hdr->biBitCount = (word) bitdepth;
	m_Hdr->biCompression = BI_RGB;
	m_Hdr->biSizeImage = ImageSize;
	m_Hdr->biClrUsed = PaletteColors;
	m_Hdr->biClrImportant = 0;

	if (create_palette)
	{
		RGBQUAD *rgb=(RGBQUAD*)((char*)m_Hdr + sizeof(BITMAPINFOHEADER));
		for (int i=0; i<256; i++)
			rgb[i].rgbBlue = rgb[i].rgbGreen = rgb[i].rgbRed =
				(unsigned char)i;
	}
	m_bLeaveIt = false;
	return true;
}

bool vtDIB::Create24From8bit(const vtDIB &from)
{
	if (!Create(from.GetWidth(), from.GetHeight(), 24))
		return false;

	RGBi rgb;
	unsigned int i, j;
	for (i = 0; i < m_iWidth; i++)
	{
		for (j = 0; j < m_iHeight; j++)
		{
			from.GetPixel24From8bit(i, j, rgb);
			SetPixel24(i, j, rgb);
		}
	}
	return true;
}


/**
 * Read a image file into the DIB.  This method will check to see if the
 * file is a BMP or JPEG and call the appropriate reader.
 */
bool vtDIB::Read(const char *fname, bool progress_callback(int))
{
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;
	unsigned char buf[2];
	if (fread(buf, 2, 1, fp) != 1)
		return false;
	fclose(fp);
	if (buf[0] == 0x42 && buf[1] == 0x4d)
		return ReadBMP(fname, progress_callback);
	else if (buf[0] == 0xFF && buf[1] == 0xD8)
		return ReadJPEG(fname, progress_callback);
	else if (buf[0] == 0x89 && buf[1] == 0x50)
		return ReadPNG(fname, progress_callback);
	return false;
}


/**
 * Read a MSWindows-style .bmp file into the DIB.
 */
bool vtDIB::ReadBMP(const char *fname, bool progress_callback(int))
{
	BITMAPFILEHEADER	bitmapHdr;
	int MemorySize;

	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	// allocate enough room for the header
	m_pDIB = malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
	if (!m_pDIB)
		return false;

	m_Hdr = (BITMAPINFOHEADER *) m_pDIB;

	if (fread(&bitmapHdr, 14, 1, fp) == 0)
		goto ErrExit;
	bitmapHdr.bfType	= SwapBytes( (short)bitmapHdr.bfType  , BO_LE, BO_CPU );
	bitmapHdr.bfSize	= SwapBytes( (long)bitmapHdr.bfSize   , BO_LE, BO_CPU );
	bitmapHdr.bfOffBits = SwapBytes( (long)bitmapHdr.bfOffBits, BO_LE, BO_CPU );

	if (bitmapHdr.bfType != 0x4d42)
		goto ErrExit;

	if (fread(m_Hdr, sizeof(BITMAPINFOHEADER), 1, fp) == 0)
		goto ErrExit;
	m_Hdr->biSize			= SwapBytes( (long )m_Hdr->biSize		,BO_LE, BO_CPU );
	m_Hdr->biWidth			= SwapBytes( (long )m_Hdr->biWidth		,BO_LE, BO_CPU );
	m_Hdr->biHeight			= SwapBytes( (long )m_Hdr->biHeight		,BO_LE, BO_CPU );
	m_Hdr->biPlanes			= SwapBytes( (short)m_Hdr->biPlanes		,BO_LE, BO_CPU );
	m_Hdr->biBitCount		= SwapBytes( (short)m_Hdr->biBitCount	,BO_LE, BO_CPU );
	m_Hdr->biCompression	= SwapBytes( (long )m_Hdr->biCompression,BO_LE, BO_CPU );
	m_Hdr->biSizeImage		= SwapBytes( (long )m_Hdr->biSizeImage	,BO_LE, BO_CPU );
	m_Hdr->biXPelsPerMeter	= SwapBytes( (long )m_Hdr->biXPelsPerMeter,BO_LE, BO_CPU );
	m_Hdr->biYPelsPerMeter	= SwapBytes( (long )m_Hdr->biYPelsPerMeter,BO_LE, BO_CPU );
	m_Hdr->biClrUsed		= SwapBytes( (long )m_Hdr->biClrUsed	  ,BO_LE, BO_CPU );
	m_Hdr->biClrImportant	= SwapBytes( (long )m_Hdr->biClrImportant ,BO_LE, BO_CPU );

	if (m_Hdr->biCompression != BI_RGB)
	{
		if (m_Hdr->biBitCount == 32 &&
			m_Hdr->biCompression != BI_BITFIELDS)
		   goto ErrExit;
	}

	if (m_Hdr->biBitCount != 8
				&& m_Hdr->biBitCount != 16
				&& m_Hdr->biBitCount != 4
				&& m_Hdr->biBitCount != 32
			&& m_Hdr->biBitCount != 24)
	{
		goto ErrExit;
	}

	if (m_Hdr->biClrUsed == 0)
	{
		if ( m_Hdr->biBitCount != 24)
			m_Hdr->biClrUsed = 1 << m_Hdr->biBitCount;
	}

	if (m_Hdr->biSizeImage == 0)
	{
		m_Hdr->biSizeImage =
			((((m_Hdr->biWidth * (long )m_Hdr->biBitCount) + 31) & ~31) >> 3)
			 * m_Hdr->biHeight;
	}

	m_iPaletteSize = m_Hdr->biClrUsed * sizeof(RGBQUAD);
	MemorySize = m_Hdr->biSize + m_iPaletteSize + m_Hdr->biSizeImage;
	m_pDIB = realloc(m_pDIB, MemorySize);
	if (!m_pDIB)
		return false;

	m_Hdr = (BITMAPINFOHEADER *) m_pDIB;
	m_Data = ((byte *)m_Hdr) + sizeof(BITMAPINFOHEADER) + m_iPaletteSize;

	// read palette
	fread(((char *)m_Hdr) + m_Hdr->biSize, m_iPaletteSize, 1, fp);

	fseek(fp, bitmapHdr.bfOffBits, SEEK_SET);

	if (progress_callback != NULL)
	{
		char *target = ((char *)m_Hdr) + m_Hdr->biSize + m_iPaletteSize;
		// Read it a row at a time;
		int row_length = m_Hdr->biSizeImage / m_Hdr->biHeight;
		for (int r = 0; r < m_Hdr->biHeight; r++)
		{
			fread(target + (row_length*r), row_length, 1, fp);
			progress_callback(r * 100 / m_Hdr->biHeight);
		}
	}
	else
	{
		// Read it in one big chunk
		fread(((char *)m_Hdr) + m_Hdr->biSize + m_iPaletteSize, m_Hdr->biSizeImage, 1, fp);
	}

	// loaded OK
	m_iWidth = m_Hdr->biWidth;
	m_iHeight = m_Hdr->biHeight;
	m_iBitCount = m_Hdr->biBitCount;
	m_iByteCount = m_iBitCount/8;
	m_iPaletteSize = sizeof(RGBQUAD) * m_Hdr->biClrUsed;

	_ComputeByteWidth();
	fclose(fp);
	return true;

ErrExit:
	fclose(fp);
	return false;
}

/**
 * Write a MSWindows-style .bmp file from the DIB.
 */
bool vtDIB::WriteBMP(const char *fname)
{
	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp) return false;

	// write file header
	BITMAPFILEHEADER	bitmapHdr;
	bitmapHdr.bfType = 0x4d42;
	bitmapHdr.bfReserved1 = 0;
	bitmapHdr.bfReserved2 = 0;
	bitmapHdr.bfOffBits = 14 + 40 + m_iPaletteSize;		/* Header and colormap */
	bitmapHdr.bfSize = bitmapHdr.bfOffBits + m_iByteWidth * m_iHeight;

	bitmapHdr.bfType	= SwapBytes( (short)bitmapHdr.bfType  , BO_CPU, BO_LE );
	bitmapHdr.bfSize	= SwapBytes( (long)bitmapHdr.bfSize   , BO_CPU, BO_LE );
	bitmapHdr.bfOffBits = SwapBytes( (long)bitmapHdr.bfOffBits, BO_CPU, BO_LE );

	fwrite(&bitmapHdr, 14, 1, fp);

	// write bitmap header
	BITMAPINFOHEADER t_Hdr = *m_Hdr;

	t_Hdr.biSize		  = SwapBytes( (long )t_Hdr.biSize		 ,BO_CPU, BO_LE );
	t_Hdr.biWidth		 = SwapBytes( (long )t_Hdr.biWidth		,BO_CPU, BO_LE );
	t_Hdr.biHeight		= SwapBytes( (long )t_Hdr.biHeight	   ,BO_CPU, BO_LE );
	t_Hdr.biPlanes		= SwapBytes( (short)t_Hdr.biPlanes	   ,BO_CPU, BO_LE );
	t_Hdr.biBitCount	  = SwapBytes( (short)t_Hdr.biBitCount	 ,BO_CPU, BO_LE );
	t_Hdr.biCompression   = SwapBytes( (long )t_Hdr.biCompression  ,BO_CPU, BO_LE );
	t_Hdr.biSizeImage	 = SwapBytes( (long )t_Hdr.biSizeImage	,BO_CPU, BO_LE );
	t_Hdr.biXPelsPerMeter = SwapBytes( (long )t_Hdr.biXPelsPerMeter,BO_CPU, BO_LE );
	t_Hdr.biYPelsPerMeter = SwapBytes( (long )t_Hdr.biYPelsPerMeter,BO_CPU, BO_LE );
	t_Hdr.biClrUsed	   = SwapBytes( (long )t_Hdr.biClrUsed	  ,BO_CPU, BO_LE );
	t_Hdr.biClrImportant  = SwapBytes( (long )t_Hdr.biClrImportant ,BO_CPU, BO_LE );

	fwrite(&t_Hdr, 40, 1, fp);

	// write palette
	if (m_iPaletteSize)
	{
		RGBQUAD *rgb=(RGBQUAD*)((char*)m_Hdr + sizeof(BITMAPINFOHEADER));
		fwrite(rgb, m_iPaletteSize, 1, fp);
	}

	// write data
	fwrite(m_Data, m_Hdr->biSizeImage, 1, fp);

	// done
	fclose(fp);
	return true;
}


/**
 * Read a JPEG file. A DIB of the necessary size and depth is allocated.
 */
bool vtDIB::ReadJPEG(const char *fname, bool progress_callback(int))
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * input_file;
	JDIMENSION num_scanlines;

	/* Initialize the JPEG decompression object with default error handling. */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	input_file = vtFileOpen(fname, "rb");
	if (input_file == NULL)
		return false;

	/* Specify data source for decompression */
	jpeg_stdio_src(&cinfo, input_file);

	/* Read file header, set default decompression parameters */
	jpeg_read_header(&cinfo, TRUE);

	int bitdepth;
	if (cinfo.num_components == 1)
		bitdepth = 8;
	else
		bitdepth = 24;
	if (!Create(cinfo.image_width, cinfo.image_height, bitdepth, bitdepth == 8))
		return false;

	/* Start decompressor */
	jpeg_start_decompress(&cinfo);

	int buffer_height = 1;
	int row_stride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	unsigned int col;

	/* Process data */
	while (cinfo.output_scanline < cinfo.output_height)
	{
		if (progress_callback != NULL)
			progress_callback(cinfo.output_scanline * 100 / cinfo.output_height);

		JSAMPROW inptr = buffer[0];
		byte *adr = ((byte *)m_Data) + (m_iHeight-1-cinfo.output_scanline)*m_iByteWidth;

		num_scanlines = jpeg_read_scanlines(&cinfo, buffer,
						buffer_height);

		/* Transfer data.  Note destination values must be in BGR order
		 * (even though Microsoft's own documents say the opposite).
		 */
		if (bitdepth == 8)
		{
			for (col = 0; col < cinfo.output_width; col++)
				*adr++ = *inptr++;
		}
		else
		{
			for (col = 0; col < cinfo.output_width; col++)
			{
				adr[2] = *inptr++;	/* can omit GETJSAMPLE() safely */
				adr[1] = *inptr++;
				adr[0] = *inptr++;
				adr += 3;
			}
		}
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	/* Close files, if we opened them */
	if (input_file != stdin)
		fclose(input_file);

	return true;
}

/**
 * Write a JPEG file.
 *
 * \param fname		The output filename.
 * \param quality	JPEG quality in the range of 0..100.
 * \param progress_callback	If supplied, this will be called back with progress
 *		indication in the range of 1 to 100.
 *
 * \return True if successful.
 */
bool vtDIB::WriteJPEG(const char *fname, int quality, bool progress_callback(int))
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outfile;

	/* Initialize the JPEG decompression object with default error handling. */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	outfile = vtFileOpen(fname, "wb");
	if (outfile == NULL)
		return false;

	/* Specify data source for decompression */
	jpeg_stdio_dest(&cinfo, outfile);

	// set parameters for compression
	cinfo.image_width = m_iWidth;	/* image width and height, in pixels */
	cinfo.image_height = m_iHeight;
	cinfo.input_components = m_iByteCount;	/* # of color components per pixel */
	if (m_iByteCount == 1)
		cinfo.in_color_space = JCS_GRAYSCALE;
	else
		cinfo.in_color_space = JCS_RGB;	/* colorspace of input image */

	// Now use the library's routine to set default compression parameters.
	jpeg_set_defaults(&cinfo);

	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Start compressor */
	jpeg_start_compress(&cinfo, TRUE);

	int row_stride = cinfo.image_width * cinfo.input_components;

	unsigned int col;
	JSAMPROW row_buffer = new JSAMPLE[row_stride];

	/* Process data */
	while (cinfo.next_scanline < cinfo.image_height)
	{
		if (progress_callback != NULL)
			progress_callback(cinfo.next_scanline * 100 / cinfo.image_height);

		// Transfer data.  Note source values are in BGR order.
		JSAMPROW outptr = row_buffer;
		JSAMPROW adr = ((JSAMPROW)m_Data) + (m_iHeight-1-cinfo.next_scanline)*m_iByteWidth;
		if (m_iBitCount == 8)
		{
			for (col = 0; col < m_iWidth; col++)
				*outptr++ = *adr++;
		}
		else
		{
			for (col = 0; col < m_iWidth; col++)
			{
				*outptr++ = adr[2];
				*outptr++ = adr[1];
				*outptr++ = adr[0];
				adr += 3;
			}
		}
		jpeg_write_scanlines(&cinfo, &row_buffer, 1);
	}

	delete row_buffer;

	jpeg_finish_compress(&cinfo);

	/* After finish_compress, we can close the output file. */
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);

	return true;
}

/**
 * Read a PNG file. A DIB of the necessary size and depth is allocated.
 *
 * \return True if successful.
 */
bool vtDIB::ReadPNG(const char *fname, bool progress_callback(int))
{
	FILE *fp = NULL;

	unsigned char header[8];
	png_structp png;
	png_infop   info;
	png_infop   endinfo;
	png_bytep  *row_p;

	png_uint_32 width, height;
	int depth, color;

	png_uint_32 i;
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
	{
		// We compiled against the headers of one version of libpng, but
		// linked against the libraries from another version.  If you get
		// this, fix the paths in your development environment.
		return false;
	}
	info = png_create_info_struct(png);
	endinfo = png_create_info_struct(png);

	fp = vtFileOpen(fname, "rb");
	if (fp && fread(header, 1, 8, fp) && png_check_sig(header, 8))
		png_init_io(png, fp);
	else
	{
		png_destroy_read_struct(&png, &info, &endinfo);
		return false;
	}
	png_set_sig_bytes(png, 8);

	png_read_info(png, info);
	png_get_IHDR(png, info, &width, &height, &depth, &color, NULL, NULL, NULL);

	if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	// Don't strip alpha
//	if (color&PNG_COLOR_MASK_ALPHA)
//	{
//		png_set_strip_alpha(png);
//		color &= ~PNG_COLOR_MASK_ALPHA;
//	}

	// Always expand paletted images
//	if (!(PalettedTextures && mipmap >= 0 && trans == PNG_SOLID))
		if (color == PNG_COLOR_TYPE_PALETTE)
			png_set_expand(png);

	/*--GAMMA--*/
//	checkForGammaEnv();
	double screenGamma = 2.2 / 1.0;
#if 0
	// Getting the gamma from the PNG file is disabled here, since
	// PhotoShop writes bizarre gamma values like .227 (PhotoShop 5.0)
	// or .45 (newer versions)
	double	fileGamma;
	if (png_get_gAMA(png, info, &fileGamma))
		png_set_gamma(png, screenGamma, fileGamma);
	else
#endif
		png_set_gamma(png, screenGamma, 1.0/2.2);

	png_read_update_info(png, info);

	unsigned char *m_pPngData = (png_bytep) malloc(png_get_rowbytes(png, info)*height);
	row_p = (png_bytep *) malloc(sizeof(png_bytep)*height);

	bool StandardOrientation = true;
	for (i = 0; i < height; i++) {
		if (StandardOrientation)
			row_p[height - 1 - i] = &m_pPngData[png_get_rowbytes(png, info)*i];
		else
			row_p[i] = &m_pPngData[png_get_rowbytes(png, info)*i];
	}

	png_read_image(png, row_p);
	free(row_p);

	int iBitCount;
	switch (color)
	{
		case PNG_COLOR_TYPE_GRAY:
		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_PALETTE:
			iBitCount = 24;
			break;

		case PNG_COLOR_TYPE_GRAY_ALPHA:
		case PNG_COLOR_TYPE_RGB_ALPHA:
			iBitCount = 32;
			break;

		default:
			return false;
	}

	Create(width, height, iBitCount, iBitCount == 8);

	int png_stride = png_get_rowbytes(png, info);
	unsigned int row, col;
	for (row = 0; row < height; row++)
	{
		if (progress_callback != NULL)
			progress_callback(row * 100 / height);

		byte *adr = ((byte *)m_Data) + row*m_iByteWidth;
		png_bytep inptr = m_pPngData + row*png_stride;
		if (iBitCount == 8)
		{
			for (col = 0; col < width; col++)
				*adr++ = *inptr++;
		}
		else if (iBitCount == 24)
		{
			for (col = 0; col < width; col++)
			{
				adr[2] = *inptr++;
				adr[1] = *inptr++;
				adr[0] = *inptr++;
				adr += 3;
			}
		}
		else if (iBitCount == 32)
		{
			for (col = 0; col < width; col++)
			{
				adr[2] = *inptr++;
				adr[1] = *inptr++;
				adr[0] = *inptr++;
				adr[3] = *inptr++;
				adr += 4;
			}
		}
	}

	png_read_end(png, endinfo);
	png_destroy_read_struct(&png, &info, &endinfo);
	free(m_pPngData);
	fclose(fp);
	return true;
}


/**
 * Write a PNG file.
 *
 * \return True if successful.
 */
bool vtDIB::WritePNG(const char *fname)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp palette = NULL;

	/* open the file */
	fp = vtFileOpen(fname, "wb");
	if (fp == NULL)
		return false;

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also check that
	* the library version is compatible with the one used at compile time,
	* in case we are using dynamically linked libraries.  REQUIRED.
	*/
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(fp);
		return false;
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_write_struct(&png_ptr,  png_infopp_NULL);
		return false;
	}

	/* Set error handling.  REQUIRED if you aren't supplying your own
	* error handling functions in the png_create_write_struct() call.
	*/
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem reading the file */
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}

	/* set up the output control if you are using standard C streams */
	png_init_io(png_ptr, fp);

	/* This is the hard way */

	/* Set the image information here.  Width and height are up to 2^31,
	* bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
	* the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
	* PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
	* or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
	* PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
	* currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
	*/
	int color_type=0;	// set to 0 to avoid compiler warning
	int png_bit_depth;
	if (m_iBitCount <= 8)
	{
		color_type = PNG_COLOR_TYPE_PALETTE;
		png_bit_depth = m_iBitCount;
	}
	if (m_iBitCount == 24)
	{
		color_type = PNG_COLOR_TYPE_RGB;
		png_bit_depth = 8;
	}
	if (m_iBitCount == 32)
	{
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		png_bit_depth = 8;
	}

	png_set_IHDR(png_ptr, info_ptr, m_iWidth, m_iHeight, png_bit_depth, color_type,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	if (m_iBitCount <= 8)
	{
		/* set the palette if there is one.  REQUIRED for indexed-color images */
		palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH
				 * sizeof (png_color));
		RGBQUAD *rgb=(RGBQUAD*)((char*)m_Hdr + sizeof(BITMAPINFOHEADER));
		int p;
		for (p = 0; p < 256; p++)
		{
			palette[p].red = rgb[p].rgbRed;
			palette[p].green = rgb[p].rgbGreen;
			palette[p].red = rgb[p].rgbBlue;
		}
		png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);
	}
	/* You must not free palette here, because png_set_PLTE only makes a link to
	  the palette that you malloced.  Wait until you are about to destroy
	  the png structure. */

	/* Optional gamma chunk is strongly suggested if you have any guess
	* as to the correct gamma of the image.
	*/
//	png_set_gAMA(png_ptr, info_ptr, gamma);

	/* other optional chunks like cHRM, bKGD, tRNS, tIME, oFFs, pHYs, */
	/* note that if sRGB is present the gAMA and cHRM chunks must be ignored
	 * on read and must be written in accordance with the sRGB profile */

	/* Write the file header information.  REQUIRED */
	png_write_info(png_ptr, info_ptr);

	/* The easiest way to write the image (you may have a different memory
	 * layout, however, so choose what fits your needs best).  You need to
	 * use the first method if you aren't handling interlacing yourself.
	 */
	png_uint_32 k, height = m_iHeight, width = m_iWidth;
	png_byte *image = (png_byte *)malloc(height * width * m_iByteCount);
	png_bytep *row_pointers = (png_bytep *)malloc(height * sizeof(png_bytep *));
	for (k = 0; k < height; k++)
		row_pointers[k] = image + k*width*m_iByteCount;

	unsigned int row, col;
	for (row = 0; row < height; row++)
	{
		byte *adr = ((byte *)m_Data) + row*m_iByteWidth;
		png_bytep pngptr = row_pointers[height-1-row];
		if (m_iBitCount == 8)
		{
			for (col = 0; col < width; col++)
				*pngptr++ = *adr++;
		}
		else if (m_iBitCount == 24)
		{
			for (col = 0; col < width; col++)
			{
				*pngptr++ = adr[2];
				*pngptr++ = adr[1];
				*pngptr++ = adr[0];
				adr += 3;
			}
		}
		else if (m_iBitCount == 32)
		{
			for (col = 0; col < width; col++)
			{
				*pngptr++ = adr[2];
				*pngptr++ = adr[1];
				*pngptr++ = adr[0];
				*pngptr++ = adr[3];
				adr += 4;
			}
		}
	}

	/* write out the entire image data in one call */
	png_write_image(png_ptr, row_pointers);

   /* You can write optional chunks like tEXt, zTXt, and tIME at the end
	* as well.  Shouldn't be necessary in 1.1.0 and up as all the public
	* chunks are supported and you can use png_set_unknown_chunks() to
	* register unknown chunks into the info structure to be written out.
	*/

	/* It is REQUIRED to call this to finish writing the rest of the file */
	png_write_end(png_ptr, info_ptr);

	/* If you png_malloced a palette, free it here (don't free info_ptr->palette,
	  as recommended in versions 1.0.5m and earlier of this example; if
	  libpng mallocs info_ptr->palette, libpng will free it).  If you
	  allocated it with malloc() instead of png_malloc(), use free() instead
	  of png_free(). */
	if (palette)
	{
		png_free(png_ptr, palette);
		palette=NULL;
	}

	free(image);
	free(row_pointers);

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* close the file */
	fclose(fp);

	/* that's it */
	return true;
}

/**
 * Write a TIFF file.  If extents and projection are support, a GeoTIFF file
 * will be written.
 *
 * \param fname		The output filename.
 * \param area		The extents if the image.  Can be NULL if the image is
 *		not georeferenced.
 * \param proj		The CRS if the image.  Can be NULL if the image is
 *		not georeferenced.
 * \param progress_callback	If supplied, this will be called back with progress
 *		indication in the range of 1 to 100.
 *
 * \return True if successful.
 */
bool vtDIB::WriteTIF(const char *fname, const DRECT *area,
					 const vtProjection *proj, bool progress_callback(int))
{
	g_GDALWrapper.RequestGDALFormats();

	// Save with GDAL to GeoTIFF
	GDALDriverManager *pManager = GetGDALDriverManager();
	if (!pManager)
		return false;

	GDALDriver *pDriver = pManager->GetDriverByName("GTiff");
	if (!pDriver)
		return false;

	// GDAL doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(fname);

	char **papszParmList = NULL;
	GDALDataset *pDataset;
	if (m_iBitCount == 8)
		pDataset = pDriver->Create(fname_local, m_iWidth, m_iHeight,
			1, GDT_Byte, papszParmList );
	else
		pDataset = pDriver->Create(fname_local, m_iWidth, m_iHeight,
			3, GDT_Byte, papszParmList );
	if (!pDataset)
		return false;

	if (area != NULL)
	{
		DPoint2 spacing(area->Width() / m_iWidth, area->Height() / m_iHeight);
		double adfGeoTransform[6] = { area->left, spacing.x, 0,
									area->top, 0, -spacing.y };
		pDataset->SetGeoTransform(adfGeoTransform);
	}
	if (proj != NULL)
	{
		char *pszSRS_WKT = NULL;
		proj->exportToWkt( &pszSRS_WKT );
		pDataset->SetProjection(pszSRS_WKT);
		CPLFree( pszSRS_WKT );
	}

	GByte *raster = new GByte[m_iWidth*m_iHeight];

	unsigned int x, y;
	GDALRasterBand *pBand;

	if (m_iBitCount == 8)
	{
		pBand = pDataset->GetRasterBand(1);
		for (x = 0; x < m_iWidth; x++)
		{
			for (y = 0; y < m_iHeight; y++)
				raster[y*m_iWidth + x] = GetPixel8(x, y);
		}
		pBand->RasterIO( GF_Write, 0, 0, m_iWidth, m_iHeight,
			raster, m_iWidth, m_iHeight, GDT_Byte, 0, 0 );
	}
	else
	{
		RGBi rgb;
		int i;
		for (i = 1; i <= 3; i++)
		{
			pBand = pDataset->GetRasterBand(i);

			for (x = 0; x < m_iWidth; x++)
			{
				if (progress_callback != NULL)
					progress_callback((i-1)*33 + (x * 33 / m_iWidth));

				for (y = 0; y < m_iHeight; y++)
				{
					GetPixel24(x, y, rgb);
					if (i == 1) raster[y*m_iWidth + x] = (GByte) rgb.r;
					if (i == 2) raster[y*m_iWidth + x] = (GByte) rgb.g;
					if (i == 3) raster[y*m_iWidth + x] = (GByte) rgb.b;
				}
			}
			pBand->RasterIO( GF_Write, 0, 0, m_iWidth, m_iHeight,
				raster, m_iWidth, m_iHeight, GDT_Byte, 0, 0 );
		}
	}

	delete raster;
	GDALClose(pDataset);

	return true;
}

void vtDIB::_ComputeByteWidth()
{
	// compute the width in bytes of each scanline (with DIB padding)
	m_iByteWidth = (((m_iWidth)*(m_iBitCount) + 31) / 32 * 4);
}

/**
 * Pass true to indicate that the DIB should not free its internal memory
 * when the object is deleted.
 */
void vtDIB::LeaveInternalDIB(bool bLeaveIt)
{
	m_bLeaveIt = bLeaveIt;
}

/**
 * Get a 24-bit RGB value from a 24-bit bitmap.
 *
 * \return R,G,B as the three lowest bytes in an unsigned int.
 */
unsigned int vtDIB::GetPixel24(int x, int y) const
{
	register byte* adr;

	// note: Most processors don't support unaligned int/float reads, and on
	//	   those that do, it's slower than aligned reads.
	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + (x*m_iByteCount);
	return (*(byte *)(adr+0)) << 16 |
		   (*(byte *)(adr+1)) <<  8 |
		   (*(byte *)(adr+2));
}

/**
 * Get a 24-bit RGB value from a 24-bit bitmap, place it in the rgb parameter.
 */
void vtDIB::GetPixel24(int x, int y, RGBi &rgb) const
{
	if (m_iBitCount == 8)
	{
		GetPixel24From8bit(x, y, rgb);
		return;
	}

	register byte* adr;

	// note: Most processors don't support unaligned int/float reads, and on
	//	   those that do, it's slower than aligned reads.
	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + (x*m_iByteCount);
	rgb.b = *((byte *)(adr+0));
	rgb.g = *((byte *)(adr+1));
	rgb.r = *((byte *)(adr+2));
}

void vtDIB::GetPixel24From8bit(int x, int y, RGBi &rgb) const
{
	register byte* adr;
	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x;
	unsigned char val = *adr;

	RGBQUAD *palette=(RGBQUAD*)((char*)m_Hdr + sizeof(BITMAPINFOHEADER));
	rgb.b = palette[val].rgbBlue;
	rgb.g = palette[val].rgbGreen;
	rgb.r = palette[val].rgbRed;
}

void vtDIB::GetPixel32(int x, int y, RGBAi &rgba) const
{
	register byte* adr;

	// note: Most processors don't support unaligned int/float reads, and on
	//	   those that do, it's slower than aligned reads.
	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + (x*m_iByteCount);
	rgba.b = *((byte *)(adr+0));
	rgba.g = *((byte *)(adr+1));
	rgba.r = *((byte *)(adr+2));
	rgba.a = *((byte *)(adr+3));
}


/**
 * Set a 24-bit RGB value in a 24-bit bitmap.
 */
void vtDIB::SetPixel24(int x, int y, dword color)
{
	register byte* adr;

	// note: Most processors don't support unaligned int/float writes, and on
	//	   those that do, it's slower than unaligned writes.
	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + (x*m_iByteCount);
	*((byte *)(adr+0)) = (unsigned char) (color >> 16);
	*((byte *)(adr+1)) = (unsigned char) (color >>  8);
	*((byte *)(adr+2)) = (unsigned char) color;
}

void vtDIB::SetPixel24(int x, int y, const RGBi &rgb)
{
	register byte* adr;

	// note: Most processors don't support unaligned int/float writes, and on
	//	   those that do, it's slower than unaligned writes.
	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + (x*m_iByteCount);
	*((byte *)(adr+0)) = (unsigned char) rgb.b;
	*((byte *)(adr+1)) = (unsigned char) rgb.g;
	*((byte *)(adr+2)) = (unsigned char) rgb.r;
}

void vtDIB::SetPixel32(int x, int y, const RGBAi &rgba)
{
	register byte* adr;

	// note: Most processors don't support unaligned int/float writes, and on
	//	   those that do, it's slower than unaligned writes.
	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + (x*m_iByteCount);
	*((byte *)(adr+0)) = (unsigned char) rgba.b;
	*((byte *)(adr+1)) = (unsigned char) rgba.g;
	*((byte *)(adr+2)) = (unsigned char) rgba.r;
	if (m_iByteCount == 4)
		*((byte *)(adr+3)) = (unsigned char) rgba.a;
}

/**
 * Get a single byte from an 8-bit bitmap.
 */
unsigned char vtDIB::GetPixel8(int x, int y) const
{
	register byte* adr;

	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x;
	return *adr;
}


/**
 * Set a single byte in an 8-bit bitmap.
 */
void vtDIB::SetPixel8(int x, int y, unsigned char value)
{
	register byte* adr;

	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x;
	*adr = value;
}

/**
 * Get a single bit from a 1-bit bitmap.
 */
bool vtDIB::GetPixel1(int x, int y) const
{
	// untested - just my guess
	register int byte_offset = x/8;
	register int bit_offset = x%8;
	register byte* adr;

	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + byte_offset;
	return ((*adr) >> bit_offset) & 0x01;
}


/**
 * Set a single bit in a 1-bit bitmap.
 */
void vtDIB::SetPixel1(int x, int y, bool value)
{
	// untested - just my guess
	register int byte_offset = x/8;
	register int bit_offset = x%8;
	register byte* adr;

	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + byte_offset;
	if (value)
		(*adr) |= (1 << bit_offset);
	else
		(*adr) &= ~(1 << bit_offset);
}

/**
 * Sets the entire bitmap to a single color.
 */
void vtDIB::SetColor(const RGBi &rgb)
{
	unsigned int i, j;
	for (i = 0; i < m_iWidth; i++)
		for (j = 0; j < m_iHeight; j++)
		{
			SetPixel24(i, j, rgb);
		}
}

/**
 * Invert the bitmap colors.
 */
void vtDIB::Invert()
{
	unsigned int i, j;
	if (m_iBitCount == 8)
	{
		for (i = 0; i < m_iWidth; i++)
			for (j = 0; j < m_iHeight; j++)
			{
				SetPixel8(i, j, 8-GetPixel8(i, j));
			}
	}
	if (m_iBitCount == 24)
	{
		RGBi rgb;
		for (i = 0; i < m_iWidth; i++)
			for (j = 0; j < m_iHeight; j++)
			{
				GetPixel24(i, j, rgb);
				rgb.r = 255 - rgb.r;
				rgb.g = 255 - rgb.g;
				rgb.b = 255 - rgb.b;
				SetPixel24(i, j, rgb);
			}
	}
}

/**
 * Copy from this bitmap to another.  Currently, this is implemented with
 * a slow, completely unoptimized approach of one pixel at a time.
 */
void vtDIB::Blit(vtDIB &target, int x, int y)
{
	unsigned int depth = GetDepth();
	if (depth != target.GetDepth())
		return;
	int tw = target.GetWidth();
	int th = target.GetHeight();

	unsigned int i, j;
	for (i = 0; i < m_iWidth; i++)
	{
		for (j = 0; j < m_iHeight; j++)
		{
			int tx = i+x, ty = j+y;
			if (tx < 0 || tx > tw-1 || ty < 0 || ty > th-1)
				continue;

			if (depth == 8)
			{
				unsigned char value = GetPixel8(i, j);
				target.SetPixel8(tx, ty, value);
			}
			else if (depth == 24)
			{
				unsigned int value2 = GetPixel24(i, j);
				target.SetPixel24(tx, ty, value2);
			}
		}
	}
}

