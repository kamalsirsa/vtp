//
// vtDIB.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include <stdlib.h>
#include "vtDIB.h"
#include "ByteOrder.h"


//
// create new empty DIB wrapper
//
vtDIB::vtDIB()
{
	m_bLoadedSuccessfully = false;
	m_bLeaveIt = false;
	m_pDIB = NULL;
}


//
// load new DIB from file
//
vtDIB::vtDIB(const char *fname)
{
	// assume failure
	m_bLoadedSuccessfully = false;
	m_bLeaveIt = false;
	m_pDIB = NULL;

	if (ReadBMP(fname))
		m_bLoadedSuccessfully = true;
}

//
// create new DIB in memory
//
vtDIB::vtDIB(int xsize, int ysize, int bitdepth, bool create_palette)
{
	m_iWidth = xsize;
	m_iHeight = ysize;
	m_iBitCount = bitdepth;

	ComputeByteWidth();

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
}

vtDIB::vtDIB(void *pDIB)
{
	m_pDIB = pDIB;

	m_Hdr = (BITMAPINFOHEADER *) m_pDIB;
	m_Data = ((byte *)m_Hdr) + sizeof(BITMAPINFOHEADER) + m_iPaletteSize;

	m_iWidth = m_Hdr->biWidth;
	m_iHeight = m_Hdr->biHeight;
	m_iBitCount = m_Hdr->biBitCount;

	ComputeByteWidth();
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
	bitmapHdr.bfType    = SwapBytes( (short)bitmapHdr.bfType  , BO_LE, BO_CPU );
	bitmapHdr.bfSize    = SwapBytes( (long)bitmapHdr.bfSize   , BO_LE, BO_CPU );
	bitmapHdr.bfOffBits = SwapBytes( (long)bitmapHdr.bfOffBits, BO_LE, BO_CPU );

	if (bitmapHdr.bfType != 0x4d42)
		goto ErrExit;

	if (fread(m_Hdr, sizeof(BITMAPINFOHEADER), 1, fp) == 0)
		goto ErrExit;
	m_Hdr->biSize          = SwapBytes( (long )m_Hdr->biSize         ,BO_LE, BO_CPU );
	m_Hdr->biWidth         = SwapBytes( (long )m_Hdr->biWidth        ,BO_LE, BO_CPU );
	m_Hdr->biHeight        = SwapBytes( (long )m_Hdr->biHeight       ,BO_LE, BO_CPU );
	m_Hdr->biPlanes        = SwapBytes( (short)m_Hdr->biPlanes       ,BO_LE, BO_CPU );
	m_Hdr->biBitCount      = SwapBytes( (short)m_Hdr->biBitCount     ,BO_LE, BO_CPU );
	m_Hdr->biCompression   = SwapBytes( (long )m_Hdr->biCompression  ,BO_LE, BO_CPU );
	m_Hdr->biSizeImage     = SwapBytes( (long )m_Hdr->biSizeImage    ,BO_LE, BO_CPU );
	m_Hdr->biXPelsPerMeter = SwapBytes( (long )m_Hdr->biXPelsPerMeter,BO_LE, BO_CPU );
	m_Hdr->biYPelsPerMeter = SwapBytes( (long )m_Hdr->biYPelsPerMeter,BO_LE, BO_CPU );
	m_Hdr->biClrUsed       = SwapBytes( (long )m_Hdr->biClrUsed      ,BO_LE, BO_CPU );
	m_Hdr->biClrImportant  = SwapBytes( (long )m_Hdr->biClrImportant ,BO_LE, BO_CPU );

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

	ComputeByteWidth();
	fclose(fp);
	return true;

ErrExit:
	fclose(fp);
	return false;
}

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

	bitmapHdr.bfType    = SwapBytes( (short)bitmapHdr.bfType  , BO_CPU, BO_LE );
	bitmapHdr.bfSize    = SwapBytes( (long)bitmapHdr.bfSize   , BO_CPU, BO_LE );
	bitmapHdr.bfOffBits = SwapBytes( (long)bitmapHdr.bfOffBits, BO_CPU, BO_LE );

	fwrite(&bitmapHdr, 14, 1, fp);

	// write bitmap header
	BITMAPINFOHEADER t_Hdr = *m_Hdr;

	t_Hdr.biSize          = SwapBytes( (long )t_Hdr.biSize         ,BO_CPU, BO_LE );
	t_Hdr.biWidth         = SwapBytes( (long )t_Hdr.biWidth        ,BO_CPU, BO_LE );
	t_Hdr.biHeight        = SwapBytes( (long )t_Hdr.biHeight       ,BO_CPU, BO_LE );
	t_Hdr.biPlanes        = SwapBytes( (short)t_Hdr.biPlanes       ,BO_CPU, BO_LE );
	t_Hdr.biBitCount      = SwapBytes( (short)t_Hdr.biBitCount     ,BO_CPU, BO_LE );
	t_Hdr.biCompression   = SwapBytes( (long )t_Hdr.biCompression  ,BO_CPU, BO_LE );
	t_Hdr.biSizeImage     = SwapBytes( (long )t_Hdr.biSizeImage    ,BO_CPU, BO_LE );
	t_Hdr.biXPelsPerMeter = SwapBytes( (long )t_Hdr.biXPelsPerMeter,BO_CPU, BO_LE );
	t_Hdr.biYPelsPerMeter = SwapBytes( (long )t_Hdr.biYPelsPerMeter,BO_CPU, BO_LE );
	t_Hdr.biClrUsed       = SwapBytes( (long )t_Hdr.biClrUsed      ,BO_CPU, BO_LE );
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

void vtDIB::ComputeByteWidth()
{
	m_iByteWidth = (((m_iWidth)*(m_iBitCount) + 31) / 32 * 4);
}

void vtDIB::LeaveInternalDIB(bool bLeaveIt)
{
	m_bLeaveIt = bLeaveIt;
}

//
// Get/Set a 24-bit RGB value in a 24-bit bitmap
//
dword vtDIB::GetPixel24(int x, int y)
{
	register byte* adr;

	// note: Most processors don't support unaligned int/float reads, and on
	//       those that do, it's slower than aligned reads.
	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x+x+x;
	return (*(byte *)(adr+0)) << 16 |
	       (*(byte *)(adr+1)) <<  8 |
	       (*(byte *)(adr+2));
}


void vtDIB::SetPixel24(int x, int y, dword color)
{
	register byte* adr;

	// note: Most processors don't support unaligned int/float writes, and on
	//       those that do, it's slower than unaligned writes.
	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x+x+x;
	*((byte *)(adr+0)) = color >> 16;
	*((byte *)(adr+1)) = color >>  8;
	*((byte *)(adr+2)) = color;
}

//
// Get/Set a single byte in an 8-bit bitmap
//
byte vtDIB::GetPixel8(int x, int y)
{
	register byte* adr;

	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x;
	return *adr;
}


void vtDIB::SetPixel8(int x, int y, byte value)
{
	register byte* adr;

	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x;
	*adr = value;
}

//
// Get/Set a single bit in a 1-bit bitmap
//
bool vtDIB::GetPixel1(int x, int y)
{
	// untested - just my guess
	register int byte_offset = x/8;
	register int bit_offset = x%8;
	register byte* adr;

	adr = ((byte *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + byte_offset;
	return ((*adr) >> bit_offset) & 0x01;
}


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



