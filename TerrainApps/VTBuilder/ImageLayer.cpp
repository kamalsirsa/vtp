//
// ImageLayer.cpp
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ogr_spatialref.h"
#include "gdal_priv.h"
#include "ScaledView.h"
#include "ImageLayer.h"
#include "Helper.h"

vtImageLayer::vtImageLayer() : vtLayer(LT_IMAGE)
{
	m_pImage = NULL;
	m_pBitmap = NULL;
}

vtImageLayer::~vtImageLayer()
{
	if (NULL != m_pImage)
		delete m_pImage;
	if (NULL != m_pBitmap)
		delete m_pBitmap;
}

bool vtImageLayer::GetExtent(DRECT &rect)
{
	rect = m_Extents;
	return true;
}

bool vtImageLayer::GetAreaExtent(DRECT &rect)
{
	DPoint2 pixel_size(m_Extents.Width() / (m_pImage->GetWidth() - 1),
		m_Extents.Height() / (m_pImage->GetHeight() - 1));

	rect.left =   m_Extents.left - (pixel_size.x / 2.0f);
	rect.top =    m_Extents.top + (pixel_size.y / 2.0f);
	rect.right =  m_Extents.right + (pixel_size.x / 2.0f);
	rect.bottom = m_Extents.bottom - (pixel_size.y / 2.0f);
	return true;
}

void vtImageLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	if (NULL == m_pImage)
		return;
	if (!m_pImage->Ok())
		return;

	DRECT area;
	GetAreaExtent(area);

	wxRect screenrect = pView->WorldToCanvas(area);
	wxRect destRect = screenrect;
	wxRect srcRect(0, 0, m_pImage->GetWidth(), m_pImage->GetHeight());

/*	wxPen yellow(wxColor(255,255,0), 1, wxSOLID);
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(yellow);
	pDC->DrawRectangle(screenrect.x, screenrect.y, screenrect.width, screenrect.height); */

#if WIN32
	::SetStretchBltMode((HDC) (pDC->GetHDC()), HALFTONE );

	wxDC2 *pDC2 = (wxDC2 *) pDC;
	pDC2->StretchBlit(*m_pBitmap, destRect.x, destRect.y,
		destRect.width, destRect.height);
#else
	// scale and draw the bitmap
	// must use SetUserScale since StretchBlt is not available
	double ratio_x = (double) srcRect.GetWidth() / destRect.GetWidth();
	double ratio_y = (double) srcRect.GetHeight() / destRect.GetHeight();

	double scale_x = 1.0/ratio_x;
	double scale_y = 1.0/ratio_y;
	pDC->SetUserScale(scale_x, scale_y);
	pDC->DrawBitmap(*m_pBitmap, (int) (destRect.x/scale_x),
		(int) (destRect.y/scale_y), false);

	// restore
	pDC->SetUserScale(1.0, 1.0);
#endif
}

bool vtImageLayer::ConvertProjection(vtProjection &proj)
{
	return false;
}

bool vtImageLayer::OnSave()
{
	return false;
}

bool vtImageLayer::OnLoad()
{
	return LoadFromGDAL();
}

bool vtImageLayer::AppendDataFrom(vtLayer *pL)
{
	return false;
}

void vtImageLayer::GetProjection(vtProjection &proj)
{
	proj = m_Proj;
}

void vtImageLayer::SetProjection(const vtProjection &proj)
{
	m_Proj = proj;
}

DPoint2 vtImageLayer::GetSpacing()
{
	if (!m_pImage)
		return DPoint2(0,0);

	return DPoint2(m_Extents.Width() / (m_pImage->GetWidth() - 1),
		m_Extents.Height() / (m_pImage->GetHeight() - 1));
}

#include "vtdata/vtDIB.h"

