//
// ImageLayer.h
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef IMAGELAYER_H
#define IMAGELAYER_H

#include "wx/image.h"
#include "Layer.h"
#include "vtBitmap.h"

class GDALDataset;
class GDALRasterBand;
class GDALColorTable;

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

class vtImageLayer : public vtLayer 
{
public:
	vtImageLayer();
	vtImageLayer(const DRECT &area, int xsize, int ysize,
		const vtProjection &proj);
	virtual ~vtImageLayer();

	// overrides for vtLayer methods
	bool GetExtent(DRECT &rect);
	bool GetAreaExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, class vtScaledView *pView);
	bool TransformCoords(vtProjection &proj);
	bool OnSave();
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetProjection(vtProjection &proj);
	void SetProjection(const vtProjection &proj);
	void Offset(const DPoint2 &delta);

	// optional overrides
	void GetPropertyText(wxString &str);

	DPoint2 GetSpacing();
	vtBitmap *GetBitmap() { return m_pBitmap; }

	void GetDimensions(int &xsize, int &ysize)
	{
		xsize = m_iXSize;
		ysize = m_iYSize;
	}
	bool GetFilteredColor(double x, double y, RGBi &rgb);
	bool SaveToFile(const char *fname);
	void SetRGB(int x, int y, unsigned char r, unsigned char g, unsigned char b);

	bool ReadFeaturesFromTerraserver(const DRECT &area, int iTheme,
		int iMetersPerPixel, int iUTMZone, const char *filename);

protected:
	void SetDefaults();
	bool LoadFromGDAL();
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
};

#endif
