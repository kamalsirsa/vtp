
#include "stdafx.h"
#include "BImage.h"
#include "Dib.h"

// GBM
#include "GBMWrapper.h"

// GDAL
#include "gdal_priv.h"

/////////////////////////////////////////////////////////////////////////////
// CBImage commands

CBImage::CBImage()
{
	m_pSourceGBM = NULL;
//	m_pMonoGBM = NULL;

	m_pSourceDIB = NULL;
	m_pMonoDIB = NULL;
	m_pCurrentDIB = NULL;
}

CBImage::~CBImage()
{
	if (m_pSourceGBM)
		delete m_pSourceGBM;
//	if (m_pMonoGBM)
//		delete m_pMonoGBM;

	if (m_pSourceDIB)
		delete m_pSourceDIB;
	if (m_pMonoDIB)
		delete m_pMonoDIB;
}

bool CBImage::LoadTFW(const char *szPathName)
{
	char worldfile[100];
	strcpy(worldfile, szPathName);
	char *pPtr = strrchr(worldfile, '.');

	if (!pPtr)
		return false;

	strcpy(pPtr, ".tfw");
	FILE* tfwstream;
	float dummy;
	if ( (tfwstream = fopen(worldfile, "r")) != NULL)
	{
		fscanf(tfwstream, "%f\n", &m_xMetersPerPixel);
		fscanf(tfwstream, "%f\n", &dummy); //don't want these two
		fscanf(tfwstream, "%f\n", &dummy); 
		fscanf(tfwstream, "%f\n", &m_yMetersPerPixel); 
		fscanf(tfwstream, "%f\n", &m_xUTMoffset);
		fscanf(tfwstream, "%f\n", &m_yUTMoffset);
		fclose(tfwstream);
	}
	else
	{
		CString msg;
		msg.Format("Couldn't find corresponding world file '%s'", worldfile);
		AfxMessageBox(msg);
		return false;
	}
	return true;
}

#define USE_GDAL 0

bool CBImage::LoadFromFile(const char *szPathName)
{
	if (!LoadTFW(szPathName))
		return false;

#if USE_GDAL
	// Testing using GDAL instead of GBM
	GDALDataset  *poDataset;

	GDALAllRegister();

	poDataset = (GDALDataset *) GDALOpen( szPathName, GA_ReadOnly );
	if( poDataset == NULL )
	{
		// failed.
		return false;
	}
	m_PixelSize.x = poDataset->GetRasterXSize();
	m_PixelSize.y = poDataset->GetRasterYSize();

	// compute size of image in meters
	m_fImageWidth = m_xMetersPerPixel * m_PixelSize.x;
	m_fImageHeight = -(m_yMetersPerPixel * m_PixelSize.y);

	// Raster count should be 1 for elevation datasets
	int rc = poDataset->GetRasterCount();

	GDALRasterBand  *poBand;
	poBand = poDataset->GetRasterBand( 1 );

	// Check data type - it's either integer or float
	GDALDataType rtype = poBand->GetRasterDataType();
	if (rtype == GDT_Byte)
	{
		AfxMessageBox("Sorry, you cannot load a .tif or a .bmp whose "
			"color depth does not equal 8 bits!");
		return false;
	}

	// Clean up
	//CPLFree(pasScanline);
	delete poDataset;

	// Return success
	return true;
#else
	// Original GBM-using code
	char *pPtr = strrchr(szPathName, '.');
	pPtr++;
	char  szExt[4];
	strncpy (szExt, pPtr, 3);
	szExt[3] = '\0';
	_strupr(szExt);

	if (strcmp(szExt, "TIF") == 0)
	{
		m_pSourceGBM = new CGBM(szPathName);
		m_initialized = false;
	}
	else
		return false;
	
	// Look up the actual pixel size of the bitmap
	m_PixelSize.x = m_pSourceGBM->GetWidth();
	m_PixelSize.y = m_pSourceGBM->GetHeight();

	// compute size of image in meters
	m_fImageWidth = m_xMetersPerPixel * m_PixelSize.x;
	m_fImageHeight = -(m_yMetersPerPixel * m_PixelSize.y);

	//check color depth
	if 	(m_pSourceGBM->GetDepth()!=8)
	{
		AfxMessageBox("Sorry, you cannot load a .tif or a .bmp whose color depth does not equal 8 bits!");
		return false;
	}
	else
		return true;
#endif
}

