//
// vtImage.cpp
//
// Copyright (c) 2002-2010 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ImageLayer.h"
#include "vtdata/vtLog.h"

#include "vtui/Helper.h"
#include "vtui/ProjectionDlg.h"

#include "ogr_spatialref.h"
#include "gdal_priv.h"
#include "BuilderView.h"	// For grid marks
#include "vtImage.h"
#include "Helper.h"
#include "Builder.h"
#include "vtBitmap.h"
#include "minidata/LocalDatabuf.h"
#include "Options.h"

#include "ExtentDlg.h"

// static global
bool vtImage::bTreatBlackAsTransparent = false;


///////////////////////////////////////////////////////////////////////

LineBufferGDAL::LineBufferGDAL()
{
	m_pBlock = NULL;
	m_pRedBlock = NULL;
	m_pGreenBlock = NULL;
	m_pBlueBlock = NULL;
	m_pBand = NULL;
	m_pRed = NULL;
	m_pGreen = NULL;
	m_pBlue = NULL;
	m_pTable = NULL;

	for (int i = 0; i < BUF_SCANLINES; i++)
		m_row[i].m_data = NULL;
}

void LineBufferGDAL::Setup(GDALDataset *pDataset)
{
	m_iXSize = pDataset->GetRasterXSize();
	m_iYSize = pDataset->GetRasterYSize();

	// Prepare scanline buffers
	for (int i = 0; i < BUF_SCANLINES; i++)
	{
		m_row[i].m_data = new RGBi[m_iXSize];
		m_row[i].m_y = -1;
	}
	m_use_next = 0;
	m_found_last = -1;
	m_linereads = 0;
	m_blockreads = 0;

	// Raster count should be 3 for colour images (assume RGB)
	m_iRasterCount = pDataset->GetRasterCount();

	vtString message;

	if (m_iRasterCount != 1 && m_iRasterCount != 3)
	{
		message.Format("Image has %d bands (not 1 or 3).", m_iRasterCount);
		throw (const char *)message;
	}

	FindMaxBlockSize(pDataset);

	if (m_iRasterCount == 1)
	{
		m_pBand = pDataset->GetRasterBand(1);
		m_iViewCount = m_pBand->GetOverviewCount()+1;

		// Check data type - it's either integer or float
		if (GDT_Byte != m_pBand->GetRasterDataType())
			throw "Raster is not of type byte.";
		GDALColorInterp ci = m_pBand->GetColorInterpretation();
		if (ci == GCI_PaletteIndex)
		{
			if (NULL == (m_pTable = m_pBand->GetColorTable()))
				throw "Couldn't get color table.";
		}
		else if (ci == GCI_GrayIndex)
		{
			// we will assume 0-255 is black to white
		}
		else
			throw "Unsupported color interpretation.";

		if (NULL == (m_pBlock = new uchar[m_MaxBlockSize]))
			throw "Couldnt allocate scan line.";
	}

	if (m_iRasterCount == 3)
	{
		int num_undefined = 0;
		for (int i = 1; i <= 3; i++)
		{
			GDALRasterBand *pBand = pDataset->GetRasterBand(i);
			// Check data type - it's either integer or float
			if (GDT_Byte != pBand->GetRasterDataType())
				throw "Three rasters, but not of type byte.";
			// I assume that the bands are in order RGB
			// I know "could do better"... but
			switch(pBand->GetColorInterpretation())
			{
			case GCI_RedBand:
				m_pRed = pBand;
				break;
			case GCI_GreenBand:
				m_pGreen = pBand;
				break;
			case GCI_BlueBand:
				m_pBlue = pBand;
				break;
			case GCI_Undefined:
				num_undefined++;
				break;
			}
		}
		if (num_undefined == 3)
		{
			// All three are undefined, assume they are R,G,B
			m_pRed = pDataset->GetRasterBand(1);
			m_pGreen = pDataset->GetRasterBand(2);
			m_pBlue = pDataset->GetRasterBand(3);
		}
		if ((NULL == m_pRed) || (NULL == m_pGreen) || (NULL == m_pBlue))
			throw "Couldn't find bands for Red, Green, Blue.";

		// Get overview count from Red band; assumes others match
		m_iViewCount = m_pRed->GetOverviewCount()+1;

		//m_pRed->GetBlockSize(&m_xBlockSize, &m_yBlockSize);
		//m_nxBlocks = (m_iXSize + m_xBlockSize - 1) / m_xBlockSize;
		//m_nyBlocks = (m_iYSize + m_yBlockSize - 1) / m_yBlockSize;

		m_pRedBlock = new uchar[m_MaxBlockSize];
		m_pGreenBlock = new uchar[m_MaxBlockSize];
		m_pBlueBlock = new uchar[m_MaxBlockSize];
	}
}

void LineBufferGDAL::FindMaxBlockSize(GDALDataset *pDataset)
{
	m_MaxBlockSize = -1;

	int rasters = pDataset->GetRasterCount();
	for (int i = 1; i <= rasters; i++)
	{
		GDALRasterBand *band = pDataset->GetRasterBand(i);
		int xblocksize, yblocksize;
		band->GetBlockSize(&xblocksize, &yblocksize);
		if (xblocksize * yblocksize > m_MaxBlockSize)
			m_MaxBlockSize = xblocksize * yblocksize;

		for (int j = 0; j < band->GetOverviewCount(); j++)
		{
			GDALRasterBand *ovrband = band->GetOverview(j);
			ovrband->GetBlockSize(&xblocksize, &yblocksize);
			if (xblocksize * yblocksize > m_MaxBlockSize)
				m_MaxBlockSize = xblocksize * yblocksize;
		}
	}
}

void LineBufferGDAL::Cleanup()
{
	if (NULL != m_pBlock)
		delete m_pBlock;
	if (NULL != m_pRedBlock)
		delete m_pRedBlock;
	if (NULL != m_pGreenBlock)
		delete m_pGreenBlock;
	if (NULL != m_pBlueBlock)
		delete m_pBlueBlock;

	m_pBlock = NULL;
	m_pRedBlock = NULL;
	m_pGreenBlock = NULL;
	m_pBlueBlock = NULL;
	m_pBand = NULL;
	m_pRed = NULL;
	m_pGreen = NULL;
	m_pBlue = NULL;

	for (int i = 0; i < BUF_SCANLINES; i++)
	{
		if (m_row[i].m_data)
			delete m_row[i].m_data;
		m_row[i].m_data = NULL;
	}
}

