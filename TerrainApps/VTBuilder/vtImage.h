//
// vtImage.h
//
// Copyright (c) 2002-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTIMAGE_H
#define VTIMAGE_H

#include "wx/image.h"
#include "TilingOptions.h"
#include "vtdata/Projections.h"

class vtBitmap;
class GDALDataset;
class GDALRasterBand;
class GDALColorTable;
class BuilderView;
class ImageGLCanvas;

// The following mechanism is for a small buffer, consisting of a small
//  number of scanlines, to cache the results of accessing large image
//  files out of memory (direct from disk).
struct Scanline
{
	RGBi *m_data;
	int m_y;
};
#define BUF_SCANLINES	4

//////////////////////////////////////////////////////////

class vtImage
{
public:
	vtImage();
	vtImage(const DRECT &area, int xsize, int ysize,
		const vtProjection &proj);
	virtual ~vtImage();

	bool GetExtent(DRECT &rect);
	void SetExtent(const DRECT &rect);
	void DrawToView(wxDC* pDC, vtScaledView *pView);
	bool ConvertProjection(vtImage *input, vtProjection &proj_new,
						   int iSampleN, bool progress_callback(int) = NULL);

	bool IsAllocated() const;
	DPoint2 GetSpacing() const;
	vtBitmap *GetBitmap() { return m_pBitmap; }

	void GetProjection(vtProjection &proj);
	vtProjection &GetAtProjection() { return m_proj; }
	void SetProjection(const vtProjection &proj);
	bool ReprojectExtents(const vtProjection &proj_new);

	void GetDimensions(int &xsize, int &ysize)
	{
		xsize = m_iXSize;
		ysize = m_iYSize;
	}
	IPoint2 GetDimensions()
	{
		return IPoint2(m_iXSize, m_iYSize);
	}
	bool GetColorSolid(const DPoint2 &p, RGBi &rgb);
	bool GetMultiSample(const DPoint2 &p, const DLine2 &offsets, RGBi &rgb);
	void GetRGB(int x, int y, RGBi &rgb);
	void SetRGB(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	void SetRGB(int x, int y, const RGBi &rgb);
	void ReplaceColor(const RGBi &rgb1, const RGBi &rgb2);

	// File IO
	bool ReadPPM(const char *fname, bool progress_callback(int) = NULL);
	bool WritePPM(const char *fname);
	bool SaveToFile(const char *fname) const;
	bool ReadPNGFromMemory(unsigned char *buf, int len);
	bool LoadFromGDAL(const char *fname);

	bool ReadFeaturesFromTerraserver(const DRECT &area, int iTheme,
		int iMetersPerPixel, int iUTMZone, const char *filename);
	bool WriteGridOfTilePyramids(TilingOptions &opts, BuilderView *pView);
	bool WriteTile(const TilingOptions &opts, BuilderView *pView, vtString &dirname,
		DRECT &tile_area, DPoint2 &tile_dim, int col, int row, int lod);

	// global switch
	static bool bTreatBlackAsTransparent;

protected:
	void SetDefaults();
	void CleanupGDALUsage();

	vtProjection	m_proj;

	DRECT   m_Extents;
	int		m_iXSize;
	int		m_iYSize;

	vtBitmap	*m_pBitmap;

	// used when reading from a file with GDAL
	int iRasterCount;
	unsigned char *pScanline;
	unsigned char *pRedline;
	unsigned char *pGreenline;
	unsigned char *pBlueline;
	GDALRasterBand *pBand;
	GDALRasterBand *pRed;
	GDALRasterBand *pGreen;
	GDALRasterBand *pBlue;
	int nxBlocks, nyBlocks;
	int xBlockSize, yBlockSize;
	GDALColorTable *pTable;
	GDALDataset *pDataset;

	Scanline m_row[BUF_SCANLINES];
	int m_use_next;

	void ReadScanline(int y, int bufrow);
	RGBi *GetScanlineFromBuffer(int y);

	// Used during writing of tilesets
	int m_iTotal, m_iCompleted;
	ImageGLCanvas *m_pCanvas;
};

// Helper
int GetBitDepthUsingGDAL(const char *fname);
void MakeSampleOffsets(const DPoint2 cellsize, unsigned int N, DLine2 &offsets);

#endif	// VTIMAGE_H
