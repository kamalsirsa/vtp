//
// vtDIB.cpp
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include <stdlib.h>
#include "vtDIB.h"
#include "ByteOrder.h"

// Headers for JPEG support, which uses the library "libjpeg"
extern "C" {
#include "jinclude.h"
#include "jpeglib.h"
}

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

	_ComputeByteWidth();
}


vtDIB::~vtDIB()
{
	// Only free the encapsulated DIB if we're allowed to
	if (!m_bLeaveIt)
	{
		if (m_pDIB != NULL)
			free(m_pDIB);
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

	_ComputeByteWidth();

	int ImageSize = m_iByteWidth * m_iHeight;

	int PaletteColors = create_palette ? 256 : 0;
	m_iPaletteSize = sizeof(RGBQUAD) * PaletteColors;

	m_pDIB = malloc(sizeof(BITMAPINFOHEADER) + m_iPaletteSize + ImageSize);
	m_Hdr = (BITMAPINFOHEADER *) m_pDIB;
	m_Data = ((byte *)m_Hdr) + sizeof(BITMAPINFOHEADER) + m_iPaletteSize;

	m_Hdr->biSize = sizeof(BITMAPINFOHEADER);
	m_Hdr->biWidth = xsize;
	m_Hdr->biHeight = ysize;
	m_Hdr->biPlanes = 1;
	m_Hdr->biBitCount = bitdepth;
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


/**
 * Read a image file into the DIB.  This method will check to see if the
 * file is a BMP or JPEG and call the appropriate reader.
 */
bool vtDIB::Read(const char *fname)
{
	FILE *fp = fopen(fname, "rb");
	if (!fp)
		return false;
	unsigned char buf[2];
	if (fread(buf, 2, 1, fp) != 1)
		return false;
	fclose(fp);
	if (buf[0] == 0x42 && buf[1] == 0x4d)
		return ReadBMP(fname);
	else
		return ReadJPEG(fname);
}


/**
 * Read a MSWindows-style .bmp file into the DIB.
 */
bool vtDIB::ReadBMP(const char *fname)
{
	BITMAPFILEHEADER	bitmapHdr;
	int MemorySize;

	FILE *fp = fopen(fname, "rb");
	if (!fp)
		return false;

	// allocate enough room for the header
	m_pDIB = malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
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
	m_Hdr->biSize		  	= SwapBytes( (long )m_Hdr->biSize		,BO_LE, BO_CPU );
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

	m_Hdr = (BITMAPINFOHEADER *) m_pDIB;
	m_Data = ((byte *)m_Hdr) + sizeof(BITMAPINFOHEADER) + m_iPaletteSize;

	// read palette
	fread(((char *)m_Hdr) + m_Hdr->biSize, m_iPaletteSize, 1, fp);

	fseek(fp, bitmapHdr.bfOffBits, SEEK_SET);

	fread(((char *)m_Hdr) + m_Hdr->biSize + m_iPaletteSize, m_Hdr->biSizeImage, 1, fp);

	// loaded OK
	m_iWidth = m_Hdr->biWidth;
	m_iHeight = m_Hdr->biHeight;
	m_iBitCount = m_Hdr->biBitCount;
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
	FILE *fp = fopen(fname, "wb");
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
bool vtDIB::ReadJPEG(const char *fname)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * input_file;
	JDIMENSION num_scanlines;

	/* Initialize the JPEG decompression object with default error handling. */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	input_file = fopen(fname, "rb");
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
	Create(cinfo.image_width, cinfo.image_height, bitdepth, bitdepth == 8);

	/* Start decompressor */
	jpeg_start_decompress(&cinfo);

	int buffer_height = 1;
	int row_stride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	int cur_output_row = 0;
	unsigned int col;

	/* Process data */
	while (cinfo.output_scanline < cinfo.output_height)
	{
		num_scanlines = jpeg_read_scanlines(&cinfo, buffer,
						buffer_height);

		/* Transfer data.  Note destination values must be in BGR order
		* (even though Microsoft's own documents say the opposite).
		*/
		JSAMPROW inptr = buffer[0];
		byte *adr = ((byte *)m_Data) + (m_iHeight-cur_output_row-1)*m_iByteWidth;

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
		cur_output_row++;
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	/* Close files, if we opened them */
	if (input_file != stdin)
		fclose(input_file);

	return true;
}


void vtDIB::_ComputeByteWidth()
{
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
 */
dword vtDIB::GetPixel24(int x, int y)
{
	register byte* adr;

	// note: Most processors don't support unaligned int/float reads, and on
	//	   those that do, it's slower than aligned reads.
	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x+x+x;
	return (*(byte *)(adr+0)) << 16 |
		   (*(byte *)(adr+1)) <<  8 |
		   (*(byte *)(adr+2));
}


/**
 * Set a 24-bit RGB value in a 24-bit bitmap.
 */
void vtDIB::SetPixel24(int x, int y, dword color)
{
	register byte* adr;

	// note: Most processors don't support unaligned int/float writes, and on
	//	   those that do, it's slower than unaligned writes.
	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x+x+x;
	*((byte *)(adr+0)) = (unsigned char) (color >> 16);
	*((byte *)(adr+1)) = (unsigned char) (color >>  8);
	*((byte *)(adr+2)) = (unsigned char) color;
}

/**
 * Get a single byte from an 8-bit bitmap.
 */
byte vtDIB::GetPixel8(int x, int y)
{
	register byte* adr;

	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x;
	return *adr;
}


/**
 * Set a single byte in an 8-bit bitmap.
 */
void vtDIB::SetPixel8(int x, int y, byte value)
{
	register byte* adr;

	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x;
	*adr = value;
}

/**
 * Get a single bit from a 1-bit bitmap.
 */
bool vtDIB::GetPixel1(int x, int y)
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