void LineBufferGDAL::ReadScanline(int iYRequest, int bufrow, int overview)
{
	m_linereads++;	// statistics

	CPLErr Err;
	GDALColorEntry Ent;
	int nxValid;

	if (m_iRasterCount == 1)
	{
		int xBlockSize, yBlockSize;
		m_pBand->GetBlockSize(&xBlockSize, &yBlockSize);
		int nxBlocks = (m_pBand->GetXSize() + xBlockSize - 1) / xBlockSize;
		int nyBlocks = (m_pBand->GetYSize() + yBlockSize - 1) / yBlockSize;

		int iyBlock = iYRequest / yBlockSize;
		int iY = iYRequest - (iyBlock * yBlockSize);

		// Convert to rgb and write to image
		for(int ixBlock = 0; ixBlock < nxBlocks; ixBlock++)
		{
			m_blockreads++;	// statistics

			Err = m_pBand->ReadBlock(ixBlock, iyBlock, m_pBlock);
			if (Err != CE_None)
				throw "Readblock failed.";

			// Compute the portion of the block that is valid
			// for partial edge blocks.
			if ((ixBlock+1) * xBlockSize > m_iXSize)
				nxValid = m_iXSize - ixBlock * xBlockSize;
			else
				nxValid = xBlockSize;

			for( int iX = 0; iX < nxValid; iX++ )
			{
				int val = m_pBlock[iY * xBlockSize + iX];
				if (m_pTable)
				{
					m_pTable->GetColorEntryAsRGB(val, &Ent);
					m_row[bufrow].m_data[iX].Set(Ent.c1, Ent.c2, Ent.c3);
				}
				else
				{
					// greyscale
					m_row[bufrow].m_data[iX].Set(val, val, val);
				}
			}
		}
	}
	else if (m_iRasterCount == 3)
	{
		GDALRasterBand *band1, *band2, *band3;
		if (m_iViewCount > 0 && overview > 0)
		{
			band1 = m_pRed->GetOverview(overview-1);
			band2 = m_pGreen->GetOverview(overview-1);
			band3 = m_pBlue->GetOverview(overview-1);
		}
		else
		{
			band1 = m_pRed;
			band2 = m_pGreen;
			band3 = m_pBlue;
		}

		int xBlockSize, yBlockSize;
		band1->GetBlockSize(&xBlockSize, &yBlockSize);
		int nxBlocks = (band1->GetXSize() + xBlockSize - 1) / xBlockSize;
		int nyBlocks = (band1->GetYSize() + yBlockSize - 1) / yBlockSize;

		int iyBlock = iYRequest / yBlockSize;
		int iY = iYRequest - (iyBlock * yBlockSize);

		RGBi rgb;
		for(int ixBlock = 0; ixBlock < nxBlocks; ixBlock++)
		{
			try {
				Err = band1->ReadBlock(ixBlock, iyBlock, m_pRedBlock);
				if (Err != CE_None)
					throw "Readblock (red) failed";
				Err = band2->ReadBlock(ixBlock, iyBlock, m_pGreenBlock);
				if (Err != CE_None)
					throw "Readblock (green) failed";
				Err = band3->ReadBlock(ixBlock, iyBlock, m_pBlueBlock);
				if (Err != CE_None)
					throw "Readblock (blue) failed";
			}
			catch (const char *msg)
			{
				VTLOG1(msg);
				VTLOG(" in ReadScanline(y=%d) (XSize %d, YSize %d, yBlock %d, yBlockSize %d, yBlocks %d)\n",
					iYRequest, m_iXSize, m_iYSize, iyBlock, yBlockSize, nyBlocks);

				// Posible TODO: Fill the buffer with some fixed value, like
				//  black, before returning
				return;
			}

			m_blockreads += 3;	// statistics

			// Compute the portion of the block that is valid
			// for partial edge blocks.
			if ((ixBlock+1) * xBlockSize > m_iXSize)
				nxValid = m_iXSize - ixBlock * xBlockSize;
			else
				nxValid = xBlockSize;

			for( int iX = 0; iX < nxValid; iX++ )
			{
				rgb.Set(m_pRedBlock[iY * xBlockSize + iX],
					m_pGreenBlock[iY * xBlockSize + iX],
					m_pBlueBlock[iY * xBlockSize + iX]);
				m_row[bufrow].m_data[ixBlock*xBlockSize + iX] = rgb;
			}
		}
	}
}

RGBi *LineBufferGDAL::GetScanlineFromBuffer(int y, int overview)
{
	// first check if the row is already in memory
	int i;
	for (i = 0; i < BUF_SCANLINES; i++)
	{
		if (m_row[i].m_y == y && m_row[i].m_overview == overview)
		{
			// yes it is
			return m_row[i].m_data;
		}
	}

	// ok, it isn't. load it into the next appropriate slot.
	ReadScanline(y, m_use_next, overview);
	m_row[m_use_next].m_y = y;
	m_row[m_use_next].m_overview = overview;
	m_row[m_use_next].m_data;
	RGBi *data = m_row[m_use_next].m_data;

	// increment which buffer row we'll use next
	m_use_next++;
	if (m_use_next == BUF_SCANLINES)
		m_use_next = 0;

	return data;
}


///////////////////////////////////////////////////////////////////////

vtImage::vtImage()
{
	SetDefaults();
}

vtImage::vtImage(const DRECT &area, int xsize, int ysize,
				 const vtProjection &proj)
{
	SetDefaults();
	m_Extents = area;
	m_proj = proj;

	// yes, we could use some error-checking here
	vtBitmap *pBitmap = new vtBitmap;
	pBitmap->Allocate(xsize, ysize);
	SetupBitmapInfo(xsize, ysize);
	m_Bitmaps[0].m_pBitmap = pBitmap;
}

vtImage::~vtImage()
{
	for (uint i = 0; i < m_Bitmaps.size(); i++)
	{
		if (m_Bitmaps[i].m_pBitmap != NULL)
			delete m_Bitmaps[i].m_pBitmap;
	}
	if (NULL != m_pDataset)
		GDALClose(m_pDataset);
}

void vtImage::SetDefaults()
{
	m_pDataset = NULL;
	m_pCanvas = NULL;
}

bool vtImage::GetExtent(DRECT &rect) const
{
	rect = m_Extents;
	return true;
}

void vtImage::DrawToView(wxDC *pDC, vtScaledView *pView)
{
	bool bDrawImage = true;

	// If there are mipmaps in memory, choose the appropriate one to draw
	vtBitmap *pBitmap = NULL;

	// Determine which overview resolution (with an in-memory bitmap) is
	//  most appropriate to draw
	double dRes = 1.0 / pView->GetScale();
	DPoint2 spacing = GetSpacing();
	double diff = 1E9;
	for (uint i = 0; i < m_Bitmaps.size(); i++)
	{
		double d2 = fabs(dRes - m_Bitmaps[i].m_Spacing.x);
		if (d2 < diff && m_Bitmaps[i].m_pBitmap)
		{
			diff = d2;
			pBitmap = m_Bitmaps[i].m_pBitmap;
		}
	}

	if (pBitmap == NULL)
		bDrawImage = false;

	wxRect screenrect = pView->WorldToCanvas(m_Extents);
	wxRect destRect = screenrect;

	//clip stuff, so we only blit what we need
	int client_width, client_height;
	wxPoint client1, client2;

	pView->GetClientSize(&client_width, &client_height); //get client window size
	pView->CalcUnscrolledPosition(0, 0, &client1.x, &client1.y);
	pView->CalcUnscrolledPosition(client_width, client_height, &client2.x, &client2.y);

	if ((destRect.x + destRect.width < client1.x) ||
		(destRect.y + destRect.height < client1.y) ||
		(destRect.x > client2.x) ||
		(destRect.y > client2.y))
		return;		//image completely off screen

	if (bDrawImage)
	{
		wxRect srcRect(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight());
		double ratio_x = (double) srcRect.width / destRect.width;
		double ratio_y = (double) srcRect.height / destRect.height;

		int diff, diff_source;

		// clip left
		diff = client1.x - destRect.x;
		diff_source = (int)(diff * ratio_x); // round to number of whole pixels
		diff = (int) (diff_source / ratio_x);
		if (diff > 0)
		{
			destRect.x += diff;
			destRect.width -= diff;
			srcRect.x += diff_source;
			srcRect.width -= diff_source;
		}

		// clip top
		diff = client1.y - destRect.y;
		diff_source = (int)(diff * ratio_y); // round to number of whole pixels
		diff = (int) (diff_source / ratio_y);
		if (diff > 0)
		{
			destRect.y += diff;
			destRect.height -= diff;
			srcRect.y += diff_source;
			srcRect.height -= diff_source;
		}

		// clip right
		diff = destRect.x + destRect.width - client2.x;
		diff_source = (int)(diff * ratio_x); // round to number of whole pixels
		diff = (int) (diff_source / ratio_x);
		if (diff > 0)
		{
			destRect.width -= diff;
			srcRect.width -= diff_source;
		}

		// clip bottom
		diff = destRect.y + destRect.height - client2.y;
		diff_source = (int)(diff * ratio_y); // round to number of whole pixels
		diff = (int) (diff_source / ratio_y);
		if (diff > 0)
		{
			destRect.height -= diff;
			srcRect.height -= diff_source;
		}

#if (wxVERSION_NUMBER > 2900)
		wxMemoryDC temp_dc;
		temp_dc.SelectObject(*pBitmap->m_pBitmap);
		pDC->StretchBlit(destRect.x, destRect.y,
			destRect.width, destRect.height,
			&temp_dc,
			srcRect.x, srcRect.y,
			srcRect.width, srcRect.height,
			wxCOPY);
#elif WIN32
		// Using StretchBlit is much faster and has less scaling/roundoff
		//  problems than using the wx method DrawBitmap
		::SetStretchBltMode((HDC) (pDC->GetHDC()), HALFTONE );

		wxDC2 *pDC2 = (wxDC2 *) pDC;
		pDC2->StretchBlit(*pBitmap->m_pBitmap, destRect.x, destRect.y,
			destRect.width, destRect.height, srcRect.x, srcRect.y,
			srcRect.width, srcRect.height);
#else
		// scale and draw the bitmap
		// must use SetUserScale since StretchBlt is not available
		double scale_x = 1.0/ratio_x;
		double scale_y = 1.0/ratio_y;
		pDC->SetUserScale(scale_x, scale_y);
		pDC->DrawBitmap(*pBitmap->m_pBitmap, (int) (destRect.x/scale_x),
			(int) (destRect.y/scale_y), false);

		// restore
		pDC->SetUserScale(1.0, 1.0);
#endif
	}
	else
	{
		// Draw placeholder yellow frame
		wxPen yellow(wxColor(255,255,0), 1, wxSOLID);
		pDC->SetLogicalFunction(wxCOPY);
		pDC->SetPen(yellow);

		DrawRectangle(pDC, screenrect, true);
	}
}

