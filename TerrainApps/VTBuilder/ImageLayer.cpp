//
// ImageLayer.cpp
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtLog.h"
#ifdef ENVIRON
#include "vtdata/vtError.h"
#endif
#if ROGER
#include "gdalwarper.h"
#endif
#include "vtui/Helper.h"
#include "ogr_spatialref.h"
#include "gdal_priv.h"
#include "ScaledView.h"
#include "ImageLayer.h"
#include "Helper.h"
#include "Frame.h"

#include "ExtentDlg.h"
#include "Projection2Dlg.h"


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
	m_pBitmap = new vtBitmap();
#ifdef ENVIRON
	if (!m_pBitmap->Allocate(m_iXSize, m_iYSize))
		throw VTERR_ALLOCATE;
#else
	m_pBitmap->Allocate(m_iXSize, m_iYSize);
#endif
}

vtImageLayer::~vtImageLayer()
{
	if (NULL != m_pBitmap)
		delete m_pBitmap;
	CleanupGDALUsage();

	for (int i = 0; i < BUF_SCANLINES; i++)
	{
		if (m_row[i].m_data)
			delete m_row[i].m_data;
	}
}

void vtImageLayer::SetRGB(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	// this method clearly only works for in-memory images
	if (m_pBitmap)
		m_pBitmap->SetPixel24(x, y, r, g, b);
}

void vtImageLayer::SetDefaults()
{
	m_iXSize = 0;
	m_iYSize = 0;
	m_pBitmap = NULL;

	// GDAL stuff
	pScanline = NULL;
	pRedline = NULL;
	pGreenline = NULL;
	pBlueline = NULL;
	pDataset = NULL;
	pTable = NULL;
	iRasterCount = 0;

	// scanline buffers
	for (int i = 0; i < BUF_SCANLINES; i++)
		m_row[i].m_data = NULL;
	m_use_next = 0;

	m_wsFilename = _("Untitled");
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
	rect.top =	  m_Extents.top + (pixel_size.y / 2.0f);
	rect.right =  m_Extents.right + (pixel_size.x / 2.0f);
	rect.bottom = m_Extents.bottom - (pixel_size.y / 2.0f);
	return true;
}

#ifdef ENVIRON
void vtImageLayer::DrawLayer(wxDC* pDC, vtScaledView *pView, UIContext &ui)
#else
void vtImageLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
#endif
{
	bool bDrawImage = true;
	if (m_pBitmap == NULL)
		bDrawImage = false;

	DRECT area;
	GetAreaExtent(area);

	wxRect screenrect = pView->WorldToCanvas(area);
	wxRect destRect = screenrect;
	wxRect srcRect(0, 0, m_iXSize, m_iYSize);

	double ratio_x = (double) srcRect.width / destRect.width;
	double ratio_y = (double) srcRect.height / destRect.height;

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
		
#if WIN32
		// Using StretchBlit is much faster and has less scaling/roundoff
		//  problems than using the wx method DrawBitmap
		::SetStretchBltMode((HDC) (pDC->GetHDC()), HALFTONE );

		wxDC2 *pDC2 = (wxDC2 *) pDC;
		pDC2->StretchBlit(*m_pBitmap->m_pBitmap, destRect.x, destRect.y,
			destRect.width, destRect.height, srcRect.x, srcRect.y,
			srcRect.width, srcRect.height);
#else
		// scale and draw the bitmap
		// must use SetUserScale since StretchBlt is not available
		double scale_x = 1.0/ratio_x;
		double scale_y = 1.0/ratio_y;
		pDC->SetUserScale(scale_x, scale_y);
		pDC->DrawBitmap(*m_pBitmap->m_pBitmap, (int) (destRect.x/scale_x),
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

		DrawRectangle(pDC, screenrect);
	}
}

bool vtImageLayer::TransformCoords(vtProjection &proj)
{
	return false;
}

