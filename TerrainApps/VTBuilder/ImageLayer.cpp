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

#if ROGER
#include "gdalwarper.h"
#endif
#include "ogr_spatialref.h"
#include "gdal_priv.h"
#include "ScaledView.h"
#include "ImageLayer.h"
#include "Helper.h"
#include "Projection2Dlg.h"
#include "Frame.h"

vtImageLayer::vtImageLayer() : vtLayer(LT_IMAGE)
{
	SetDefaults();
}

vtImageLayer::vtImageLayer(const DRECT &area, int xsize, int ysize,
						   const vtProjection &proj) : vtLayer(LT_IMAGE)
{
	SetDefaults();
	m_Extents = area;
	m_iXSize = xsize;
	m_iYSize = ysize;
	m_proj = proj;

	// yes, we could use some error-checking here
	m_pImage = new wxImage(m_iXSize, m_iYSize);
	m_pBitmap = new wxBitmap(m_pImage);
}

vtImageLayer::~vtImageLayer()
{
	if (NULL != m_pImage)
		delete m_pImage;
	if (NULL != m_pBitmap)
		delete m_pBitmap;
	CleanupGDALUsage();
}

void vtImageLayer::SetDefaults()
{
	m_pImage = NULL;
	m_pBitmap = NULL;
	m_bInMemory = false;
	pScanline = NULL;
	pRedline = NULL;
	pGreenline = NULL;
	pBlueline = NULL;
	pDataset = NULL;
	pTable = NULL;
	m_iXSize = 0;
	m_iYSize = 0;
	iRasterCount = 0;
	m_use_next = 0;
}

bool vtImageLayer::GetExtent(DRECT &rect)
{
	rect = m_Extents;
	return true;
}

/**
 * The standard extents of an image are the min and max of its data points,
 * but the area covered by the image is actually half a pixel larger in
 * each direction.
 */
bool vtImageLayer::GetAreaExtent(DRECT &rect)
{
	DPoint2 pixel_size(m_Extents.Width() / (m_iXSize - 1),
		m_Extents.Height() / (m_iYSize - 1));

	rect.left =   m_Extents.left - (pixel_size.x / 2.0f);
	rect.top =    m_Extents.top + (pixel_size.y / 2.0f);
	rect.right =  m_Extents.right + (pixel_size.x / 2.0f);
	rect.bottom = m_Extents.bottom - (pixel_size.y / 2.0f);
	return true;
}

void vtImageLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	bool bDrawImage = true;
	if (m_pImage == NULL)
		bDrawImage = false;
	else if (!m_pImage->Ok())
		bDrawImage = false;

	DRECT area;
	GetAreaExtent(area);

	wxRect screenrect = pView->WorldToCanvas(area);
	wxRect destRect = screenrect;
	wxRect srcRect(0, 0, m_iXSize, m_iYSize);

	if (bDrawImage)
	{
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
	else
	{
		wxPen yellow(wxColor(255,255,0), 1, wxSOLID);
		pDC->SetLogicalFunction(wxCOPY);
		pDC->SetPen(yellow);

		DrawRectangle(pDC, screenrect);
	}
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
	proj = m_proj;
}

void vtImageLayer::SetProjection(const vtProjection &proj)
{
	m_proj = proj;
}

void vtImageLayer::GetPropertyText(wxString &str)
{
	str.Printf(_T("Dimensions %d by %d pixels"), m_iXSize, m_iYSize);
}

DPoint2 vtImageLayer::GetSpacing()
{
	return DPoint2(m_Extents.Width() / (m_iXSize - 1),
		m_Extents.Height() / (m_iYSize - 1));
}

bool vtImageLayer::GetFilteredColor(double x, double y, RGBi &rgb)
{
	// could speed this up by keeping these values around
	DPoint2 spacing = GetSpacing();
	DPoint2 half = spacing/2;

	double u = (x - m_Extents.left + half.x) / spacing.x;
	int ix = (int) u;
	if (ix < 0 || ix >= m_iXSize)
		return false;

	double v = (m_Extents.top - y + half.y) / spacing.y;
	int iy = (int) v;
	if (iy < 0 || iy >= m_iYSize)
		return false;

	if (!m_pImage)
	{
		// support for out-of-memory image here
		RGBi *data = GetScanlineFromBuffer(iy);
		rgb = data[ix];
	}
	else
	{
		// TODO: real filtering (interpolation)
		// for now, just grab closest pixel
		rgb.r = m_pImage->GetRed(ix, iy);
		rgb.g = m_pImage->GetGreen(ix, iy);
		rgb.b = m_pImage->GetBlue(ix, iy);
	}
	return true;
}