bool vtImage::ConvertProjection(vtImage *pOld, vtProjection &NewProj,
								int iSampleN, bool progress_callback(int))
{
	int i, j;

	// Create conversion object
	const vtProjection *pSource, *pDest;
	pSource = &pOld->GetAtProjection();
	pDest = &NewProj;

	OCT *trans = CreateCoordTransform(pSource, pDest);
	if (!trans)
	{
		// inconvertible projections
		return false;
	}

	// find where the extent corners are going to be in the new terrain
	DRECT OldExtents;
	pOld->GetExtent(OldExtents);
	DLine2 OldCorners;
	OldCorners.Append(DPoint2(OldExtents.left, OldExtents. bottom));
	OldCorners.Append(DPoint2(OldExtents.right, OldExtents. bottom));
	OldCorners.Append(DPoint2(OldExtents.right, OldExtents. top));
	OldCorners.Append(DPoint2(OldExtents.left, OldExtents. top));

	DLine2 Corners = OldCorners;
	m_Extents.SetRect(1E9, -1E9, -1E9, 1E9);
	int success;
	for (i = 0; i < 4; i++)
	{
		success = trans->Transform(1, &Corners[i].x, &Corners[i].y);
		if (success == 0)
		{
			// inconvertible projections
			delete trans;
			return false;
		}
		m_Extents.GrowToContainPoint(Corners[i]);
	}
	delete trans;

	// now, how large an array will we need for the new terrain?
	// try to preserve the sampling rate approximately
	//
	bool bOldGeo = (pSource->IsGeographic() != 0);
	bool bNewGeo = (pDest->IsGeographic() != 0);

	DPoint2 old_step = pOld->GetSpacing();
	DPoint2 new_step;
	double meters_per_longitude;

	if (bOldGeo && !bNewGeo)
	{
		// convert degrees to meters (approximately)
		meters_per_longitude = MetersPerLongitude(OldCorners[0].y);
		new_step.x = old_step.x * meters_per_longitude;
		new_step.y = old_step.y * METERS_PER_LATITUDE;
	}
	else if (!bOldGeo && bNewGeo)
	{
		// convert meters to degrees (approximately)
		meters_per_longitude = MetersPerLongitude(Corners[0].y);
		new_step.x = old_step.x / meters_per_longitude;
		new_step.y = old_step.y / METERS_PER_LATITUDE;	// convert degrees to meters (approximately)
	}
	else
	{
		// check horizontal units or old and new terrain
		double units_old = pSource->GetLinearUnits(NULL);
		double units_new = pDest->GetLinearUnits(NULL);
		new_step = old_step * (units_old / units_new);
	}
	double fColumns = m_Extents.Width() / new_step.x;
	double fRows = m_Extents.Height() / new_step.y;

	// round up to the nearest integer
	int iXSize = (int)(fColumns + 0.999);
	int iYSize = (int)(fRows + 0.999);

	// do safety checks
	if (iXSize < 1 || iYSize < 1)
		return false;
	if (iXSize > 40000 || iYSize > 40000)
		return false;

	// Now we're ready to fill in the new image.
	m_proj = NewProj;
	vtBitmap *pBitmap = new vtBitmap;
	if (!pBitmap->Allocate(iXSize, iYSize))
		return false;
	SetupBitmapInfo(iXSize, iYSize);
	m_Bitmaps[0].m_pBitmap = pBitmap;

	// Convert each bit of data from the old array to the new
	// Transformation points backwards, from the target to the source
	trans = CreateCoordTransform(pDest, pSource);
	if (!trans)
	{
		// inconvertible projections
		// "Couldn't convert between coordinate systems.";
		return false;
	}

	// Prepare to multisample
	DPoint2 step = GetSpacing();
	DLine2 offsets;
	MakeSampleOffsets(step, iSampleN, offsets);

	DPoint2 p, mp;
	RGBi value, sum;
	int count;
	for (i = 0; i < iXSize; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i*99/iXSize);

		for (j = 0; j < iYSize; j++)
		{
			// Sample at pixel centers
			p.x = m_Extents.left + (step.x/2) + (i * step.x);
			p.y = m_Extents.bottom + (step.y/2)+ (j * step.y);

			count = 0;
			sum.Set(0,0,0);
			for (uint k = 0; k < offsets.GetSize(); k++)
			{
				mp = p + offsets[k];

				// Since transforming the extents succeeded, it's safe to assume
				// that the points will also transform without errors.
				trans->Transform(1, &mp.x, &mp.y);

				if (pOld->GetColorSolid(mp, value))
				{
					count++;
					sum += value;
				}
			}
			if (count > 0)
				SetRGB(i, iYSize-1-j, sum / count);
			else
				SetRGB(i, iYSize-1-j, RGBi(0,0,0));	// nodata
		}
	}
	delete trans;

	return true;
}

void vtImage::GetProjection(vtProjection &proj) const
{
	proj = m_proj;
}

void vtImage::SetProjection(const vtProjection &proj)
{
	m_proj = proj;
}

void vtImage::SetExtent(const DRECT &rect)
{
	m_Extents = rect;
}

DPoint2 vtImage::GetSpacing(int bitmap) const
{
	return DPoint2(m_Extents.Width() / m_Bitmaps[bitmap].m_Size.x,
		m_Extents.Height() / m_Bitmaps[bitmap].m_Size.y);
}

/**
 * Reprojects an image by converting just the extents to a new
 * projection.
 *
 * This is much faster than creating a new grid and reprojecting every
 * heixel, but it only produces correct results when the difference
 * between the projections is only a horizontal shift.  For example, this
 * occurs when the only difference between the old and new projection
 * is choice of Datum.
 *
 * \param proj_new	The new projection to convert to.
 *
 * \return True if successful.
 */
bool vtImage::ReprojectExtents(const vtProjection &proj_new)
{
	// Create conversion object
	OCT *trans = CreateCoordTransform(&m_proj, &proj_new);
	if (!trans)
		return false;	// inconvertible projections

	int success = 0;
	success += trans->Transform(1, &m_Extents.left, &m_Extents.bottom);
	success += trans->Transform(1, &m_Extents.right, &m_Extents.top);
	delete trans;

	if (success != 2)
		return false;	// inconvertible projections

	m_proj = proj_new;
	return true;
}

/**
 * Sample image color at a given point, assuming that the pixels are solid.
 *
 * \return true if a value was found, false if the point is outside the
 *		extent or (if the option is enabled) the value was 'nodata'.
 */
bool vtImage::GetColorSolid(const DPoint2 &p, RGBi &rgb, double dRes)
{
	// could speed this up by keeping these values around
	const DPoint2 &spacing = m_Bitmaps[0].m_Spacing;
	const IPoint2 &size = m_Bitmaps[0].m_Size;

	double u = (p.x - m_Extents.left) / spacing.x;
	if (u < -0.5 || u > size.x+0.5) return false; // check extents
	if (u < 0.5) u = 0.5; // adjust left edge
	if (u > size.x-0.5) u = size.x-0.5; // adjust right edge
	int ix = (int) u; // round to closest pixel

	double v = (m_Extents.top - p.y) / spacing.y;
	if (v < -0.5 || v > size.y+0.5) return false; // check extents
	if (v < 0.5) v = 0.5; // adjust top edge
	if (v > size.y-0.5) v = size.y-0.5; // adjust bottom edge
	int iy = (int) v; // round to closest pixel

	GetRGB(ix, iy, rgb, dRes);
	if (bTreatBlackAsTransparent && rgb == RGBi(0,0,0))
		return false;

	return true;
}

