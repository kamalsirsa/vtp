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
	rect = m_Area;
	return true;
}

void vtImageLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	if (NULL == m_pImage)
		return;
	if (!m_pImage->Ok())
		return;

	wxRect screenrect = pView->WorldToCanvas(m_Area);
	wxRect destRect = screenrect;
	wxRect srcRect(0, 0, m_pImage->GetWidth(), m_pImage->GetHeight());

	double ratio_x = (float) srcRect.GetWidth() / destRect.GetWidth();
	double ratio_y = (float) srcRect.GetHeight() / destRect.GetHeight();

	// scale and draw the bitmap
	// must use SetUserScale since StretchBlt is not supported
	double scale_x = 1.0/ratio_x;
	double scale_y = 1.0/ratio_y;
	pDC->SetUserScale(scale_x, scale_y);
#if WIN32
	::SetStretchBltMode((HDC) (pDC->GetHDC()), HALFTONE );
#endif
	pDC->DrawBitmap(*m_pBitmap, (int) (destRect.x/scale_x),
		(int) (destRect.y/scale_y), false);

	// restore
	pDC->SetUserScale(1.0, 1.0);
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

bool vtImageLayer::LoadFromGDAL()
{
	GDALDataset *pDataset = NULL;
	OGRErr err;
	const char *pProjectionString;
	double affineTransform[6];
	OGRSpatialReference SpatialReference;
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
	double TLGeoX, TLGeoY, BRGeoX, BRGeoY;

	try
	{
		GDALAllRegister();

		pDataset = (GDALDataset *) GDALOpen( m_strFilename, GA_ReadOnly );
		if(pDataset == NULL )
			throw "Couldn't open that file.";

		iPixelWidth = pDataset->GetRasterXSize();
		iPixelHeight = pDataset->GetRasterYSize();

		if (NULL == (pProjectionString = pDataset->GetProjectionRef()))
			throw "Unknown coordinate system.";

		err = SpatialReference.importFromWkt((char**)&pProjectionString);
		if (err != OGRERR_NONE)
			throw "Unknown coordinate system.";

		if (CE_None != pDataset->GetGeoTransform(affineTransform))
			throw "Dataset does not contain a valid affine transform.";

		if (SpatialReference.IsGeographic())
		{
			// More work needed...
			// Try to convert to projected
			OGRCoordinateTransformation *pCoordTransform;
			OGRSpatialReference NewSpatialReference;
			double Lat, Long;

			NewSpatialReference.SetWellKnownGeogCS("WGS84");

			// Get GEO coords of centre of image
			Long = affineTransform[0] + affineTransform[1] * iPixelWidth / 2 + affineTransform[2] * iPixelHeight / 2;
			Lat = affineTransform[3] + affineTransform[4] * iPixelWidth / 2 + affineTransform[5] * iPixelHeight / 2;

			// Set UTM projection
			NewSpatialReference.SetUTM((int)(Long + 180)/6 + 1, Lat > 0 ? 1 : 0);

			// Convert top left and bottom right to UTM
			if (NULL == (pCoordTransform = OGRCreateCoordinateTransformation(&SpatialReference, &NewSpatialReference)))
				throw "Couldn't create coordinate conversion.";
			// Calculate image size in metres
			TLGeoX = affineTransform[0];
			TLGeoY = affineTransform[3];
			BRGeoX = affineTransform[0] + affineTransform[1] * iPixelWidth + affineTransform[2] * iPixelHeight;
			BRGeoY = affineTransform[3] + affineTransform[4] * iPixelWidth + affineTransform[5] * iPixelHeight;
			if (!pCoordTransform->Transform(1, &TLGeoX, &TLGeoY))
				throw "Couldn't transform coordinates.";
			if (!pCoordTransform->Transform(1, &BRGeoX, &BRGeoY))
				throw "Couldn't transform coordinates.";
			m_Area.top = TLGeoY;
			m_Area.bottom = BRGeoY;
			m_Area.left = TLGeoX;
			m_Area.right = BRGeoX;
			m_Proj.SetSpatialReference(&NewSpatialReference);
		}
		else
		{
			linearConversionFactor = SpatialReference.GetLinearUnits();

			// Compute extent using the top left and bottom right image co-ordinates
			m_Area.left = (affineTransform[0] /*+ affineTransform[1] * 0*/ + affineTransform[2] * iPixelHeight) * linearConversionFactor;
			m_Area.right = (affineTransform[0] + affineTransform[1] * iPixelWidth /*+ affineTransform[2] * 0*/) * linearConversionFactor;
			m_Area.top = (affineTransform[3] /*+ affineTransform[4] * 0*/ + affineTransform[5] * iPixelHeight) * linearConversionFactor;
			m_Area.bottom = (affineTransform[3] + affineTransform[4] * iPixelWidth /*+ affineTransform[5] * 0*/) * linearConversionFactor;
			m_Proj.SetSpatialReference(&SpatialReference);
		}

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
//							*(pData + (y * iPixelWidth) + x) = Ent.c1;
//							*(pData + (y * iPixelWidth) + x + 1) = Ent.c2;
//							*(pData + (y * iPixelWidth) + x + 2) = Ent.c3;
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
//							*(pData + (y * iPixelWidth) + x) = pRedline[iY * xBlockSize + iX];
//							*(pData + (y * iPixelWidth) + x + 1) = pGreenline[iY * xBlockSize + iX];
//							*(pData + (y * iPixelWidth) + x + 2) = pBlueline[iY * xBlockSize + iX];
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
		wxMessageBox(msg);
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
			str.Printf("Couldn't create bitmap of size %d x %d", iPixelWidth, iPixelHeight);
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