bool vtImageLayer::SaveToFile(const char *fname)
{
	if (!m_pImage)
		return false;

	// Save with GDAL
	GDALDriverManager *pManager = GetGDALDriverManager();
	if (!pManager)
		return false;

	// JPEG: Error 6: GDALDriver::Create() ... no create method implemented for this format.
//	GDALDriver *pDriver = pManager->GetDriverByName("JPEG");
	GDALDriver *pDriver = pManager->GetDriverByName("GTiff");
	if (!pDriver)
		return false;

	char ** papszParmList = NULL;
	DPoint2 spacing = GetSpacing();

	GDALDataset *pDataset;
	pDataset = pDriver->Create(fname, m_iXSize, m_iYSize, 3, GDT_Byte, papszParmList );
	if (!pDataset)
		return false;

	double adfGeoTransform[6] = { m_Extents.left, spacing.x, 0, m_Extents.top, 0, -spacing.y };
	pDataset->SetGeoTransform(adfGeoTransform);

	GByte *raster = new GByte[m_iXSize*m_iYSize];

	char *pszSRS_WKT = NULL;
	m_proj.exportToWkt( &pszSRS_WKT );
	pDataset->SetProjection(pszSRS_WKT);
	CPLFree( pszSRS_WKT );

	GDALRasterBand *pBand;
	int i, x, y;
	for (i = 1; i <= 3; i++)
	{
		pBand = pDataset->GetRasterBand(i);

		for (x = 0; x < m_iXSize; x++)
		{
			for (y = 0; y < m_iYSize; y++)
			{
				if (i == 1)
					raster[y*m_iXSize + x] = m_pImage->GetRed(x, y);
				if (i == 2)
					raster[y*m_iXSize + x] = m_pImage->GetGreen(x, y);
				if (i == 3)
					raster[y*m_iXSize + x] = m_pImage->GetBlue(x, y);
			}
		}
		pBand->RasterIO( GF_Write, 0, 0, m_iXSize, m_iYSize, 
			raster, m_iXSize, m_iYSize, GDT_Byte, 0, 0 );
	}
	delete raster;
	GDALClose(pDataset);

	return true;
}


static int WarpProgress(double dfComplete, const char *pszMessage, void *pProgressArg)
{
	int amount = (int)(99.0 * dfComplete);
	return !UpdateProgressDialog(amount, wxString(pszMessage));
}