void MakeSampleOffsets(const DPoint2 cellsize, uint N, DLine2 &offsets)
{
	DPoint2 spacing = cellsize / N;
	DPoint2 base = spacing * -((float) (N-1) / 2);
	for (uint i = 0; i < N; i++)
		for (uint j = 0; j < N; j++)
			offsets.Append(DPoint2(base.x + (i*spacing.x), base.y + (j*spacing.y)));
}

/**
 * Get image color by sampling several points and averaging them.
 * The area to test is given by center and offsets, use MakeSampleOffsets()
 * to make a set of N x N offsets.
 */
bool vtImage::GetMultiSample(const DPoint2 &p, const DLine2 &offsets, RGBi &rgb, double dRes)
{
	RGBi color;
	rgb.Set(0,0,0);
	int count = 0;
	for (uint i = 0; i < offsets.GetSize(); i++)
	{
		if (GetColorSolid(p+offsets[i], color, dRes))
		{
			rgb += color;
			count++;
		}
	}
	if (count)
	{
		rgb /= count;
		return true;
	}
	return false;
}

int vtImage::NumBitmapsInMemory()
{
	int count = 0;
	for (uint i = 0; i < m_Bitmaps.size(); i++)
		if (m_Bitmaps[i].m_pBitmap != NULL)
			count++;
	return count;
}

int vtImage::NumBitmapsOnDisk()
{
	int count = 0;
	for (uint i = 0; i < m_Bitmaps.size(); i++)
		if (m_Bitmaps[i].m_bOnDisk)
			count++;
	return count;
}

void vtImage::GetRGB(int x, int y, RGBi &rgb, double dRes)
{
	int closest_bitmap = -1;
	double diff = 1E9;

	if (dRes == 0.0)
	{
		// no resolution to match, take first available in memory
		for (int i = 0; i < (int)m_Bitmaps.size(); i++)
		{
			if (m_Bitmaps[i].m_pBitmap)
			{
				closest_bitmap = i;
				break;
			}
		}
	}
	else
	{
		// What overview resolution is most appropriate
		for (int i = 0; i < (int)m_Bitmaps.size(); i++)
		{
			// if it is available
			if (m_Bitmaps[i].m_pBitmap || m_Bitmaps[i].m_bOnDisk)
			{
				double spc = (m_Bitmaps[i].m_Spacing.x + m_Bitmaps[i].m_Spacing.y)/2.0;
				double rel_spc = fabs(dRes - spc);

				if (rel_spc < diff)
				{
					diff = rel_spc;
					closest_bitmap = i;
				}
			}
		}
	}

	if (closest_bitmap < 0)
	{
		// safety measure for missing overviews
		rgb=RGBi(255,0,0);
		return;
	}

	if (closest_bitmap != 0)
	{
		// get smaller coordinates from subsampled view
		x >>= closest_bitmap;
		y >>= closest_bitmap;
	}

	const BitmapInfo &bm = m_Bitmaps[closest_bitmap];
	if (bm.m_pBitmap)
	{
		// get pixel from bitmap in memory
		bm.m_pBitmap->GetPixel24(x, y, rgb);
	}
	else if (bm.m_bOnDisk)
	{
		// support for out-of-memory image
		RGBi *data = m_linebuf.GetScanlineFromBuffer(y, closest_bitmap);
		rgb = data[x];
	}
}

void vtImage::SetRGB(int x, int y, uchar r, uchar g, uchar b)
{
	// this method clearly only works for in-memory images
	if (m_Bitmaps[0].m_pBitmap)
		m_Bitmaps[0].m_pBitmap->SetPixel24(x, y, r, g, b);
}

void vtImage::SetRGB(int x, int y, const RGBi &rgb)
{
	// this method clearly only works for in-memory images
	if (m_Bitmaps[0].m_pBitmap)
		m_Bitmaps[0].m_pBitmap->SetPixel24(x, y, rgb);
}

void vtImage::ReplaceColor(const RGBi &rgb1, const RGBi &rgb2)
{
	// this method only works for in-memory images
	BitmapInfo &bmi = m_Bitmaps[0];
	if (!bmi.m_pBitmap)
		return;
	RGBi color;
	for (int i = 0; i < bmi.m_Size.x; i++)
		for (int j = 0; j < bmi.m_Size.y; j++)
		{
			bmi.m_pBitmap->GetPixel24(i, j, color);
			if (color == rgb1)
				bmi.m_pBitmap->SetPixel24(i, j, rgb2);
		}
}

void vtImage::SetupBitmapInfo(int iXSize, int iYSize)
{
	DPoint2 spacing(m_Extents.Width() / iXSize, m_Extents.Height() / iYSize);
	int smaller = min(iXSize, iYSize);

	// How many mipmaps to consider?
	int powers = vt_log2(smaller) - 3;

	m_Bitmaps.resize(powers);
	for (int m = 0; m < powers; m++)
	{
		m_Bitmaps[m].number = m;
		m_Bitmaps[m].m_pBitmap = NULL;
		m_Bitmaps[m].m_bOnDisk = false;
		m_Bitmaps[m].m_Size.Set(iXSize, iYSize);
		m_Bitmaps[m].m_Spacing = spacing;

		iXSize /= 2;
		iYSize /= 2;
		spacing *= 2;
	}
}

void vtImage::AllocMipMaps()
{
	FreeMipMaps();

	if (m_Bitmaps[0].m_pBitmap == NULL)
		return;

	IPoint2 size = m_Bitmaps[0].m_Size;
	int depth = m_Bitmaps[0].m_pBitmap->GetDepth();

	for (size_t m = 1; m < m_Bitmaps.size(); m++)
	{
		if (!m_Bitmaps[m].m_pBitmap)
		{
			vtBitmap *bm = new vtBitmap;
			bm->Allocate(size.x >> m, size.y >> m, depth);
			m_Bitmaps[m].m_pBitmap = bm;
		}
	}
}

void vtImage::DrawMipMaps()
{
	vtBitmap *big = m_Bitmaps[0].m_pBitmap;
	for (size_t m = 1; m < m_Bitmaps.size(); m++)
	{
		vtBitmap *smaller = m_Bitmaps[m].m_pBitmap;
		SampleMipLevel(big, smaller);
		big = smaller;
	}
}

void vtImage::FreeMipMaps()
{
	for (size_t m = 1; m < m_Bitmaps.size(); m++)
	{
		delete m_Bitmaps[m].m_pBitmap;
		m_Bitmaps[m].m_pBitmap = NULL;
	}
}

