//
// vtImage.cpp
//
// Copyright (c) 2002-2007 Virtual Terrain Project
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
#include "Frame.h"
#include "vtBitmap.h"
#include "LocalDatabuf.h"

#include "ExtentDlg.h"

vtImage::vtImage()
{
	SetDefaults();
}

vtImage::vtImage(const DRECT &area, int xsize, int ysize,
						   const vtProjection &proj)
{
	SetDefaults();
	m_Extents = area;
	m_iXSize = xsize;
	m_iYSize = ysize;
	m_proj = proj;

	// yes, we could use some error-checking here
	m_pBitmap = new vtBitmap;
	m_pBitmap->Allocate(m_iXSize, m_iYSize);
}

vtImage::~vtImage()
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

void vtImage::SetRGB(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	// this method clearly only works for in-memory images
	if (m_pBitmap)
		m_pBitmap->SetPixel24(x, y, r, g, b);
}

void vtImage::SetRGB(int x, int y, const RGBi &rgb)
{
	// this method clearly only works for in-memory images
	if (m_pBitmap)
		m_pBitmap->SetPixel24(x, y, rgb);
}

void vtImage::SetDefaults()
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

	m_pCanvas = NULL;
}

bool vtImage::GetExtent(DRECT &rect)
{
	rect = m_Extents;
	return true;
}

void vtImage::DrawToView(wxDC* pDC, vtScaledView *pView)
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

bool vtImage::ConvertProjection(vtImage *input, vtProjection &proj_new,
								 bool progress_callback(int))
{
	bool success = false;
	return success;
}

void vtImage::GetProjection(vtProjection &proj)
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

bool vtImage::IsAllocated() const
{
	return (m_pBitmap != NULL && m_pBitmap->IsAllocated());
}

