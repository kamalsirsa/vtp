//
// GBMWrapper.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "GBMWrapper.h"

CGBM::CGBM()
{
	m_data = m_ptr = NULL;
}

CGBM::CGBM(const char *fname)
{
	char *opt_src;
	int	fd, ft_src;
	GBM_ERR	rc;
	int ok = 1;

	m_data = m_ptr = NULL;
	opt_src = "";
	gbm_init();
	if ( gbm_guess_filetype(fname, &ft_src) != GBM_ERR_OK )
		ok = 0;

	if ( ((fd = gbm_io_open(fname, O_RDONLY|O_BINARY)) == -1) & (ok == 1) )
		ok = 0;

	if ( ((rc = gbm_read_header(fname, fd, ft_src, &m_gbm, opt_src)) != GBM_ERR_OK) & (ok == 1) )
	{
		gbm_io_close(fd);
		ok = 0;
	}

	if (m_gbm.bpp == 8)
	{
		if ( ((rc = gbm_read_palette(fd, ft_src, &m_gbm, m_gbmrgb)) != GBM_ERR_OK) & (ok == 1) )
		{
			gbm_io_close(fd);
			ok = 0;
		}
	}

	if (ok == 1)
	{
		ComputeStrideAndSize();
		AllocateMem();
		ConvertPalette();
	}

	if (m_ptr == NULL)
	{
		gbm_io_close(fd);
		ok = 0;
	}

	if ( ((rc = gbm_read_data(fd, ft_src, &m_gbm, m_data)) != GBM_ERR_OK) & (ok == 1) )
	{
		free(m_data);
		m_ptr = NULL;
		m_data = NULL;
		gbm_io_close(fd);
		ok = 0;
	}

	gbm_io_close(fd);
	gbm_deinit();
}

CGBM::CGBM(int xsize, int ysize, int bitdepth)
{
	m_gbm.w = xsize;
	m_gbm.h = ysize;
	m_gbm.bpp = bitdepth;

	ComputeStrideAndSize();
	AllocateMem();

	if (bitdepth == 8)
	{
		// default palette: greyscale
		for (int i=0; i<256; i++)
			m_dibrgb[i].rgbBlue = m_dibrgb[i].rgbGreen = m_dibrgb[i].rgbRed =
				(unsigned char)i;
	}
}

CGBM::~CGBM()
{
	if (m_ptr)
		free(m_ptr);
}

byte CGBM::GetPixel8(int x, int y)
{
	y = m_gbm.h - 1 - y;	// bottom-up -> top-down
	return m_data[y * m_stride + x];
}

void CGBM::SetPixel8(int x, int y, byte val)
{
	y = m_gbm.h - 1 - y;	// bottom-up -> top-down
	m_data[y * m_stride + x] = val;
}

void CGBM::GetPixel24(int x, int y, GBMRGB &rgb)
{
	y = m_gbm.h - 1 - y;	// bottom-up -> top-down
	if (m_gbm.bpp == 8)
	{
		byte i = m_data[y * m_stride + x];
		rgb = m_gbmrgb[i];
	}
	else
	{
		rgb.b = m_data[y * m_stride + (x*3) + 0];
		rgb.g = m_data[y * m_stride + (x*3) + 1];
		rgb.r = m_data[y * m_stride + (x*3) + 2];
	}
}

unsigned int CGBM::GetPixel24(int x, int y)		// rgb as a packed int
{
	y = m_gbm.h - 1 - y;	// bottom-up -> top-down
	unsigned char r, g, b;
	if (m_gbm.bpp == 8)
	{
		byte i = m_data[y * m_stride + x];
		r = m_gbmrgb[i].r;
		g = m_gbmrgb[i].g;
		b = m_gbmrgb[i].b;
	}
	else
	{
		b = m_data[y * m_stride + (x*3) + 0];
		g = m_data[y * m_stride + (x*3) + 1];
		r = m_data[y * m_stride + (x*3) + 2];
	}
	return (b | (g<<8)) | (r<<16);
}

void CGBM::GenerateBMI()
{
#if WIN32
	m_bmi->biSize = sizeof(BITMAPINFOHEADER);
    m_bmi->biWidth = m_gbm.w;
    m_bmi->biHeight = m_gbm.h;
    m_bmi->biPlanes = 1;
	m_bmi->biBitCount = m_gbm.bpp;
	m_bmi->biCompression = BI_RGB;
	m_bmi->biSizeImage = m_datasize;
	m_bmi->biXPelsPerMeter = 1;
	m_bmi->biYPelsPerMeter = 1;
	m_bmi->biClrUsed = m_gbm.bpp == 8 ? 256 : 0;
	m_bmi->biClrImportant = 0;
#endif
}

void CGBM::ComputeStrideAndSize()
{
	m_stride = ( ((m_gbm.w * m_gbm.bpp + 31)/32) * 4 );
	m_datasize = (size_t) (m_stride * m_gbm.h);
#if WIN32
	m_dibsize = sizeof(BITMAPINFOHEADER) + m_datasize;
	if (m_gbm.bpp == 8)
		m_dibsize += 256 * sizeof(RGBQUAD);
#endif
}

void CGBM::AllocateMem()
{
#if WIN32
	m_ptr = (byte *)malloc(m_dibsize);
	m_bmi = (BITMAPINFOHEADER *)m_ptr;
	m_dibrgb = (RGBQUAD*)(m_ptr + sizeof(BITMAPINFOHEADER));
	m_data = m_ptr + sizeof(BITMAPINFOHEADER) + (m_gbm.bpp == 8 ? 256 * sizeof(RGBQUAD) : 0);
	GenerateBMI();
#else
	m_ptr = (byte *)malloc(m_datasize);
	m_data = m_ptr;
#endif
}

void CGBM::ConvertPalette()
{
#if WIN32
	if (GetDepth() != 8)
		return;
	for (int i=0; i<256; i++)
	{
		m_dibrgb[i].rgbBlue = m_gbmrgb[i].b;
		m_dibrgb[i].rgbGreen = m_gbmrgb[i].g;
		m_dibrgb[i].rgbRed = m_gbmrgb[i].r;
	}
#endif
}

