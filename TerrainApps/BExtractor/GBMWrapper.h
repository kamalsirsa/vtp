//
// GBMWrapper.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef GBMWRAPPERH
#define GBMWRAPPERH

extern "C"
{
#include "GBM/gbm.h"
}

#include "vtdata/vtDIB.h"

#ifndef O_BINARY
#define	O_BINARY	0
#endif

class CGBM {
public:
	CGBM();
	CGBM(const char *fname);
	CGBM(int xsize, int ysize, int bitdepth);
	~CGBM();

	// methods
	bool	HasData() { return (m_ptr != NULL); }
	void	GetPixel24(int x, int y, GBMRGB &rgb);
	unsigned int GetPixel24(int x, int y);		// rgb as a packed int
	byte	GetPixel8(int x, int y);
	void	SetPixel8(int x, int y, byte val);
	int		GetWidth() { return m_gbm.w; }
	int		GetHeight() { return m_gbm.h; }
	int		GetDepth() { return m_gbm.bpp; }
	BITMAPINFOHEADER *GetDIBHeader() { return m_bmi; }

	// data
	byte	*m_data;
	int		m_datasize;

protected:
	// methods
	void	ComputeStrideAndSize();
	void	AllocateMem();
	void	GenerateBMI();
	void	ConvertPalette();

	// data
	GBM		m_gbm;
	byte	*m_ptr;
	GBMRGB	m_gbmrgb[0x100];	// palette for 8-bit bitmaps
	int		m_stride;

	BITMAPINFOHEADER *m_bmi;
	RGBQUAD *m_dibrgb;
	int		m_dibsize;
};

#endif