DPoint2 vtImage::GetSpacing() const
{
	return DPoint2(m_Extents.Width() / (m_iXSize),
		m_Extents.Height() / (m_iYSize));
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
 */
bool vtImage::GetColorSolid(const DPoint2 &p, RGBi &rgb)
{
	// could speed this up by keeping these values around
	DPoint2 spacing = GetSpacing();

	double u = (p.x - m_Extents.left) / spacing.x;
	int ix = (int) u;				// round to closest pixel
	if (u == (double) m_iXSize)		// check for exact far edge
		ix = m_iXSize-1;
	if (ix < 0 || ix >= m_iXSize)
		return false;

	double v = (m_Extents.top - p.y) / spacing.y;
	int iy = (int) v;				// round to closest pixel
	if (v == (double) m_iYSize)		// check for exact far edge
		iy = m_iYSize-1;
	if (iy < 0 || iy >= m_iYSize)
		return false;
	GetRGB(ix, iy, rgb);
	return true;
}

void MakeSampleOffsets(const DPoint2 cellsize, unsigned int N, DLine2 &offsets)
{
	DPoint2 spacing = cellsize / N;
	DPoint2 base = spacing * -((float) (N-1) / 2);
	for (unsigned int i = 0; i < N; i++)
		for (unsigned int j = 0; j < N; j++)
			offsets.Append(DPoint2(base.x + (i*spacing.x), base.y + (j*spacing.y)));
}

/**
 * Get image color by sampling several points and averaging them.
 * The area to test is given by center and offsets, use MakeSampleOffsets()
 * to make a set if N x N offsets.
 */
bool vtImage::GetMultiSample(const DPoint2 &p, const DLine2 &offsets, RGBi &rgb)
{
	RGBi color;
	rgb.Set(0,0,0);
	int count = 0;
	for (unsigned int i = 0; i < offsets.GetSize(); i++)
	{
		if (GetColorSolid(p+offsets[i], color))
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

void vtImage::GetRGB(int x, int y, RGBi &rgb)
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
	delete [] line;
	fclose(fp);
	return true;
}

bool vtImage::WritePPM(const char *fname)
{
	// open input file
	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)		// Could not open output file
		return false;

	fprintf(fp, "P6\n");		// PGM binary format
	fprintf(fp, "%d %d\n", m_iXSize, m_iYSize);
	fprintf(fp, "255\n");		// PGM standard value

	int line_length = 3 * m_iXSize;
	unsigned char *line = new unsigned char[line_length];

	RGBi rgb;
	for (int j = 0; j < m_iYSize; j++)
	{
		for (int i = 0; i < m_iXSize; i++)
		{
			GetRGB(i, j, rgb);
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
	delete [] raster;
	GDALClose(pDataset);

	CloseProgressDialog();

	return true;
}

bool vtImage::ReadPNGFromMemory(unsigned char *buf, int len)
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

bool vtImage::LoadFromGDAL(const char *fname)
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
		// GDAL doesn't yet support utf-8 or wide filenames, so convert
		vtString fname_local = UTF8ToLocal(fname);

		pDataset = (GDALDataset *) GDALOpen(fname_local, GA_ReadOnly);
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
			int num_undefined = 0;
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
				case GCI_Undefined:
					num_undefined++;
					break;
				}
			}
			if (num_undefined == 3)
			{
				// All three are undefined, assume they are R,G,B
				pRed = pDataset->GetRasterBand(1);
				pGreen = pDataset->GetRasterBand(2);
				pBlue = pDataset->GetRasterBand(3);
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
		wxString msg;
		if (m_iXSize * m_iYSize > (4096 * 4096))
		{
			if (GetMainFrame()->m_Options.GetValueBool(TAG_LOAD_IMAGES_ALWAYS))
				bDefer = false;
			else if (GetMainFrame()->m_Options.GetValueBool(TAG_LOAD_IMAGES_NEVER))
				bDefer = true;
			else
			{
				// Ask
				msg.Printf(_("Image is very large (%d x %d).\n"), m_iXSize, m_iYSize);
				msg += _("Would you like to create the layer using out-of-memory access to the image?"),
				VTLOG(msg.mb_str(wxConvUTF8));
				int result = wxMessageBox(msg, _("Question"), wxYES_NO);
				if (result == wxYES)
					bDefer = true;
			}
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
	}

	catch (const char *msg)
	{
		if (!bDefer)
		{
			bRet = false;
			vtString str = "Couldn't load Image layer: ";
			str += msg;
			DisplayAndLog(str);
		}
	}

	CloseProgressDialog();

	// Don't close the GDAL Dataset; leave it open just in case we need it.
//	GDALClose(pDataset);

	return bRet;
}

void vtImage::CleanupGDALUsage()
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

RGBi *vtImage::GetScanlineFromBuffer(int y)
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

void vtImage::ReadScanline(int iYRequest, int bufrow)
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
	else if (iRasterCount == 3)
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
#if SUPPORT_HTTP

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

bool vtImage::WriteGridOfTilePyramids(const TilingOptions &opts, BuilderView *pView)
{
	wxFrame *frame = NULL;

#if USE_OPENGL
	if (opts.bUseTextureCompression && opts.eCompressionType == TC_OPENGL)
	{
		frame = new wxFrame;
		frame->Create(pView, -1, _T("Texture Compression OpenGL Context"),
			wxPoint(100,400), wxSize(280, 300), wxCAPTION | wxCLIP_CHILDREN);
		m_pCanvas = new ImageGLCanvas(frame);
	}
#endif

	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

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
	m_iTotal = opts.rows * opts.cols * opts.numlods;
	m_iCompleted = 0;

	clock_t tm1 = clock();
	bool bCancelled = false;
	for (j = opts.rows-1; j >= 0 && !bCancelled; j--)
	{
		for (i = 0; i < opts.cols && !bCancelled; i++)
		{
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
			&lod_existence_map);

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
	unsigned char *rgb_bytes = (unsigned char *) malloc(tilesize * tilesize * 3);
	int cb = 0;	// count bytes

	DPoint2 p;
	int x, y;
	RGBi rgb;

	for (y = tilesize-1; y >= 0; y--)
	{
		p.y = tile_area.bottom + y * spacing.y;
		for (x = 0; x < tilesize; x++)
		{
			p.x = tile_area.left + x * spacing.x;

			GetColorSolid(p, rgb);
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
	output_buf.set_extents(tile_area.left, tile_area.right, tile_area.top, tile_area.bottom);

	// Write and optionally compress the image
	WriteMiniImage(fname, opts, rgb_bytes, output_buf,
		iUncompressedSize, m_pCanvas);

	// Free the uncompressed image
	free(rgb_bytes);

	m_iCompleted++;

	return true;
}