bool vtImageLayer::LoadFromGDAL()
{
	OGRErr err;
	const char *pProjectionString;
	double affineTransform[6];
	double linearConversionFactor;
	int i;
	bool bRet = true, bBitmap = true;

	g_GDALWrapper.RequestGDALFormats();

	pDataset = NULL;
	pScanline = NULL;
	pRedline = NULL;
	pGreenline = NULL;
	pBlueline = NULL;
	pBand = NULL;
	pRed = NULL;
	pGreen = NULL;
	pBlue = NULL;

	try
	{
		pDataset = (GDALDataset *) GDALOpen(m_strFilename.mb_str(), GA_ReadOnly);
		if(pDataset == NULL )
			throw "Couldn't open that file.";

		m_iXSize = pDataset->GetRasterXSize();
		m_iYSize = pDataset->GetRasterYSize();

		if (CE_None != pDataset->GetGeoTransform(affineTransform))
			throw "Dataset does not contain a valid affine transform.";

		bool bHaveProj = false;
		pProjectionString = pDataset->GetProjectionRef();
		if (pProjectionString)
		{
			err = m_proj.importFromWkt((char**)&pProjectionString);
			if (err == OGRERR_NONE)
				bHaveProj = true;
		}
		if (!bHaveProj)
		{
			// check for existence of .prj file
			bool bSuccess = m_proj.ReadProjFile(m_strFilename.mb_str());
			if (bSuccess)
				bHaveProj = true;
		}
		// if we still don't have it
		if (!bHaveProj)
		{
			wxString2 msg = "File lacks a projection.  Would you like to specify one?\n"
				"Yes - specify projection\n"
				"No - use current projection\n";
			int res = wxMessageBox(msg, _T("Image Import"), wxYES_NO | wxCANCEL);
			if (res == wxYES)
			{
				GetMainFrame()->GetProjection(m_proj);
				Projection2Dlg dlg(NULL, -1, _T("Please indicate projection"));
				dlg.SetProjection(m_proj);

				if (dlg.ShowModal() == wxID_CANCEL)
					throw "Import Cancelled";
				dlg.GetProjection(m_proj);
			}
			if (res == wxNO)
			{
				GetMainFrame()->GetProjection(m_proj);
			}
			if (res == wxCANCEL)
				throw "Import Cancelled.";
		}

#if ROGER
		if ((fabs(affineTransform[1]) != fabs(affineTransform[5])) || (affineTransform[2] != 0.0) || (affineTransform[4] != 0.0))
		{
			if (wxYES == wxMessageBox(_T("Image pixels are not square or image is rotated\nDo you want to create a realigned image?"),
										wxMessageBoxCaptionStr, wxYES_NO|wxCENTRE))
			{
				wxFileDialog saveRealigned(NULL, _T("Save realigned image as"), _T(""), _T(""), _T("GeoTiff files (*.tif)|*.tif"), wxSAVE|wxOVERWRITE_PROMPT);

				if (wxID_OK == saveRealigned.ShowModal())
				{
					GDALDriverH hDriver;
					void *hTransformArg;
					char *pWkt;
					double adfDstGeoTransform[6];
					int nPixels=0, nLines=0;
					GDALDataset *pDstDataset;
					GDALWarpOptions *psWarpOptions;
					GDALWarpOperation oOperation;

					if (OGRERR_NONE != m_proj.exportToWkt(&pWkt))
						throw "Unable to get projection string";

					if (NULL == (hDriver = GDALGetDriverByName("GTiff")))
						throw "unable to get GTiff driver handle";

					if (NULL == (hTransformArg = GDALCreateGenImgProjTransformer((GDALDatasetH)pDataset, pWkt, NULL, pWkt, FALSE, 0, 1 )))
						throw "unable to create transform argument";

					if (CE_None != GDALSuggestedWarpOutput( (GDALDatasetH)pDataset, GDALGenImgProjTransform, hTransformArg, adfDstGeoTransform, &nPixels, &nLines))
						throw "Unable to get suggested warp output";

					GDALDestroyGenImgProjTransformer(hTransformArg);

					if (NULL == (pDstDataset = (GDALDataset *)GDALCreate(hDriver,
										saveRealigned.GetPath(),
										nPixels,
										nLines,
										pDataset->GetRasterCount(),
										pDataset->GetRasterBand(1)->GetRasterDataType(),
										NULL)))
						throw "Unable to create destination dataset";

					pDstDataset->SetProjection(pWkt);
					pDstDataset->SetGeoTransform(adfDstGeoTransform);

					GDALColorTable *pCT;

					pCT = pDataset->GetRasterBand(1)->GetColorTable();
					if (pCT != NULL)
						pDstDataset->GetRasterBand(1)->SetColorTable(pCT); 

					if (NULL == (psWarpOptions = GDALCreateWarpOptions()))
						throw "Unable to create WarpOptions";

					psWarpOptions->hSrcDS = (GDALDatasetH)pDataset;
					psWarpOptions->hDstDS = (GDALDatasetH)pDstDataset;

					psWarpOptions->nBandCount = 0;

					// Assume a non cancellable progress dialog is already open
					// more work needed to clean up after cancel
					psWarpOptions->pfnProgress = WarpProgress;   

					if (NULL == (psWarpOptions->pTransformerArg = GDALCreateGenImgProjTransformer( (GDALDatasetH)pDataset,
																					pWkt, 
																					(GDALDatasetH)pDstDataset,
																					pWkt, 
																					FALSE, 0.0, 1 )))
						throw "Unable to create GenImgProTransformer";

					psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

					// Initialize and execute the warp operation. 

					oOperation.Initialize(psWarpOptions);
					
					if (CE_None != oOperation.ChunkAndWarpImage( 0, 0, pDstDataset->GetRasterXSize(), pDstDataset->GetRasterYSize()))
						throw "Warp operation failed";

					GDALDestroyGenImgProjTransformer(psWarpOptions->pTransformerArg);
					GDALDestroyWarpOptions(psWarpOptions);

					// Clean up
					OGRFree(pWkt);
					// rror handling, close original, setup new
					m_strFilename = saveRealigned.GetPath();
					GDALClose((GDALDatasetH)pDataset);

#if 0				// Roger says this is a workaround for GDAL oddness
					pDataset = pDstDataset;
#else
					GDALClose((GDALDatasetH)pDstDataset);
					pDataset = (GDALDataset *) GDALOpen(m_strFilename.mb_str(), GA_ReadOnly);
#endif
					m_iXSize = pDataset->GetRasterXSize();
					m_iYSize = pDataset->GetRasterYSize();
					if (CE_None != pDataset->GetGeoTransform(affineTransform))
						throw "New dataset does not contain a valid affine transform.";
				}
			}
		}
#endif

		if (m_proj.IsGeographic())
		{
			m_Extents.left = affineTransform[0];
			m_Extents.right = m_Extents.left + affineTransform[1] * (m_iXSize-1);
			m_Extents.top = affineTransform[3];
			m_Extents.bottom = m_Extents.top + affineTransform[5] * (m_iYSize-1);
		}
		else
		{
			linearConversionFactor = m_proj.GetLinearUnits();

			// Compute extent using the top left and bottom right image co-ordinates
			m_Extents.left = affineTransform[0]  * linearConversionFactor;
			m_Extents.top = affineTransform[3] * linearConversionFactor;
			m_Extents.right = (affineTransform[0] + affineTransform[1] * m_iXSize + affineTransform[2] * m_iYSize) * linearConversionFactor;
			m_Extents.bottom = (affineTransform[3] + affineTransform[4] * m_iXSize + affineTransform[5] * m_iYSize) * linearConversionFactor;
		}

		// Prepare scanline buffers
		for (i = 0; i < BUF_SCANLINES; i++)
		{
			m_row[i].m_data = new RGBi[m_iXSize];
			m_row[i].m_y = -1;
		}
		m_use_next = 0;

		// Raster count should be 3 for colour images (assume RGB)
		iRasterCount = pDataset->GetRasterCount();

		if (iRasterCount != 1 && iRasterCount != 3)
			throw "Does not have 1 or 3 bands.";

		if (iRasterCount == 1)
		{
			pBand = pDataset->GetRasterBand(1);
			// Check data type - it's either integer or float
			if (GDT_Byte != pBand->GetRasterDataType())
				throw "Raster is not of type byte.";
			GDALColorInterp ci = pBand->GetColorInterpretation();
			if (ci == GCI_PaletteIndex)
			{
				if (NULL == (pTable = pBand->GetColorTable()))
					throw "Couldn't get color table.";
			}
			else if (ci == GCI_GrayIndex)
			{
				// we will assume 0-255 is black to white
			}
			else
				throw "Unsupported color interpretation.";

			pBand->GetBlockSize(&xBlockSize, &yBlockSize);
			nxBlocks = (m_iXSize + xBlockSize - 1) / xBlockSize;
			nyBlocks = (m_iYSize + yBlockSize - 1) / yBlockSize;
			if (NULL == (pScanline = new unsigned char[xBlockSize * yBlockSize]))
				throw "Couldnt allocate scan line.";
		}

		if (iRasterCount == 3)
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
			nxBlocks = (m_iXSize + xBlockSize - 1) / xBlockSize;
			nyBlocks = (m_iYSize + yBlockSize - 1) / yBlockSize;

			pRedline = new unsigned char[xBlockSize * yBlockSize];
			pGreenline = new unsigned char[xBlockSize * yBlockSize];
			pBlueline = new unsigned char[xBlockSize * yBlockSize];
		}

		if (m_iXSize * m_iYSize > (4096 * 4096))
		{
			// don't try to load giant image
			throw "Deferring load of large image";
		}

		// Set up wxWindows image
		if (NULL == (m_pImage = new wxImage(m_iXSize, m_iYSize)))
			throw "Couldn't create image.";

		// Read the data
		for(int iy = 0; iy < m_iYSize; iy++ )
		{
			ReadScanline(iy, 0);
			progress_callback(iy * 99 / m_iYSize);
			for(int iX = 0; iX < m_iXSize; iX++ )
			{
				RGBi rgb = m_row[0].m_data[iX];
				m_pImage->SetRGB(iX, iy, rgb.r, rgb.g, rgb.b);
			}
		}
	}
	
	catch (const char *msg)
	{
		wxString2 str = msg;
		if (!str.CmpNoCase(_T("Deferring load of large image")))
		{
			wxMessageBox(str);
			bBitmap = false;
		}
		else
		{
			bRet = false;
			wxString2 str2 = "Can't load Image layer: ";
			wxMessageBox(str2+str);
		}
	}

	if (bRet == true && bBitmap == true)
	{
		m_pBitmap = new wxBitmap(m_pImage);
		if (!m_pBitmap || (m_pBitmap && !m_pBitmap->Ok()))
		{
			DisplayAndLog("Couldn't create bitmap of size %d x %d", m_iXSize, m_iYSize);
			bRet = false;
			delete m_pBitmap;
			m_pBitmap = NULL;
		}
		else
			m_bInMemory = true;
	}
	if (bRet == false)
	{
		// Tidy up on failure
		if (NULL != m_pImage)
		{
			delete m_pImage;
			m_pImage = NULL;
		}
	}
	return bRet;
}

