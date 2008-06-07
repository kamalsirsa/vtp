//
// vtImage.h
//
// Copyright (c) 2002-2008 Virtual Terrain Project
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
	int m_overview;	// 0 is the base image, 1 2 3.. are the overviews
};
#define BUF_SCANLINES	4

class LineBufferGDAL
{
public:
	LineBufferGDAL();
	~LineBufferGDAL() { Cleanup(); }

	void Setup(GDALDataset *pDataset);
	void Cleanup();

	//void GetRGB(int x, int y, RGBi &rgb, double dRes = 0.0);
	void ReadScanline(int y, int bufrow, int overlay);
	RGBi *GetScanlineFromBuffer(int y, int overlay);
	void FindMaxBlockSize(GDALDataset *pDataset);

	int		m_iXSize;
	int		m_iYSize;

	int m_iRasterCount;
	unsigned char *m_pBlock;
	unsigned char *m_pRedBlock;
	unsigned char *m_pGreenBlock;
	unsigned char *m_pBlueBlock;
	GDALRasterBand *m_pBand;
	GDALRasterBand *m_pRed;
	GDALRasterBand *m_pGreen;
	GDALRasterBand *m_pBlue;
	GDALColorTable *m_pTable;
	int m_MaxBlockSize;

	// Overviews
	int m_iViewCount;

	Scanline m_row[BUF_SCANLINES];
	int m_use_next;
	int m_found_last;

	// Statistics
	int m_linereads;
	int m_blockreads;
};

class BitmapInfo
{
public:
	BitmapInfo() { m_pBitmap = NULL; m_bOverlay = false; }

	int number;				// 0, 1, 2..
	vtBitmap *m_pBitmap;	// non-NULL if in memory
	bool m_bOverlay;		// true if GDAL overlay exists on disk
	IPoint2 m_Size;			// size in pixels
	DPoint2 m_Spacing;		// spatial resolution in earth units/pixel
};

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
	void DrawToView(wxDC *pDC, vtScaledView *pView);
	bool ConvertProjection(vtImage *input, vtProjection &proj_new,
						   int iSampleN, bool progress_callback(int) = NULL);

	DPoint2 GetSpacing(int bitmap = 0) const;
	vtBitmap *GetBitmap() { return m_Bitmaps[0].m_pBitmap; }
	bool HasOverviews() { return m_linebuf.m_iViewCount > 0; }

	void GetProjection(vtProjection &proj);
	vtProjection &GetAtProjection() { return m_proj; }
	void SetProjection(const vtProjection &proj);
	bool ReprojectExtents(const vtProjection &proj_new);

	void GetDimensions(int &xsize, int &ysize)
	{
		xsize = m_Bitmaps[0].m_Size.x;
		ysize = m_Bitmaps[0].m_Size.y;
	}
	IPoint2 GetDimensions()
	{
		return m_Bitmaps[0].m_Size;
	}
	bool GetColorSolid(const DPoint2 &p, RGBi &rgb, double dRes = 0.0);
	bool GetMultiSample(const DPoint2 &p, const DLine2 &offsets, RGBi &rgb, double dRes = 0.0);
	void GetRGB(int x, int y, RGBi &rgb, double dRes = 0.0);
	void SetRGB(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	void SetRGB(int x, int y, const RGBi &rgb);
	void ReplaceColor(const RGBi &rgb1, const RGBi &rgb2);
	void SetupBitmapInfo(int iXSize, int iYSize);

	// File IO
	bool ReadPPM(const char *fname, bool progress_callback(int) = NULL);
	bool WritePPM(const char *fname);
	bool SaveToFile(const char *fname) const;
	bool ReadPNGFromMemory(unsigned char *buf, int len);
	bool LoadFromGDAL(const char *fname);
	bool CreateOverviews();

	bool ReadFeaturesFromTerraserver(const DRECT &area, int iTheme,
		int iMetersPerPixel, int iUTMZone, const char *filename);
	bool WriteGridOfTilePyramids(TilingOptions &opts, BuilderView *pView);
	bool WriteTile(const TilingOptions &opts, BuilderView *pView, vtString &dirname,
		DRECT &tile_area, DPoint2 &tile_dim, int col, int row, int lod);

	// global switch
	static bool bTreatBlackAsTransparent;

	// used when reading from a file with GDAL
	GDALDataset *m_pDataset;
	LineBufferGDAL m_linebuf;

	int NumBitmaps() { return m_Bitmaps.size(); }
	BitmapInfo &GetBitmapInfo(int i) { return m_Bitmaps[i]; }
	int NumBitmapsInMemory();

	void AllocMipMaps();
	void DrawMipMaps();
	void FreeMipMaps();

protected:
	void SetDefaults();
	void CleanupGDALUsage();

	vtProjection	m_proj;

	DRECT   m_Extents;

	std::vector<BitmapInfo> m_Bitmaps;

	// Used during writing of tilesets
	int m_iTotal, m_iCompleted;
	ImageGLCanvas *m_pCanvas;
};

// Helpers
int GetBitDepthUsingGDAL(const char *fname);
void MakeSampleOffsets(const DPoint2 cellsize, unsigned int N, DLine2 &offsets);
void SampleMipLevel(vtBitmap *bigger, vtBitmap *smaller);

#endif	// VTIMAGE_H