bool vtImageLayer::LoadFromGDAL()
{
	GDALDataset *pDataset = NULL;
	OGRErr err;
	const char *pProjectionString;
	double affineTransform[6];
	vtProjection SpatialReference;
	double linearConversionFactor;
	GDALRasterBand *pBand;
	GDALColorTable *pTable;
	GDALColorEntry Ent;
	int i;
	int iRasterCount;
	bool bRet = true;
	unsigned char *pData;
	int xBlockSize, yBlockSize;
	int nxBlocks, nyBlocks;
	int ixBlock, iyBlock;
	int nxValid, nyValid;
	CPLErr Err;
	char *pScanline = NULL;
	char *pRedline = NULL;
	char *pGreenline = NULL;
	char *pBlueline = NULL;
	GDALRasterBand *pRed = NULL;
	GDALRasterBand *pGreen = NULL;
	GDALRasterBand *pBlue = NULL;
	int x, y;
	int iPixelHeight;
	int iPixelWidth;

	try
	{
		GDALAllRegister();

		pDataset = (GDALDataset *) GDALOpen(m_strFilename.mb_str(), GA_ReadOnly);
		if(pDataset == NULL )
			throw "Couldn't open that file.";

		iPixelWidth = pDataset->GetRasterXSize();
		iPixelHeight = pDataset->GetRasterYSize();

		if (NULL == (pProjectionString = pDataset->GetProjectionRef()))
		{
			// check for existence of .prj file
			bool bSuccess = SpatialReference.ReadProjFile(m_strFilename.mb_str());

			if (!bSuccess)
				throw "Unknown coordinate system.";
		}

		err = SpatialReference.importFromWkt((char**)&pProjectionString);
		if (err != OGRERR_NONE)
		{
			// check for existence of .prj file
			bool bSuccess = SpatialReference.ReadProjFile(m_strFilename.mb_str());

			if (!bSuccess)
				throw "Unknown coordinate system.";
		}

		if (CE_None != pDataset->GetGeoTransform(affineTransform))
			throw "Dataset does not contain a valid affine transform.";

		if (SpatialReference.IsGeographic())
		{
			m_Extents.left = affineTransform[0];
			m_Extents.right = m_Extents.left + affineTransform[1] * (iPixelWidth-1);
			m_Extents.top = affineTransform[3];
			m_Extents.bottom = m_Extents.top + affineTransform[5] * (iPixelHeight-1);
		}
		else
		{
			linearConversionFactor = SpatialReference.GetLinearUnits();

			// Compute extent using the top left and bottom right image co-ordinates
			m_Extents.left = (affineTransform[0] /*+ affineTransform[1] * 0*/ + affineTransform[2] * iPixelHeight) * linearConversionFactor;
			m_Extents.right = (affineTransform[0] + affineTransform[1] * iPixelWidth /*+ affineTransform[2] * 0*/) * linearConversionFactor;
			m_Extents.top = (affineTransform[3] /*+ affineTransform[4] * 0*/ + affineTransform[5] * iPixelHeight) * linearConversionFactor;
			m_Extents.bottom = (affineTransform[3] + affineTransform[4] * iPixelWidth /*+ affineTransform[5] * 0*/) * linearConversionFactor;
		}
		m_Proj.SetSpatialReference(&SpatialReference);

		// Set up bitmap
		if (NULL == (m_pImage = new wxImage(iPixelWidth, iPixelHeight)))
			throw "Couldn't create image.";
		
		pData = m_pImage->GetData();

		// Raster count should be 3 for colour images (assume RGB)
		iRasterCount = pDataset->GetRasterCount();
		if (iRasterCount == 1)
		{
			pBand = pDataset->GetRasterBand(1);
			// Check data type - it's either integer or float
			if (GDT_Byte != pBand->GetRasterDataType())
				throw "Raster is not of type byte.";
			if (GCI_PaletteIndex != pBand->GetColorInterpretation())
				throw "Couldn't get palette.";
			if (NULL == (pTable = pBand->GetColorTable()))
				throw "Couldn't get color table.";

			pBand->FlushBlock(0, 0);

			pBand->GetBlockSize(&xBlockSize, &yBlockSize);
			nxBlocks = (iPixelWidth + xBlockSize - 1) / xBlockSize;
			nyBlocks = (iPixelHeight + yBlockSize - 1) / yBlockSize;
			if (NULL == (pScanline = new char[xBlockSize * yBlockSize]))
				throw "Couldnt allocate scan line.";
			// Read the data
			// Convert to rgb and write to image
			for( iyBlock = 0; iyBlock < nyBlocks; iyBlock++ )
			{
				progress_callback(iyBlock * 100 / nyBlocks);
				for( ixBlock = 0; ixBlock < nxBlocks; ixBlock++ )
				{
					Err = pBand->ReadBlock(ixBlock, iyBlock, pScanline);
					if (Err != CE_None)
						return false;

					// Compute the portion of the block that is valid
					// for partial edge blocks.
					if ((ixBlock+1) * xBlockSize > iPixelWidth)
						nxValid = iPixelWidth - ixBlock * xBlockSize;
					else
						nxValid = xBlockSize;

					if( (iyBlock+1) * yBlockSize > iPixelHeight)
						nyValid = iPixelHeight - iyBlock * yBlockSize;
					else
						nyValid = yBlockSize;

					for( int iY = 0; iY < nyValid; iY++ )
					{
						y = (iyBlock * yBlockSize + iY);
						for( int iX = 0; iX < nxValid; iX++ )
						{
							x = (ixBlock * xBlockSize) + iX;
							pTable->GetColorEntryAsRGB(pScanline[iY * xBlockSize + iX], &Ent);
							m_pImage->SetRGB(x, y, Ent.c1, Ent.c2, Ent.c3);
						}
					}
				}
			}
		}
		else if (iRasterCount == 3)
		{
			for (i = 1; i <= 3; i++)
			{
				pBand = pDataset->GetRasterBand(i);
				// Check data type - it's either integer or float
				if (GDT_Byte != pBand->GetRasterDataType())
					throw "Three rasters, but not of type byte.";
				// I assume that the bands are in order RGB
				// I know "could do better"... but
				switch(pBand->GetColorInterpretation())
				{
				case GCI_RedBand:
					pRed = pBand;
					break;
				case GCI_GreenBand:
					pGreen = pBand;
					break;
				case GCI_BlueBand:
					pBlue = pBand;
					break;
				}
			}
			if ((NULL == pRed) || (NULL == pGreen) || (NULL == pBlue))
				throw "Couldn't find bands for Red, Green, Blue.";

			pRed->GetBlockSize(&xBlockSize, &yBlockSize);
			nxBlocks = (iPixelWidth + xBlockSize - 1) / xBlockSize;
			nyBlocks = (iPixelHeight + yBlockSize - 1) / yBlockSize;
			pRed->FlushBlock(0, 0);
			pGreen->FlushBlock(0, 0);
			pBlue->FlushBlock(0, 0);
			pRedline = new char[xBlockSize * yBlockSize];
			pGreenline = new char[xBlockSize * yBlockSize];
			pBlueline = new char[xBlockSize * yBlockSize];

			for( iyBlock = 0; iyBlock < nyBlocks; iyBlock++ )
			{
				progress_callback(iyBlock * 100 / nyBlocks);
				for( ixBlock = 0; ixBlock < nxBlocks; ixBlock++ )
				{
					Err = pRed->ReadBlock(ixBlock, iyBlock, pRedline);
					if (Err != CE_None)
						return false;
					Err = pGreen->ReadBlock(ixBlock, iyBlock, pGreenline);
					if (Err != CE_None)
						return false;
					Err = pBlue->ReadBlock(ixBlock, iyBlock, pBlueline);
					if (Err != CE_None)
						return false;

					// Compute the portion of the block that is valid
					// for partial edge blocks.
					if ((ixBlock+1) * xBlockSize > iPixelWidth)
						nxValid = iPixelWidth - ixBlock * xBlockSize;
					else
						nxValid = xBlockSize;

					if( (iyBlock+1) * yBlockSize > iPixelHeight)
						nyValid = iPixelHeight - iyBlock * yBlockSize;
					else
						nyValid = yBlockSize;

					for( int iY = 0; iY < nyValid; iY++ )
					{
						y = (iyBlock * yBlockSize + iY);
						for( int iX = 0; iX < nxValid; iX++ )
						{
							x = (ixBlock * xBlockSize) + iX;
							m_pImage->SetRGB(x, y,
								pRedline[iY * xBlockSize + iX],
								pGreenline[iY * xBlockSize + iX],
								pBlueline[iY * xBlockSize + iX]);
						}
					}
				}
			}
		}
		else
			throw "Image does not have 1 or 3 bands.";
	}
	
	catch (const char *msg)
	{
		wxString2 str = msg;
		wxMessageBox(str);
		bRet = false;
	}

	if (NULL != pDataset)
		GDALClose(pDataset);
	if (NULL != pScanline)
		delete pScanline;
	if (NULL != pRedline)
		delete pRedline;
	if (NULL != pGreenline)
		delete pGreenline;
	if (NULL != pBlueline)
		delete pBlueline;

	if (bRet == true)
	{
		m_pBitmap = new wxBitmap(m_pImage->ConvertToBitmap());
		if (!m_pBitmap || (m_pBitmap && !m_pBitmap->Ok()))
		{
			wxString str;
			str.Printf(_T("Couldn't create bitmap of size %d x %d"), iPixelWidth, iPixelHeight);
			wxMessageBox(str);
			bRet = false;
			delete m_pBitmap;
			m_pBitmap = NULL;
		}
	}
	if (bRet == false)
	{
		// Tidy up
		if (NULL != m_pImage)
		{
			delete m_pImage;
			m_pImage = NULL;
		}
	}
	return bRet;
}