void vtImageLayer::CleanupGDALUsage()
{
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
}

RGBi *vtImageLayer::GetScanlineFromBuffer(int y)
{
	// first check if the row is already in memory
	int i;
	for (i = 0; i < BUF_SCANLINES; i++)
	{
		if (m_row[i].m_y == y)
		{
			// yes it is
			return m_row[i].m_data;
		}
	}

	// ok, it isn't. load it into the next appropriate slot.
	RGBi *data;
	ReadScanline(y, m_use_next);
	m_row[m_use_next].m_y = y;
	data = m_row[m_use_next].m_data;

	// increment which row we'll use next
	m_use_next++;
	if (m_use_next == BUF_SCANLINES)
		m_use_next = 0;

	return data;
}

void vtImageLayer::ReadScanline(int iYRequest, int bufrow)
{
	CPLErr Err;
	GDALColorEntry Ent;
	int ixBlock;
	int nxValid;

	int iyBlock = iYRequest / yBlockSize;
	int iY = iYRequest - (iyBlock * yBlockSize);

	if (iRasterCount == 1)
	{
		// Convert to rgb and write to image
		for( ixBlock = 0; ixBlock < nxBlocks; ixBlock++ )
		{
			Err = pBand->ReadBlock(ixBlock, iyBlock, pScanline);
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
				int val = pScanline[iY * xBlockSize + iX];
				if (pTable)
				{
					pTable->GetColorEntryAsRGB(val, &Ent);
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
	if (iRasterCount == 3)
	{
		RGBi rgb;
		for( ixBlock = 0; ixBlock < nxBlocks; ixBlock++ )
		{
			Err = pRed->ReadBlock(ixBlock, iyBlock, pRedline);
			if (Err != CE_None)
				throw "Readblock failed.";
			Err = pGreen->ReadBlock(ixBlock, iyBlock, pGreenline);
			if (Err != CE_None)
				throw "Readblock failed.";
			Err = pBlue->ReadBlock(ixBlock, iyBlock, pBlueline);
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
				rgb.Set(pRedline[iY * xBlockSize + iX],
					pGreenline[iY * xBlockSize + iX],
					pBlueline[iY * xBlockSize + iX]);
				m_row[bufrow].m_data[iX] = rgb;
			}
		}
	}
}