bool vtImage::ReadPPM(const char *fname, bool progress_callback(int))
{
	// open input file
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)		// Could not open input file
		return false;

	char sbuf[512],			// buffer for file input
		 dummy[2];			// used for \n and \0
	if (fread(sbuf, sizeof(char), 2, fp) != 2)
	{
		// "Could not read file"
		return false;
	}

	if (strncmp(sbuf, "P6", 2))
	{
		// not the flavor of PNM we're expecting
		fclose(fp);
		return false;
	}

	// P5 and P2 are not the flavors of PNM we're expecting:
	// if (!strncmp(sbuf, "P5", 2) || !strncmp(sbuf, "P2", 2))

	// read PGM ASCII or binary file
	bool have_header = false;
	int coord_sys = 0;
	int coord_zone = 0;
	int coord_datum = 0;
	DRECT ext;
	double x, y;
	int quiet;
	while ((fscanf(fp, "%s", sbuf) != EOF) && sbuf[0] == '#')
	{
		// comment
		quiet = fscanf(fp,"%[^\n]", sbuf);  // read comment beyond '#'
		quiet = fscanf(fp,"%[\n]", dummy);  // read newline
		char *buf = sbuf+1;	// skip leading space after '#'
		if (!strncmp(buf, "DEM", 3))
		{
			have_header= true;
		}
		else if (!strncmp(buf, "description", 11))
		{
			// do nothing
		}
		else if (!strncmp(buf, "coordinate system", 17))
		{
			if (!strcmp(buf+18, "LL")) coord_sys=0;
			else if (!strcmp(buf+18,"UTM")) coord_sys=1;
		}
		else if (!strncmp(buf, "coordinate zone", 15))
		{
			coord_zone = atoi(buf+16);
		}
		else if (!strncmp(buf, "coordinate datum", 16))
		{
			coord_datum = atoi(buf+17);
		}
		else if (!strncmp(buf, "SW corner", 9))
		{
			sscanf(buf+10, "%lf/%lf", &x, &y);
			ext.left = x;
			ext.bottom = y;
		}
		else if (!strncmp(buf, "NE corner", 9))
		{
			sscanf(buf+10, "%lf/%lf", &x, &y);
			ext.right = x;
			ext.top = y;
		}
		else if (!strncmp(buf, "missing value", 13))
		{
			// do nothing
		}
	}

	int iXSize = atoi(sbuf);		// store xsize of array
	quiet = fscanf(fp,"%s",sbuf);		// read ysize of array
	int iYSize = atoi(sbuf);
	quiet = fscanf(fp,"%s\n",sbuf);		// read maxval of array
	int maxval = atoi(sbuf);

	// set the corresponding vtElevationGrid info
	if (have_header)
	{
		int datum = EPSG_DATUM_WGS84;
		switch (coord_datum)
		{
		case 1: datum = EPSG_DATUM_NAD27; break;
		case 2: datum = EPSG_DATUM_WGS72; break;
		case 3: datum = EPSG_DATUM_WGS84; break;
		case 4: datum = EPSG_DATUM_NAD83; break;

		case 5: /*Sphere (with radius 6370997 meters)*/ break;
		case 6: /*Clarke1880 (Clarke spheroid of 1880)*/ break;
		case 7: /*International1909 (Geodetic Reference System of 1909)*/ break;
		case 8: /*International1967 (Geodetic Reference System of 1967)*/; break;

		case 9: /*WGS60*/ break;
		case 10: /*WGS66*/; break;
		case 11: /*Bessel1841*/ break;
		case 12: /*Krassovsky*/ break;
		}
		if (coord_sys == 0)	// LL
		{
			m_proj.SetProjectionSimple(false, 0, datum);
			ext.left /= 3600;	// arc-seconds to degrees
			ext.right /= 3600;
			ext.top /= 3600;
			ext.bottom /= 3600;
		}
		else if (coord_sys == 1)	// UTM
			m_proj.SetProjectionSimple(true, coord_zone, datum);
	}
	else
	{
		// Set the projection (actually we don't know it)
		m_proj.SetProjectionSimple(true, 1, EPSG_DATUM_WGS84);

		ext.left = 0;
		ext.top = iYSize-1;
		ext.right = iXSize-1;
		ext.bottom = 0;
	}
	m_Extents = ext;
	vtBitmap *pBitmap = new vtBitmap;
	pBitmap->Allocate(iXSize, iYSize, 24);
	SetupBitmapInfo(iXSize, iYSize);
	m_Bitmaps[0].m_pBitmap = pBitmap;

	// read PPM binary
	int offset_start = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int offset_end = ftell(fp);
	fseek(fp, offset_start, SEEK_SET);	// go back again

	int data_length = offset_end - offset_start;
	int data_size = data_length / (iXSize*iYSize);

	int line_length = 3 * iXSize;
	uchar *line = new uchar[line_length];

	for (int j = 0; j < iYSize; j++)
	{
		if (progress_callback != NULL)
			progress_callback(j * 100 / iYSize);

		quiet = (int) fread(line, line_length, 1, fp);

		for (int i = 0; i < iXSize; i++)
			pBitmap->SetPixel24(i, j, line[i*3+0], line[i*3+1], line[i*3+2]);
	}
	delete [] line;
	fclose(fp);
	return true;
}

bool vtImage::WritePPM(const char *fname) const
{
	vtBitmap *bm = m_Bitmaps[0].m_pBitmap;
	if (!bm)
		return false;

	// open input file
	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)		// Could not open output file
		return false;

	fprintf(fp, "P6\n");		// PGM binary format
	fprintf(fp, "%d %d\n", bm->GetWidth(), bm->GetHeight());
	fprintf(fp, "255\n");		// PGM standard value

	int line_length = 3 * bm->GetWidth();
	uchar *line = new uchar[line_length];

	RGBi rgb;
	for (uint j = 0; j < bm->GetHeight(); j++)
	{
		for (uint i = 0; i < bm->GetWidth(); i++)
		{
			bm->GetPixel24(i, j, rgb);
			line[i*3+0] = rgb.r;
			line[i*3+1] = rgb.g;
			line[i*3+2] = rgb.b;
		}
		fwrite(line, line_length, 1, fp);
	}
	delete [] line;
	fclose(fp);
	return true;
}

bool vtImage::SaveToFile(const char *fname) const
{
	vtBitmap *bm = m_Bitmaps[0].m_pBitmap;
	if (!bm)
		return false;
	IPoint2 size = m_Bitmaps[0].m_Size;

	DPoint2 spacing = GetSpacing();
	vtString sExt = GetExtension(fname, false); //get extension type
	if (sExt.CompareNoCase(".jpg") == 0)
	{
		if (bm->WriteJPEG(fname, 90))
		{
			// Also write JPEG world file
			vtString sJGWFile = ChangeFileExtension(fname, ".jgw");

			FILE *fout = vtFileOpen(sJGWFile, "w");
			if (fout)
			{
				// World file extents are always (strangely) pixel centers,
				//  not full image extents, so we have to scoot inwards by
				//  half a pixel.
				fprintf(fout, "%lf\n%lf\n%lf\n%lf\n%.2lf\n%.2lf\n",
					spacing.x, 0.0, 0.0, -1*spacing.y,
					m_Extents.left + (spacing.x/2),
					m_Extents.top - (spacing.y/2));
				fclose(fout);
			}
			return true;
		}
		else
			return false;
	}

	// Save with GDAL
	GDALDriverManager *pManager = GetGDALDriverManager();
	if (!pManager)
		return false;

	// JPEG: Error 6: GDALDriver::Create() ... no create method implemented for this format.
//	GDALDriver *pDriver = pManager->GetDriverByName("JPEG");
	GDALDriver *pDriver = pManager->GetDriverByName("GTiff");
	if (!pDriver)
		return false;

	char **papszOptions = NULL;

	// COMPRESS=[JPEG/LZW/PACKBITS/DEFLATE/CCITTRLE/CCITTFAX3/CCITTFAX4/NONE]
	if (g_Options.GetValueBool(TAG_TIFF_COMPRESS))
	    papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "DEFLATE" );

	GDALDataset *pDataset;
	pDataset = pDriver->Create(fname, size.x, size.y, 3, GDT_Byte, papszOptions );
    CSLDestroy(papszOptions);

	if (!pDataset)
		return false;

	double adfGeoTransform[6] = { m_Extents.left, spacing.x, 0, m_Extents.top, 0, -spacing.y };
	pDataset->SetGeoTransform(adfGeoTransform);

	GByte *raster = new GByte[size.x*size.y];

	char *pszSRS_WKT = NULL;
	m_proj.exportToWkt( &pszSRS_WKT );
	pDataset->SetProjection(pszSRS_WKT);
	CPLFree( pszSRS_WKT );

	if (size.x * size.y > 512*512)
		OpenProgressDialog(_("Writing file"), false);

	// TODO: ask Frank if there is a way to gave GDAL write the file without
	// having to make another entire copy in memory, as it does now:
	RGBi rgb;
	GDALRasterBand *pBand;
	int i, x, y;
	for (i = 1; i <= 3; i++)
	{
		pBand = pDataset->GetRasterBand(i);

		for (y = 0; y < size.y; y++)
		{
			progress_callback((i-1)*33 + (y * 33 / size.y));

			for (x = 0; x < size.x; x++)
			{
				bm->GetPixel24(x, y, rgb);
				if (i == 1) raster[y*size.x + x] = rgb.r;
				if (i == 2) raster[y*size.x + x] = rgb.g;
				if (i == 3) raster[y*size.x + x] = rgb.b;
			}
		}
		pBand->RasterIO( GF_Write, 0, 0, size.x, size.y,
			raster, size.x, size.y, GDT_Byte, 0, 0 );
	}
	delete [] raster;
	GDALClose(pDataset);

	CloseProgressDialog();

	return true;
}

