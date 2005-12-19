//
// ImageLayer.cpp
//
// Copyright (c) 2002-2005 Virtual Terrain Project
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
#include "vtui/ProjectionDlg.h"

#include "ogr_spatialref.h"
#include "gdal_priv.h"
#include "BuilderView.h"	// For grid marks
#include "ImageLayer.h"
#include "Helper.h"
#include "Frame.h"
#include "vtBitmap.h"

#include "ExtentDlg.h"


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
	m_pBitmap = new vtBitmap;
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

#ifdef ENVIRON
void vtImageLayer::DrawLayer(wxDC* pDC, vtScaledView *pView, UIContext &ui)
#else
void vtImageLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
#endif
{
	bool bDrawImage = true;
	if (m_pBitmap == NULL)
		bDrawImage = false;

	wxRect screenrect = pView->WorldToCanvas(m_Extents);
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

void vtImageLayer::GetPropertyText(wxString &strIn)
{
	wxString2 str;

	strIn.Printf(_("Dimensions %d by %d pixels"), m_iXSize, m_iYSize);
	strIn += _T("\n");

	bool bGeo = (m_proj.IsGeographic() != 0);
	strIn += _("Spacing: ");
	DPoint2 spacing(m_Extents.Width() / m_iXSize, m_Extents.Height() / m_iYSize);
	str += FormatCoord(bGeo, spacing.x);
	str += _T(" x ");
	str += FormatCoord(bGeo, spacing.y);
	str += _T("\n");
	strIn += str;
}

DPoint2 vtImageLayer::GetSpacing() const
{
	return DPoint2(m_Extents.Width() / (m_iXSize),
		m_Extents.Height() / (m_iYSize));
}

bool vtImageLayer::GetFilteredColor(const DPoint2 &p, RGBi &rgb)
{
	// could speed this up by keeping these values around
	DPoint2 spacing = GetSpacing();

	double u = (p.x - m_Extents.left) / spacing.x;
	int ix = (int) u;
	if (u == (double) m_iXSize)		// check for exact far edge
		ix = m_iXSize-1;
	if (ix < 0 || ix >= m_iXSize)
		return false;

	double v = (m_Extents.top - p.y) / spacing.y;
	int iy = (int) v;
	if (v == (double) m_iYSize)		// check for exact far edge
		iy = m_iYSize-1;
	if (iy < 0 || iy >= m_iYSize)
		return false;
	GetFilteredColor(ix, iy, rgb);
	return true;
}

void vtImageLayer::GetFilteredColor(int x, int y, RGBi &rgb)
{
	if (m_pBitmap)
	{
		// TODO: real filtering (interpolation)
		// for now, just grab closest pixel
		m_pBitmap->GetPixel24(x, y, rgb);
	}
	else
	{
		// support for out-of-memory image here
		RGBi *data = GetScanlineFromBuffer(y);
		rgb = data[x];
	}
}

bool vtImageLayer::ImportFromFile(const wxString2 &strFileName, bool progress_callback(int am))
{
	VTLOG("ImportFromFile '%s'\n", strFileName.mb_str());

	wxString strExt = strFileName.AfterLast('.');

	bool success;
	if (!strExt.Left(3).CmpNoCase(_T("ppm")))
	{
		m_pBitmap = new vtBitmap;
		success = ReadPPM(strFileName.mb_str());
		if (!success)
		{
			delete m_pBitmap;
			m_pBitmap = NULL;
		}
	}
	else
	{
		SetLayerFilename(strFileName);
		success = OnLoad();
	}
	return success;
}

bool vtImageLayer::ReadPPM(const char *fname, bool progress_callback(int))
{
	// open input file
	FILE *fp = fopen(fname, "rb");
	if (!fp)		// Could not open input file
		return false;

	char sbuf[512],			// buffer for file input
		 dummy[2];			// used for \n and \0
	if (fread(sbuf, sizeof(char), 2, fp) != 2)
	{
		// "Could not read file"
		return false;
	}

	bool bBinary = false;	// PGM binary format flag: assume PGM ascii
	if (!strncmp(sbuf,"P6",2))
		bBinary = 1;		// PGM binary format

	if (!strncmp(sbuf, "P5", 2) || !strncmp(sbuf, "P2", 2))
	{
		/* not the flavor of PNM we're expecting */
		fclose(fp);
		return false;
	}

	// read PGM ASCII or binary file
	bool have_header = false;
	int coord_sys = 0;
	int coord_zone = 0;
	int coord_datum = 0;
	DRECT ext;
	double x, y;
	while ((fscanf(fp, "%s", sbuf) != EOF) && sbuf[0] == '#')
	{
		// comment
		fscanf(fp,"%[^\n]", sbuf);  // read comment beyond '#'
		fscanf(fp,"%[\n]", dummy);  // read newline
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

	m_iXSize = atoi(sbuf);		// store xsize of array
	fscanf(fp,"%s",sbuf);		// read ysize of array
	m_iYSize = atoi(sbuf);
	fscanf(fp,"%s\n",sbuf);		// read maxval of array
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
		ext.top = m_iYSize-1;
		ext.right = m_iXSize-1;
		ext.bottom = 0;
	}
	m_Extents = ext;
	m_pBitmap->Allocate(m_iXSize, m_iYSize, 24);

	// read PPM binary
	int offset_start = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int offset_end = ftell(fp);
	fseek(fp, offset_start, SEEK_SET);	// go back again

	int data_length = offset_end - offset_start;
	int data_size = data_length / (m_iXSize*m_iYSize);

	int line_length = 3 * m_iXSize;
	unsigned char *line = new unsigned char[line_length];

	for (int j = 0; j < m_iYSize; j++)
	{
		if (progress_callback != NULL) progress_callback(j * 100 / m_iYSize);

		fread(line, line_length, 1, fp);

		for (int i = 0; i < m_iXSize; i++)
			m_pBitmap->SetPixel24(i, j, line[i*3+0], line[i*3+1], line[i*3+2]);
	}
	delete line;
	fclose(fp);
	return true;
}

bool vtImageLayer::SaveToFile(const char *fname) const
{
	if (!m_pBitmap)
		return false;

	vtString sExt = GetExtension(fname, false); //get extension type
	if (sExt.CompareNoCase(".jpg") == 0)
	{
		if (m_pBitmap->WriteJPEG(fname, 90))
		{
			// Also write JPEG world file
			vtString sJGWFile = ChangeFileExtension(fname, ".jgw");
			DPoint2 spacing = GetSpacing();

			FILE *fout = fopen(sJGWFile, "w");
			if (fout)
			{
				fprintf(fout, "%lf\n%lf\n%lf\n%lf\n%.2lf\n%.2lf\n",
					spacing.x, 0.0, 0.0, -1*spacing.y, m_Extents.left, m_Extents.top);
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

	if (m_iXSize * m_iYSize > 512*512)
		OpenProgressDialog(_("Writing file"), false);

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
			progress_callback((i-1)*33 + (x * 33 / m_iXSize));

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

	CloseProgressDialog();

	return true;
}

bool vtImageLayer::ReadPNGFromMemory(unsigned char *buf, int len)
{
	m_pBitmap = new vtBitmap;
	if (m_pBitmap->ReadPNGFromMemory(buf, len))
	{
		m_iXSize = m_pBitmap->GetWidth();
		m_iYSize = m_pBitmap->GetHeight();
		return true;
	}
	else
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
		return false;
	}
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

		if (m_iXSize * m_iYSize > 512*512)
			OpenProgressDialog(_("Reading file"), false);

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
			int res = wxMessageBox(msg, _("Image Import"), wxYES_NO | wxCANCEL);
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
			if (res == wxNO)
			{
				throw "Sorry, we need extents in order to make use of an image.";
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
				wxFileDialog saveRealigned(NULL, _T("Save realigned image as"),
					_T(""), _T(""), _T("GeoTiff files (*.tif)|*.tif"), wxSAVE|wxOVERWRITE_PROMPT);

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
		if (m_iXSize * m_iYSize > (4096 * 4096))
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
			m_pBitmap = new vtBitmap;
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

			// Read the data
			VTLOG("Reading the image data (%d x %d pixels)\n", m_iXSize, m_iYSize);
			for (int iy = 0; iy < m_iYSize; iy++ )
			{
				if (progress_callback != NULL)
					progress_callback(iy * 100 / m_iYSize);

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

	CloseProgressDialog();

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
//	VTLOG("readscanline %d\n", iYRequest);

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
	m_pBitmap = new vtBitmap;
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

bool vtImageLayer::WriteGridOfPGMPyramids(const TilingOptions &opts, BuilderView *pView)
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// largest tile size
	int base_tilesize = opts.lod0size;

	int gridcols = m_iXSize;
	int gridrows = m_iYSize;

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

	// Write .ini file
	FILE *fp = fopen(opts.fname, "wb");
	if (!fp)
		return false;

	// Try to create directory to hold the tiles
	vtString dirname = opts.fname;
	RemoveFileExtensions(dirname);
	if (!vtCreateDir(dirname))
	{
		fclose(fp);
		return false;
	}

	fprintf(fp, "[TilesetDescription]\n");
	fprintf(fp, "Columns=%d\n", opts.cols);
	fprintf(fp, "Rows=%d\n", opts.rows);
	fprintf(fp, "LOD0_Size=%d\n", opts.lod0size);
	fprintf(fp, "Extent_Left=%.16lg\n", area.left);
	fprintf(fp, "Extent_Right=%.16lg\n", area.right);
	fprintf(fp, "Extent_Bottom=%.16lg\n", area.bottom);
	fprintf(fp, "Extent_Top=%.16lg\n", area.top);
	// write CRS, but pretty it up a bit
	OGRSpatialReference *poSimpleClone = m_proj.Clone();
	poSimpleClone->GetRoot()->StripNodes( "AXIS" );
	poSimpleClone->GetRoot()->StripNodes( "AUTHORITY" );
	char *wkt;
	poSimpleClone->exportToWkt(&wkt);
	fprintf(fp, "CRS=%s\n", wkt);
	delete poSimpleClone;
	OGRFree(wkt);
	fclose(fp);

	if (!m_pBitmap)
	{
		// If we're dealing with an out-of-core imge, consider how many rows
		//  we need to cache to avoid reading the file more than once during
		//  the generation of the tiles
		int need_cache_rows = (m_iYSize + (opts.rows-1)) / opts.rows;
		int need_cache_bytes = need_cache_rows * m_iXSize * 3;
		// add a little bit for rounding up
		need_cache_bytes += (need_cache_bytes / 10);

		// there's little point in shrinking the cache, so check existing size
		int existing = GDALGetCacheMax();
		if (need_cache_bytes > existing)
			GDALSetCacheMax(need_cache_bytes);
	}

	int i, j, lod;
	int total = opts.rows * opts.cols * opts.numlods, done = 0;
	for (j = opts.rows-1; j >= 0; j--)
	{
		for (i = 0; i < opts.cols; i++)
		{
			DRECT tile_area;
			tile_area.left = area.left + tile_dim.x * i;
			tile_area.right = area.left + tile_dim.x * (i+1);
			tile_area.bottom = area.bottom + tile_dim.y * j;
			tile_area.top = area.bottom + tile_dim.y * (j+1);

			int col = i;
			int row = opts.rows-1-j;

			for (lod = 0; lod < opts.numlods; lod++)
			{
				int tilesize = base_tilesize >> lod;

				// safety check: don't create LODs which are too small
				if (tilesize < 2)
					continue;

				// Images are written as PixelIsPoint, and each LOD has
				// its own sample spacing
				DPoint2 spacing = tile_dim / (tilesize-1);

				vtString fname = dirname, str;
				fname += '/';
				if (lod == 0)
					str.Format("tile.%d-%d.ppm", col, row);
				else
					str.Format("tile.%d-%d.ppm%d", col, row, lod);
				fname += str;

				// make a message for the progress dialog
				wxString msg;
				msg.Printf(_("Writing tile '%hs', size %dx%d"),
					(const char *)fname, tilesize, tilesize);
				UpdateProgressDialog(done*99/total, msg);

				// also draw our progress in the main view
				if (pView)
					pView->ShowGridMarks(area, opts.cols, opts.rows, col, row);

				FILE *fp = fopen(fname, "wb");
				fprintf(fp, "P6\n");
				fprintf(fp, "# DEM\n");
				fprintf(fp, "# description=resampled with VTBuilder\n");
				fprintf(fp, "# coordinate system=UTM\n");
				fprintf(fp, "# coordinate zone=5\n");
				fprintf(fp, "# coordinate datum=0\n");
				fprintf(fp, "# SW corner=%lf/%lf meters\n", tile_area.left, tile_area.bottom);
				fprintf(fp, "# NW corner=%lf/%lf meters\n", tile_area.left, tile_area.top);
				fprintf(fp, "# NE corner=%lf/%lf meters\n", tile_area.right, tile_area.top);
				fprintf(fp, "# SE corner=%lf/%lf meters\n", tile_area.right, tile_area.bottom);
				fprintf(fp, "# cell size=%lf/%lf meters\n", cell_size.x*(1<<lod), cell_size.y*(1<<lod));
				fprintf(fp, "# vertical scaling=1 meters\n");
				fprintf(fp, "# missing value=-9999\n");
				fprintf(fp, "%d %d\n", tilesize, tilesize);
				fprintf(fp, "255\n");

				DPoint2 p;
				int x, y;
				RGBi rgb;
				unsigned char rgb_bytes[3];
				for (y = tilesize-1; y >= 0; y--)
				{
					p.y = tile_area.bottom + y * spacing.y;
					for (x = 0; x < tilesize; x++)
					{
						p.x = tile_area.left + x * spacing.x;

						GetFilteredColor(p, rgb);
#if 1
						// For testing, add stripes to indicate LOD
						if (lod == 3 && x == y) rgb.Set(255,0,0);

						if (lod == 2 && (
							x == tilesize-y ||
							x == y+tilesize/2 ||
							x == y-tilesize/2)) rgb.Set(0,255,0);

						if (lod == 1 && (x%16)==0) rgb.Set(0,0,90);

						if (lod == 0 && (y%8)==0) rgb.Set(90,0,90);
#endif
						rgb_bytes[0] = rgb.r;
						rgb_bytes[1] = rgb.g;
						rgb_bytes[2] = rgb.b;
						fwrite(rgb_bytes, 3, 1, fp);
					}
				}
				fclose(fp);

				done++;
			}
		}
	}
	if (pView)
		pView->HideGridMarks();
	return true;
}