bool vtImageLayer::OnSave()
{
	return SaveToFile(GetLayerFilename().mb_str());
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

void vtImageLayer::Offset(const DPoint2 &delta)
{
	// Shifting an image is as easy as shifting its extents
	m_Extents.left += delta.x;
	m_Extents.right += delta.x;
	m_Extents.top += delta.y;
	m_Extents.bottom += delta.y;
}

bool vtImageLayer::SetExtent(const DRECT &rect)
{
	m_Extents = rect;
	return true;
}

void vtImageLayer::GetPropertyText(wxString &str)
{
	str.Printf(_("Dimensions %d by %d pixels"), m_iXSize, m_iYSize);
}

DPoint2 vtImageLayer::GetSpacing() const
{
	return DPoint2(m_Extents.Width() / (m_iXSize),
		m_Extents.Height() / (m_iYSize));
}

bool vtImageLayer::GetFilteredColor(double x, double y, RGBi &rgb)
{
	// could speed this up by keeping these values around
	DPoint2 spacing = GetSpacing();

	double u = (x - m_Extents.left) / spacing.x;
	int ix = (int) u;
	if (ix < 0 || ix >= m_iXSize)
		return false;

	double v = (m_Extents.top - y) / spacing.y;
	int iy = (int) v;
	if (iy < 0 || iy >= m_iYSize)
		return false;

	if (!m_pBitmap)
	{
		// support for out-of-memory image here
		RGBi *data = GetScanlineFromBuffer(iy);
		rgb = data[ix];
	}
	else
	{
		// TODO: real filtering (interpolation)
		// for now, just grab closest pixel
		m_pBitmap->GetPixel24(ix, iy, rgb);
	}
	return true;
}

bool vtImageLayer::SaveToFile(const char *fname) const
{
	if (!m_pBitmap)
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

	// TODO: ask Frank if there is a way to gave GDAL write the file without
	// having to make another entire copy in memory, as it does now:
	RGBi rgb;
	GDALRasterBand *pBand;
	int i, x, y;
	for (i = 1; i <= 3; i++)
	{
		pBand = pDataset->GetRasterBand(i);

		for (x = 0; x < m_iXSize; x++)
		{
			for (y = 0; y < m_iYSize; y++)
			{
				m_pBitmap->GetPixel24(x, y, rgb);
				if (i == 1) raster[y*m_iXSize + x] = rgb.r;
				if (i == 2) raster[y*m_iXSize + x] = rgb.g;
				if (i == 3) raster[y*m_iXSize + x] = rgb.b;
			}
		}
		pBand->RasterIO( GF_Write, 0, 0, m_iXSize, m_iYSize, 
			raster, m_iXSize, m_iYSize, GDT_Byte, 0, 0 );
	}
	delete raster;
	GDALClose(pDataset);

	return true;
}


#if ROGER
static int WarpProgress(double dfComplete, const char *pszMessage, void *pProgressArg)
{
	int amount = (int)(99.0 * dfComplete);
	return !UpdateProgressDialog(amount, wxString2(pszMessage));
}
#endif

bool vtImageLayer::LoadFromGDAL()
{
	double affineTransform[6];
	const char *pProjectionString;
	OGRErr err;
	bool bRet = true;
	bool bDefer = false;
	int i;
	vtString message;

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
		const char *fname = GetLayerFilename().mb_str();
		pDataset = (GDALDataset *) GDALOpen(fname, GA_ReadOnly);
		if(pDataset == NULL )
			throw "Couldn't open that file.";

		m_iXSize = pDataset->GetRasterXSize();
		m_iYSize = pDataset->GetRasterYSize();

		IPoint2 OriginalSize;
		OriginalSize.x = m_iXSize;
		OriginalSize.y = m_iYSize;

		vtProjection temp;
		bool bHaveProj = false;
		pProjectionString = pDataset->GetProjectionRef();
		if (pProjectionString)
		{
			err = temp.importFromWkt((char**)&pProjectionString);
			if (err == OGRERR_NONE)
			{
				m_proj = temp;
				bHaveProj = true;
			}
		}
		if (!bHaveProj)
		{
			// check for existence of .prj file
			bool bSuccess = temp.ReadProjFile(GetLayerFilename().mb_str());
			if (bSuccess)
			{
				m_proj = temp;
				bHaveProj = true;
			}
		}
		// if we still don't have it
		if (!bHaveProj)
		{
			if (!GetMainFrame()->ConfirmValidCRS(&m_proj))
				throw "Import Cancelled.";
		}

		if (pDataset->GetGeoTransform(affineTransform) == CE_None)
		{
			m_Extents.left = affineTransform[0];
			m_Extents.right = m_Extents.left + affineTransform[1] * m_iXSize;
			m_Extents.top = affineTransform[3];
			m_Extents.bottom = m_Extents.top + affineTransform[5] * m_iYSize;

#if ROGER
			// Roger thinks that this special case is needed for
			//  non-geographic projections, but i don't think so.  I think
			//  transforms are entirely independent of choice of projection,
			//  and hence transform coeficients are expressed in the same
			//  units as the coordinate system.
			if (!m_proj.IsGeographic())
			{
				double linearConversionFactor = m_proj.GetLinearUnits();

				// Compute extent using the top left and bottom right image co-ordinates
				m_Extents.left = affineTransform[0]  * linearConversionFactor;
				m_Extents.top = affineTransform[3] * linearConversionFactor;
				m_Extents.right = (affineTransform[0] + affineTransform[1] * m_iXSize + affineTransform[2] * m_iYSize) * linearConversionFactor;
				m_Extents.bottom = (affineTransform[3] + affineTransform[4] * m_iXSize + affineTransform[5] * m_iYSize) * linearConversionFactor;
			}
#endif // ROGER
		}
		else
		{
			// No extents.
			m_Extents.Empty();
			wxString2 msg = _("File lacks geographic location (extents).  ");
			msg += _("Would you like to specify extents?\n");
			VTLOG(msg.mb_str());
			int res = wxMessageBox(msg, _("Image Import"), wxYES | wxCANCEL);
			if (res == wxYES)
			{
				VTLOG("Yes.\n");
				DRECT ext;
#ifdef ENVIRON
				ext = GetMainFrame()->m_area;
#else
				ext.Empty();
#endif
				ExtentDlg dlg(NULL, -1, _("Extents"));
				dlg.SetArea(ext, (m_proj.IsGeographic() != 0));
				if (dlg.ShowModal() == wxID_OK)
					m_Extents = dlg.m_area;
				else
					throw "Import Cancelled.";
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

#if 0
					pDataset = pDstDataset;
#else
					// Roger says this is a workaround for GDAL oddness
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
		{
			message.Format("Image has %d bands (not 1 or 3).", iRasterCount);
			throw (const char *)message;
		}

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
			nxBlocks = (OriginalSize.x + xBlockSize - 1) / xBlockSize;
			nyBlocks = (OriginalSize.y + yBlockSize - 1) / yBlockSize;
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
			nxBlocks = (OriginalSize.x + xBlockSize - 1) / xBlockSize;
			nyBlocks = (OriginalSize.y + yBlockSize - 1) / yBlockSize;

			pRedline = new unsigned char[xBlockSize * yBlockSize];
			pGreenline = new unsigned char[xBlockSize * yBlockSize];
			pBlueline = new unsigned char[xBlockSize * yBlockSize];
		}

		// don't try to load giant image?
		wxString2 msg;
		if (m_iXSize * m_iYSize > (6000 * 6000))
		{
			msg.Printf(_("Image is very large (%d x %d).\n"), m_iXSize, m_iYSize);
			msg += _("Would you like to create the layer using out-of-memory access to the image?"),
			VTLOG(msg.mb_str());
			int result = wxMessageBox(msg, _("Question"), wxYES_NO);
			if (result == wxYES)
				bDefer = true;
		}

		if (!bDefer)
		{
			m_pBitmap = new vtBitmap();
			if (!m_pBitmap->Allocate(m_iXSize, m_iYSize))
			{
				delete m_pBitmap;
				m_pBitmap = NULL;
				msg.Printf(_("Couldn't allocate bitmap of size %d x %d.\n"),
					m_iXSize, m_iYSize);
				msg += _("Would you like to create the layer using out-of-memory access to the image?"),
				VTLOG(msg.mb_str());
				int result = wxMessageBox(msg, _("Question"), wxYES_NO);
				if (result == wxYES)
					bDefer = true;
				else
					throw "Couldn't allocate bitmap";
			}
		}

		if (!bDefer)
		{
			// Read the data
			VTLOG("Reading the image data (%d x %d pixels)\n", m_iXSize, m_iYSize);
			for (int iy = 0; iy < m_iYSize; iy++ )
			{
				ReadScanline(iy, 0);
				if (UpdateProgressDialog(iy * 99 / m_iYSize))
				{
					// cancel
					throw "Cancelled";
				}
				for(int iX = 0; iX < m_iXSize; iX++ )
				{
					RGBi rgb = m_row[0].m_data[iX];
					m_pBitmap->SetPixel24(iX, iy, rgb);
				}
			}
			m_pBitmap->ContentsChanged();
		}
	}

	catch (const char *msg)
	{
		if (!bDefer)
		{
			wxString2 str = msg;
			bRet = false;
			wxString2 str2 = "Couldn't load Image layer: ";
			DisplayAndLog((str2+str).mb_str());
		}
	}

	// Don't close the GDAL Dataset; leave it open just in case we need it.
//	GDALClose(pDataset);

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

	// increment which buffer row we'll use next
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
				m_row[bufrow].m_data[ixBlock*xBlockSize + iX] = rgb;
			}
		}
	}
}