bool vtImage::ReadPNGFromMemory(uchar *buf, int len)
{
	vtBitmap *pBitmap = new vtBitmap;
	if (pBitmap->ReadPNGFromMemory(buf, len))
	{
		SetupBitmapInfo(pBitmap->GetWidth(), pBitmap->GetHeight());
		m_Bitmaps[0].m_pBitmap = pBitmap;
		return true;
	}
	else
	{
		delete pBitmap;
		return false;
	}
}

bool vtImage::LoadFromGDAL(const char *fname)
{
	bool bRet = true;

	double affineTransform[6];
	const char *pProjectionString;
	OGRErr err;
	bool bDefer = false;
	vtString message;

	g_GDALWrapper.RequestGDALFormats();

	// GDAL doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(fname);

	try
	{
		m_pDataset = (GDALDataset *) GDALOpen(fname_local, GA_Update);
		if (m_pDataset == NULL )
		{
			m_pDataset = (GDALDataset *) GDALOpen(fname_local, GA_ReadOnly);
			if (m_pDataset == NULL )
				throw "Couldn't open that file.";
		}

		int iXSize = m_pDataset->GetRasterXSize();
		int iYSize = m_pDataset->GetRasterYSize();

		IPoint2 OriginalSize;
		OriginalSize.x = iXSize;
		OriginalSize.y = iYSize;

		vtProjection temp;
		bool bHaveProj = false;
		pProjectionString = m_pDataset->GetProjectionRef();
		if (pProjectionString)
		{
			err = temp.importFromWkt((char**)&pProjectionString);

			// we must have a valid CRS, and it must not be local
			if (err == OGRERR_NONE && !temp.IsLocal())
			{
				m_proj = temp;
				bHaveProj = true;
			}
		}
		if (!bHaveProj)
		{
			// check for existence of .prj file
			bool bSuccess = temp.ReadProjFile(fname_local);
			if (bSuccess)
			{
				m_proj = temp;
				bHaveProj = true;
			}
		}
		// if we still don't have it
		if (!bHaveProj)
		{
			if (!g_bld->ConfirmValidCRS(&m_proj))
				throw "Import Cancelled.";
		}

		if (m_pDataset->GetGeoTransform(affineTransform) == CE_None)
		{
			m_Extents.left = affineTransform[0];
			m_Extents.right = m_Extents.left + affineTransform[1] * iXSize;
			m_Extents.top = affineTransform[3];
			m_Extents.bottom = m_Extents.top + affineTransform[5] * iYSize;
		}
		else
		{
			// No extents.
			m_Extents.Empty();
			wxString msg = _("File lacks geographic location (extents).  ");
			msg += _("Would you like to specify extents?\n");
			VTLOG(msg.mb_str(wxConvUTF8));
			int res = wxMessageBox(msg, _("Image Import"), wxYES_NO | wxCANCEL);
			if (res == wxYES)
			{
				VTLOG("Yes.\n");
				DRECT ext;
				ext.Empty();
				ExtentDlg dlg(NULL, -1, _("Extents"));
				dlg.SetArea(ext, (m_proj.IsGeographic() != 0));
				if (dlg.ShowModal() == wxID_OK)
					m_Extents = dlg.m_area;
				else
					throw "Import Cancelled.";
			}
			if (res == wxNO)
			{
				throw "Sorry, we need extents in order to make use of an image.";
			}
			if (res == wxCANCEL)
				throw "Import Cancelled.";
		}

		SetupBitmapInfo(iXSize, iYSize);

		if (iXSize * iYSize > 512*512)
			OpenProgressDialog(_("Reading file"), false);

		m_linebuf.Setup(m_pDataset);
		for (int i = 0; i < m_linebuf.m_iViewCount && i < (int) m_Bitmaps.size(); i++)
			m_Bitmaps[i].m_bOnDisk = true;

		int iBigImage = g_Options.GetValueInt(TAG_MAX_MEGAPIXELS) * 1024 * 1024;
		// don't try to load giant image?
		wxString msg;
		if (iXSize * iYSize > iBigImage)
		{
			if (g_Options.GetValueBool(TAG_LOAD_IMAGES_ALWAYS))
				bDefer = false;
			else if (g_Options.GetValueBool(TAG_LOAD_IMAGES_NEVER))
				bDefer = true;
			else
			{
				// Ask
				msg.Printf(_("Image is very large (%d x %d).\n"), iXSize, iYSize);
				msg += _("Would you like to create the layer using out-of-memory access to the image?"),
				VTLOG(msg.mb_str(wxConvUTF8));
				int result = wxMessageBox(msg, _("Question"), wxYES_NO);
				if (result == wxYES)
					bDefer = true;
			}
		}

		if (!bDefer)
		{
			vtBitmap *pBitmap = new vtBitmap;
			if (!pBitmap->Allocate(iXSize, iYSize))
			{
				delete pBitmap;
				msg.Printf(_("Couldn't allocate bitmap of size %d x %d.\n"),
					iXSize, iYSize);
				msg += _("Would you like to create the layer using out-of-memory access to the image?"),
				VTLOG(msg.mb_str(wxConvUTF8));
				int result = wxMessageBox(msg, _("Question"), wxYES_NO);
				if (result == wxYES)
					bDefer = true;
				else
					throw "Couldn't allocate bitmap";
			}

			if (!bDefer)
			{
				// Read the data
				VTLOG("Reading the image data (%d x %d pixels)\n", iXSize, iYSize);
				RGBi rgb;
				for (int iY = 0; iY < iYSize; iY++ )
				{
					if (UpdateProgressDialog(iY * 99 / iYSize))
					{
						// cancel
						throw "Cancelled";
					}
					for (int iX = 0; iX < iXSize; iX++ )
					{
						RGBi *data = m_linebuf.GetScanlineFromBuffer(iY, 0);
						rgb = data[iX];
						pBitmap->SetPixel24(iX, iY, rgb);
					}
				}
				pBitmap->ContentsChanged();
				m_Bitmaps[0].m_pBitmap = pBitmap;
			}
		}
	}

	catch (const char *msg)
	{
		if (!bDefer)
		{
			bRet = false;

			vtString str = "Couldn't load Image layer ";

			str += "\"";
			str += fname;
			str += "\"";

			str += ": ";
			str += msg;

			DisplayAndLog(str);
		}
	}

	CloseProgressDialog();

	// Don't close the GDAL Dataset; leave it open just in case we need it.
//	GDALClose(pDataset);

	return bRet;
}

int CPL_STDCALL GDALProgress(double amount, const char *message, void *context)
{
	static int lastamount = -1;
	int newamount = amount * 99;
	if (newamount != lastamount)
	{
		lastamount = newamount;
		progress_callback(newamount);
	}
	return 1;
}

bool vtImage::CreateOverviews()
{
	if (!m_pDataset)
		return false;

	int panOverviewList[4] = { 2, 4, 8, 16 };
	int nOverviews = 4;
	CPLErr err = m_pDataset->BuildOverviews("AVERAGE", nOverviews, panOverviewList,
		0, NULL, GDALProgress, this);

	m_linebuf.Setup(m_pDataset);

	return (err == CE_None);
}


#if SUPPORT_CURL
#include "vtdata/TripDub.h"

/*
# some terraserver hints from http://mapper.acme.com/about.html...
#
# A Terraserver tile URL looks like this:
# http://terraserver-usa.com/tile.ashx?S=10&T=1&X=2809&Y=20964&Z=10
# The parameters are as follows:
#   T: theme, 0=relief 1=image 2=topo
#   S: scale, ranges are:
#		T=0: 20-24
#		T=1: 10-16
#		T=2: 11-21
#   X: UTM easting / pixels per tile / meters per pixel (@ SW corner)
#   Y: UTM northing / pixels per tile / meters per pixel (@ SW corner)
#   Z: UTM numeric zone
# Pixels per tile is 200.  Meters per pixel is 2 ^ ( scale - 10 ).
*/

int PixelsPerTile =  200;	// terraserver tiles are 200x200 .jpg images.
int MetersPerPixel;
int MetersPerTile;

int TerrainZone;
int TerrainEastingW;
int TerrainEastingE;
int TerrainNorthingS;
int TerrainNorthingN;

int TileScaleId;
#define TileDownloadDir "terraserver_tiles"	// cache for downloaded tiles
int TileThemeId = 1;	// 1 = aerial imagery

// generate the local filename of a single terraserver image tile
vtString TileNameLocal(int easting, int northing)
{
	int x = easting / MetersPerTile;
	int y = northing / MetersPerTile;

	vtString tilename;
	tilename.Format("tile_S_%d_T_%d_X_%d_Y_%d_Z_%d.jpg",
		TileScaleId, TileThemeId, x, y, TerrainZone);
	return tilename;
}

// generate the URL to fetch an image tile from terraserver
vtString TileURL(int easting, int northing)
{
	int x = easting / MetersPerTile;
	int y = northing / MetersPerTile;

	// "tile.ashx?S=${TileScaleId}&T=${TileThemeId}&X=${x}&Y=${y}&Z=${TerrainZone}"
	vtString url;
	url.Format("http://terraserver-usa.com/tile.ashx?S=%d&T=%d&X=%d&Y=%d&Z=%d",
		TileScaleId, TileThemeId, x, y, TerrainZone);
	return url;
}

// generate the filename of a locally-cached image tile
vtString TileFileName(int easting, int northing)
{
	vtString fname = "./";
	fname += TileDownloadDir;
	fname += "/";
	fname += TileNameLocal(easting, northing);
	return fname;
}

// if a tile is not already downloaded, generate the URL and wget it.
bool DownloadATile(int easting, int northing)
{
	vtString filename = TileFileName(easting, northing);

	// check if file exists
	FILE *fp = vtFileOpen(filename, "rb");
	if (fp)
	{
		fclose(fp);
		VTLOG("already have %s\n", (const char *) filename);
	}
	else
	{
		fp = vtFileOpen(filename, "wb");
		if (!fp)
			return false;

		vtString url = TileURL(easting, northing);

		vtBytes buffer;
		ReqContext cl;
		if (cl.GetURL(url, buffer) == false)
			return false;

		fwrite(buffer.Get(), 1, buffer.Len(), fp);
		fclose(fp);
	}
	return true;
}

// ensure we have all the image tiles needed for our terrain coverage
bool DownloadAllTiles()
{
	int startn = TerrainNorthingS / MetersPerTile * MetersPerTile;
	int starte = TerrainEastingW / MetersPerTile * MetersPerTile;
	int stopn = TerrainNorthingN;
	int stope = TerrainEastingE;
	int numn = ((stopn - startn) / MetersPerTile) + 1;
	int nume = ((stope - starte) / MetersPerTile) + 1;
	int count = 0, total = numn * nume;

	for (int n = startn; n < stopn; n += MetersPerTile)
	{
		for (int e = starte; e < stope; e += MetersPerTile)
		{
			vtString msg = TileFileName(e, n);
			UpdateProgressDialog(count * 100 / total, wxString(msg, wxConvUTF8));

			if (!DownloadATile(e, n))
				return false;

			count++;
		}
	}
	return true;
}

// mosaic all the tiles into one image, then crop
bool MosaicAllTiles(vtBitmapBase &output)
{
	vtString tilelist, cmd;

	int startn = TerrainNorthingS / MetersPerTile * MetersPerTile;
	for (int n = startn; n < TerrainNorthingN; n += MetersPerTile)
	{
		int y = (int) ((double)(TerrainNorthingN - n - MetersPerTile) / MetersPerPixel);

		int starte = TerrainEastingW / MetersPerTile * MetersPerTile;
		for (int e = starte; e < TerrainEastingE; e += MetersPerTile)
		{
			int x = (int) ((double)(e - TerrainEastingW) / MetersPerPixel);

			vtString fname = TileFileName(e, n);
			vtDIB tile;
			if (tile.ReadJPEG(fname))
				tile.BlitTo(output, x, y);
		}
	}
	return true;
}

#endif

bool vtImage::ReadFeaturesFromTerraserver(const DRECT &area, int iTheme,
											   int iMetersPerPixel, int iUTMZone,
											   const char *filename)
{
#if SUPPORT_CURL

	// The cache directory needs to exist; test if it's already there.
	const char *testname = TileDownloadDir "/test.txt";
	FILE *fp = vtFileOpen(testname, "wb");
	if (fp)
	{
		// directory already exists
		fclose(fp);
		vtDeleteFile(testname);
	}
	else
	{
		bool success = vtCreateDir("terraserver_tiles");
		if (!success)
		{
			wxMessageBox(_("Couldn't create cache directory."));
			return false;
		}
	}

#if 0
	// tsmosaic boulder.png 16 13 473000 479000 4425000 4434000
	MetersPerPixel = 16;
	TerrainZone = 13;
	TerrainEastingW = 473000;
	TerrainEastingE = 479000;
	TerrainNorthingS = 4425000;
	TerrainNorthingN = 4434000;
#endif
	MetersPerPixel = iMetersPerPixel;
	TerrainZone = iUTMZone;
	if (TerrainZone < 4 || TerrainZone > 19)
		return false;

	TerrainEastingW = area.left;
	TerrainEastingE = area.right;
	TerrainNorthingS = area.bottom;
	TerrainNorthingN = area.top;
	m_Extents = area;

	// Datum is always WGS84
	m_proj.SetWellKnownGeogCS("WGS84");
	m_proj.SetUTMZone(iUTMZone);

	if (MetersPerPixel == 1) TileScaleId = 10;
	if (MetersPerPixel == 2) TileScaleId = 11;
	if (MetersPerPixel == 4) TileScaleId = 12;
	if (MetersPerPixel == 8) TileScaleId = 13;
	if (MetersPerPixel == 16) TileScaleId = 14;
	if (MetersPerPixel == 32) TileScaleId = 15;
	if (MetersPerPixel == 64) TileScaleId = 16;

	MetersPerTile = PixelsPerTile * MetersPerPixel;

	if (!DownloadAllTiles())
		return false;

	int iXSize = (TerrainEastingE - TerrainEastingW) / MetersPerPixel;
	int iYSize = (TerrainNorthingN - TerrainNorthingS) / MetersPerPixel;
	vtBitmap *pBitmap = new vtBitmap;
	pBitmap->Allocate(iXSize, iYSize);
	SetupBitmapInfo(iXSize, iYSize);
	m_Bitmaps[0].m_pBitmap = pBitmap;

	vtDIB dib;
	dib.Create(iXSize, iYSize, 8, true);
	if (!MosaicAllTiles(dib))
		return false;

	if (!dib.WriteJPEG(filename, 99))
		return false;

	return true;
#else
	return false;
#endif
}

// Helper
int GetBitDepthUsingGDAL(const char *fname)
{
	// GDAL doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(fname);

	GDALDataset *pDataset = (GDALDataset *) GDALOpen(fname_local, GA_ReadOnly);
	if (pDataset == NULL)
		return -1;

	// Raster count should be 3 for colour images (assume RGB)
	int iRasterCount = pDataset->GetRasterCount();
	GDALRasterBand *pBand = pDataset->GetRasterBand(1);
	GDALDataType eType = pBand->GetRasterDataType();
	int bits = iRasterCount * GDALGetDataTypeSize(eType);
	GDALClose(pDataset);
	return bits;
}