#if SUPPORT_HTTP
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
	FILE *fp = fopen(filename, "rb");
	if (fp)
	{
		fclose(fp);
		VTLOG("already have %s\n", (const char *) filename);
	}
	else
	{
		fp = fopen(filename, "wb");
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
			wxString2 msg;
			msg = TileFileName(e, n);
			UpdateProgressDialog(count * 100 / total, msg);

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

bool vtImageLayer::ReadFeaturesFromTerraserver(const DRECT &area, int iTheme,
											   int iMetersPerPixel, int iUTMZone,
											   const char *filename)
{
#if SUPPORT_HTTP

	// The cache directory needs to exist; test if it's already there.
	const char *testname = TileDownloadDir "/test.txt";
	FILE *fp = fopen(testname, "wb");
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

	m_iXSize = (TerrainEastingE - TerrainEastingW) / MetersPerPixel;
	m_iYSize = (TerrainNorthingN - TerrainNorthingS) / MetersPerPixel;
	m_pBitmap = new vtBitmap();
	m_pBitmap->Allocate(m_iXSize, m_iYSize);

	vtDIB dib;
	dib.Create(m_iXSize, m_iYSize, 8, true);
	if (!MosaicAllTiles(dib))
		return false;

	if (!dib.WriteJPEG(filename, 99))
		return false;

	return true;
#else
	return false;
#endif
}