bool vtImage::WriteGridOfTilePyramids(TilingOptions &opts, BuilderView *pView)
{
	VTLOG1("vtImage::WriteGridOfTilePyramids:\n");

	wxFrame *frame = NULL;

	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// Check that options are valid
	CheckCompressionMethod(opts);
	bool bJPEG = (opts.bUseTextureCompression && opts.eCompressionType == TC_JPEG);

#if USE_OPENGL
	if (opts.bUseTextureCompression && opts.eCompressionType == TC_OPENGL)
	{
		frame = new wxFrame;
		frame->Create(pView, -1, _T("Texture Compression OpenGL Context"),
			wxPoint(100,400), wxSize(280, 300), wxCAPTION | wxCLIP_CHILDREN);
		m_pCanvas = new ImageGLCanvas(frame);
	}
#endif

	// largest tile size
	int base_tilesize = opts.lod0size;

	DRECT area = m_Extents;
	DPoint2 tile_dim(area.Width()/opts.cols, area.Height()/opts.rows);
	DPoint2 cell_size = tile_dim / base_tilesize;

	vtString units = GetLinearUnitName(m_proj.GetUnits());
	units.MakeLower();
	int zone = m_proj.GetUTMZone();
	vtString crs;
	if (m_proj.IsGeographic())
		crs = "LL";
	else if (zone != 0)
		crs = "UTM";
	else
		crs = "Other";

	// Try to create directory to hold the tiles
	vtString dirname = opts.fname;
	RemoveFileExtensions(dirname);
	if (!vtCreateDir(dirname))
		return false;

	// make a note of which lods exist
	LODMap lod_existence_map(opts.cols, opts.rows);

	if (!m_Bitmaps[0].m_pBitmap)
	{
		IPoint2 size = m_Bitmaps[0].m_Size;

		// If we're dealing with an out-of-core image, consider how many rows
		//  we need to cache to avoid reading the file more than once during
		//  the generation of the tiles
		int need_cache_rows = (size.y + (opts.rows-1)) / opts.rows;
		int need_cache_bytes = need_cache_rows * size.x * 3;
		// add a little bit for rounding up
		need_cache_bytes += (need_cache_bytes / 10);

		// there's little point in shrinking the cache, so check existing size
		int existing = GDALGetCacheMax();
		if (need_cache_bytes > existing)
			GDALSetCacheMax(need_cache_bytes);
	}

	int i, j, lod;
	m_iTotal = opts.rows * opts.cols * opts.numlods;
	m_iCompleted = 0;

	clock_t tm1 = clock();
	bool bCancelled = false;
	for (j = opts.rows-1; j >= 0 && !bCancelled; j--)
	{
		for (i = 0; i < opts.cols && !bCancelled; i++)
		{
			// We might want to skip certain rows
			if (opts.iMinRow != -1 &&
				(i < opts.iMinCol || i > opts.iMaxCol ||
				 j < opts.iMinRow || j > opts.iMaxRow))
				continue;

			DRECT tile_area;
			tile_area.left = area.left + tile_dim.x * i;
			tile_area.right = area.left + tile_dim.x * (i+1);
			tile_area.bottom = area.bottom + tile_dim.y * j;
			tile_area.top = area.bottom + tile_dim.y * (j+1);

			int col = i;
			int row = opts.rows-1-j;

			// We know this tile will be included, so note the LODs present
			int base_tile_exponent = vt_log2(base_tilesize);
			lod_existence_map.set(i, j, base_tile_exponent, base_tile_exponent-(opts.numlods-1));

			for (lod = 0; lod < opts.numlods && !bCancelled; lod++)
			{
				if (!WriteTile(opts, pView, dirname, tile_area, tile_dim,
					col, row, lod))
					bCancelled = true;
			}
		}
	}
	if (bCancelled)
		wxMessageBox(_("Cancelled."));
	else
	{
		// Write .ini file
		WriteTilesetHeader(opts.fname, opts.cols, opts.rows,
			opts.lod0size, area, m_proj, INVALID_ELEVATION, INVALID_ELEVATION,
			&lod_existence_map, bJPEG);

		clock_t tm2 = clock();
		float elapsed = ((float)tm2 - tm1)/CLOCKS_PER_SEC;
		wxString str;
		str.Printf(_("Wrote %d tiles (%d cells) in %.1f seconds (%.2f seconds per cell)"),
			m_iTotal, (opts.rows * opts.cols), elapsed, elapsed/(opts.rows * opts.cols));
		wxMessageBox(str);
	}
	if (pView)
		pView->HideGridMarks();

#if USE_OPENGL
	if (frame)
		frame->Close();
	delete frame;
	m_pCanvas = NULL;
#endif

	return !bCancelled;
}

bool vtImage::WriteTile(const TilingOptions &opts, BuilderView *pView, vtString &dirname,
							 DRECT &tile_area, DPoint2 &tile_dim, int col, int row, int lod)
{
	int tilesize = opts.lod0size >> lod;

	// safety check: don't create LODs which are too small
	if (tilesize < 2)
		return true;

	// Images are written as PixelIsPoint, and each LOD has
	// its own sample spacing
	DPoint2 spacing = tile_dim / (tilesize-1);

	// Write DB file (libMini's databuf format)
	bool bJPEG = (opts.bUseTextureCompression && opts.eCompressionType == TC_JPEG);
	vtString fname = MakeFilenameDB(dirname, col, row, lod);

	// make a message for the progress dialog
	wxString msg;
	msg.Printf(_("Writing tile '%hs', size %dx%d"),
		(const char *)fname, tilesize, tilesize);
	if (UpdateProgressDialog(m_iCompleted*99/m_iTotal, msg))
	{
		// User cancelled.
		return false;
	}

	// also draw our progress in the main view
	if (pView)
		pView->ShowGridMarks(m_Extents, opts.cols, opts.rows, col, opts.rows-1-row);

	// First, fill a buffer with the uncompressed texels
	uchar *rgb_bytes = (uchar *) malloc(tilesize * tilesize * 3);
	int cb = 0;	// count bytes

	DPoint2 p;
	int x, y;
	RGBi rgb;

	// Get ready to multisample
	DLine2 offsets;
	int iNSampling = g_Options.GetValueInt(TAG_SAMPLING_N);
	MakeSampleOffsets(spacing, iNSampling, offsets);
	double dRes = (spacing.x+spacing.y)/2;

	for (y = tilesize-1; y >= 0; y--)
	{
		p.y = tile_area.bottom + y * spacing.y;
		for (x = 0; x < tilesize; x++)
		{
			p.x = tile_area.left + x * spacing.x;

			GetMultiSample(p, offsets, rgb, dRes);
#if 0 // LOD Stripes
			// For testing, add stripes to indicate LOD
			if (lod == 3 && x == y) rgb.Set(255,0,0);

			if (lod == 2 && (
				x == tilesize-y ||
				x == y+tilesize/2 ||
				x == y-tilesize/2)) rgb.Set(0,255,0);

			if (lod == 1 && (x%16)==0) rgb.Set(0,0,90);

			if (lod == 0 && (y%8)==0) rgb.Set(90,0,90);
#endif
			rgb_bytes[cb++] = rgb.r;
			rgb_bytes[cb++] = rgb.g;
			rgb_bytes[cb++] = rgb.b;
		}
	}
	int iUncompressedSize = cb;

	vtMiniDatabuf output_buf;
	output_buf.xsize = tilesize;
	output_buf.ysize = tilesize;
	output_buf.zsize = 1;
	output_buf.tsteps = 1;
	output_buf.SetBounds(m_proj, tile_area);

	// Write and optionally compress the image
	WriteMiniImage(fname, opts, rgb_bytes, output_buf,
		iUncompressedSize, m_pCanvas);

	// Free the uncompressed image
	free(rgb_bytes);

	m_iCompleted++;

	return true;
}

void SampleMipLevel(vtBitmap *bigger, vtBitmap *smaller)
{
	int xsize = bigger->GetWidth();
	int ysize = bigger->GetHeight();

	int xsmall = xsize / 2;
	int ysmall = ysize / 2;

	RGBi rgb, sum;

	for (int y = 0; y < ysmall; y++)
	{
		if ((y%32)==0)
			progress_callback(y * 99 / ysmall);

		for (int x = 0; x < xsmall; x++)
		{
			sum.Set(0,0,0);

			bigger->GetPixel24(x*2, y*2, rgb);
			sum += rgb;
			bigger->GetPixel24(x*2+1, y*2, rgb);
			sum += rgb;
			bigger->GetPixel24(x*2, y*2+1, rgb);
			sum += rgb;
			bigger->GetPixel24(x*2, y*2, rgb);
			sum += rgb;

			smaller->SetPixel24(x, y, sum/4);
		}
	}
}
