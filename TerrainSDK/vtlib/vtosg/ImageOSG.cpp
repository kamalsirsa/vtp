//
// ImageOSG.cpp
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtString.h"
#include "vtdata/vtLog.h"
#include <osgDB/ReadFile>
#include "gdal_priv.h"
#include "vtdata/Projections.h"

//
// Set any of these definitions to use OSG's own support for the various
// image file formats instead of our own.
//
#define USE_OSG_FOR_PNG		1
#define USE_OSG_FOR_BMP		1
#define USE_OSG_FOR_JPG		1
#define USE_OSG_FOR_TIF		0

// Simple cache
typedef std::map< vtString, osg::ref_ptr<vtImage> > ImageCache;
ImageCache s_ImageCache;

void vtImageCacheClear()
{
	VTLOG("Clearing image cache, contents:\n");
	for (ImageCache::iterator iter = s_ImageCache.begin();
		iter != s_ImageCache.end(); iter++)
	{
		vtString str = iter->first;
		vtImage *im = iter->second.get();
		VTLOG("  Image '%s', refcount %d\n", (const char *) str,
			im->referenceCount());
	}
	s_ImageCache.clear();

	// We must also clear the OSG cache, otherwise things can get out of synch
	osgDB::Registry::instance()->clearObjectCache();
}

////////////////////////////////////////////////////////////////////////

vtImage::vtImage()
{
	ref();
}

vtImage::vtImage(const char *fname, bool bAllowCache)
{
	ref();
	Read(fname, bAllowCache);
}

vtImage::vtImage(vtDIB *pDIB)
{
	ref();
	m_b16bit = false;
	_CreateFromDIB(pDIB);
}

vtImage::~vtImage()
{
}

bool vtImage::Create(int width, int height, int bitdepth, bool create_palette)
{
	GLenum pixelFormat;
	GLenum dataType = GL_UNSIGNED_BYTE;

	if (bitdepth == 24)
	{
		pixelFormat = GL_RGB;
	}
	else if (bitdepth == 32)
	{
		pixelFormat = GL_RGBA;
	}
	else if (bitdepth == 8)
	{
		pixelFormat = GL_LUMINANCE;
	}
	else
		return false;

	allocateImage(width, height, 1, pixelFormat, dataType);
	m_iRowSize = computeRowWidthInBytes(_s, _pixelFormat, _dataType, _packing);

	return true;
}

vtImage *vtImageRead(const char *fname, bool bAllowCache, bool progress_callback(int))
{
	ImageCache::iterator iter;
	vtImage *image;

	iter = s_ImageCache.find(vtString(fname));
	if (iter == s_ImageCache.end())
	{
		// not found.  must try loading;
		image = new vtImage;
		if (image->Read(fname, bAllowCache, progress_callback))
		{
			s_ImageCache[fname] = image; // store in cache
			return image;
		}
		else
		{
			image->Release();
			return NULL;
		}
	}
	else
	{
		image = iter->second.get();
		// because we don't use 'smart pointers', we must pretend that this is a
		//  unique instance with its own self-possessed refcount
		image->ref();
		return image;
	}
}

osg::ref_ptr<osgDB::ReaderWriter::Options> s_options;

bool vtImage::Read(const char *fname, bool bAllowCache, bool progress_callback(int))
{
	VTLOG(" vtImage::Read(%s)\n", fname);
	m_b16bit = false;
	m_strFilename = fname;

#if !USE_OSG_FOR_BMP
	if (!stricmp(fname + strlen(fname) - 3, "bmp"))
	{
		vtDIB dib;
		if (dib.ReadBMP(fname))
		{
			if (dib.GetDepth() == 8)
			{
				// We are presumably going to use this for a texture, and we
				// don't want to worry about 8-bit color paletted textures.
				vtDIB dib2;
				dib2.Create24From8bit(dib);
				_CreateFromDIB(&dib2);
			}
			else
				_CreateFromDIB(&dib);
		}
	}
	else
#endif

#if !USE_OSG_FOR_JPG
	if (!stricmp(fname + strlen(fname) - 3, "jpg"))
	{
		vtDIB pDIB;
		if (pDIB.ReadJPEG(fname))
		{
			_CreateFromDIB(&pDIB);
		}
	}
	else
#endif

#if !USE_OSG_FOR_PNG
	if (!stricmp(fname + strlen(fname) - 3, "png"))
	{
		_ReadPNG(fname);
	}
	else
#endif

#if !USE_OSG_FOR_TIF
	if (!stricmp(fname + strlen(fname) - 3, "tif") ||
		!stricmp(fname + strlen(fname) - 4, "tiff"))
	{
		_ReadTIF(fname, progress_callback);
	}
	else
#endif
	// try to load with OSG (osgPlugins libraries)
	{
		// important for efficiency: use OSG's cache
#define OPTS osgDB::ReaderWriter::Options
#define HINT OPTS::CacheHintOptions
		osgDB::Registry *reg = osgDB::Registry::instance();
		OPTS *opts;

		opts = reg->getOptions();
		if (!opts)
		{
			VTLOG("  creating osgDB::ReaderWriter::Options\n");
			s_options = new OPTS;
			opts = s_options.get();
		}
		int before = (int) opts->getObjectCacheHint();
		if (bAllowCache)
		{
			opts->setObjectCacheHint((HINT) ((opts->getObjectCacheHint()) |
				OPTS::CACHE_IMAGES));
		}
		else
		{
			opts->setObjectCacheHint((HINT) ((opts->getObjectCacheHint()) &
				~(OPTS::CACHE_IMAGES)));
		}
		int after = (int) opts->getObjectCacheHint();
		VTLOG("  calling SetOptions,");
		reg->setOptions(opts);

		// Call OSG to attempt image load.
		osg::ref_ptr<osg::Image> pOsgImage;
		try
		{
			VTLOG("  readImageFile,");
			pOsgImage = osgDB::readImageFile(fname);
		}
		catch (...)
		{
			// Don't do anything because the (!pOsgImage.valid() below will
			//  test to see if the pointer is any good. However, we need the
			//  catch or else if osgdb throws an exception, it will be passed
			/// all the way to the app.
		}

		if (!pOsgImage.valid())
		{
			VTLOG("  failed.\n");
			return false;
		}

		// beware - NO_DELETE means that OSG has given us the same image again
		//  so don't try to steal it again
		osg::Image::AllocationMode mode = pOsgImage->getAllocationMode();
		if (mode == osg::Image::NO_DELETE)
		{
			// do nothing - we already have the buffer from the previous time
		}
		else
		{
			// _Very carefully_ copy the entire resulting image into our own
			//   image class.
			_fileName = pOsgImage->getFileName();
			_s = pOsgImage->s(); _t = pOsgImage->t(); _r = pOsgImage->r();
			_internalTextureFormat = pOsgImage->getInternalTextureFormat();
			_pixelFormat = pOsgImage->getPixelFormat();
			_dataType = pOsgImage->getDataType();
			_packing = pOsgImage->getPacking();
// OSG 0.9.8-3 (Feb. 12) has a count:
			_modifiedCount = pOsgImage->getModifiedCount();
			for (unsigned int k = 0; k < pOsgImage->getNumMipmapLevels()-1; k++)
				_mipmapData.push_back(pOsgImage->getMipmapData(k) - pOsgImage->data());
			if (pOsgImage->data())
			{
				// Steal the data by grabbing the pointer
				pOsgImage->setAllocationMode(osg::Image::NO_DELETE);
				setData(pOsgImage->data(), mode);
			}
		}
		m_iRowSize = computeRowWidthInBytes(_s, _pixelFormat, _dataType, _packing);
	}
	VTLOG("  succeeded.\n");
	return true;
}

void vtImage::Release()
{
	unref();
}

unsigned char vtImage::GetPixel8(int x, int y) const
{
	unsigned char *buf = _data + x + (_t-1-y)*m_iRowSize;
	return *buf;
}

void vtImage::SetPixel8(int x, int y, unsigned char color)
{
	unsigned char *buf = _data + x + (_t-1-y)*m_iRowSize;
	*buf = color;
}

void vtImage::GetPixel24(int x, int y, RGBi &rgb) const
{
	// OSG appears to reference y=0 as the bottom of the image
	unsigned char *buf = _data + x*3 + (_t-1-y)*m_iRowSize;
	rgb.r = buf[0];
	rgb.g = buf[1];
	rgb.b = buf[2];
}

void vtImage::SetPixel24(int x, int y, const RGBi &rgb)
{
	// OSG appears to reference y=0 as the bottom of the image
	unsigned char *buf = _data + x*3 + (_t-1-y)*m_iRowSize;
	buf[0] = rgb.r;
	buf[1] = rgb.g;
	buf[2] = rgb.b;
}

void vtImage::GetPixel32(int x, int y, RGBAi &rgba) const
{
	// OSG appears to reference y=0 as the bottom of the image
	unsigned char *buf = _data + x*4 + (_t-1-y)*m_iRowSize;
	rgba.r = buf[0];
	rgba.g = buf[1];
	rgba.b = buf[2];
	rgba.a = buf[3];
}

void vtImage::SetPixel32(int x, int y, const RGBAi &rgba)
{
	// OSG appears to reference y=0 as the bottom of the image
	unsigned char *buf = _data + x*4 + (_t-1-y)*m_iRowSize;
	buf[0] = rgba.r;
	buf[1] = rgba.g;
	buf[2] = rgba.b;
	buf[3] = rgba.a;
}

unsigned int vtImage::GetWidth() const
{
	return s();
}

unsigned int vtImage::GetHeight() const
{
	return t();
}

unsigned int vtImage::GetDepth() const
{
	return getPixelSizeInBits();
}

/**
 * Call this method to tell vtlib that you want it to use a 16-bit texture
 * (internal memory format) to be sent to the graphics card.
 */
void vtImage::Set16Bit(bool bFlag)
{
	if (bFlag)
	{
		// use a 16-bit internal
		if (_pixelFormat == GL_RGB)
			_internalTextureFormat = GL_RGB5;
		if (_pixelFormat == GL_RGBA)
			_internalTextureFormat = GL_RGB5_A1;
	}
	else
		_internalTextureFormat = _pixelFormat;
}

void vtImage::_CreateFromDIB(vtDIB *pDIB)
{
	int i, w, h, bpp;
	char *data;

	w   = pDIB->GetWidth();
	h   = pDIB->GetHeight();
	bpp = pDIB->GetDepth();
	data = (char *) pDIB->GetDIBData();

	int SizeImage = w * h * (bpp / 8);

	GLubyte *image = new GLubyte[SizeImage];

#if 1
	memcpy(image, data, SizeImage);
#else
	// Flip the orientation of the image: a DIB is stored bottom-up, but
	// OSG and OpenGL expect the bitmap to be top-down.
	// (Why is this not required?? It should be, but it works correctly
	//  if we don't do the flip.)
	int SizeRow = w * (bpp / 8);
	for (i = 0; i < h; i++)
		memcpy(image + i * SizeRow, data + (h-1-i) * SizeRow, SizeRow);
#endif

	int z;
	int pixelFormat = GL_RGB;
	GLubyte tmp;
	if ( bpp == 24 )
	{
		z = 3;

		/* BGR --> RGB */
		for (i = 0; i < w * h; i++)
		{
			tmp = image [ 3 * i ];
			image [ 3 * i ] = image [ 3 * i + 2 ];
			image [ 3 * i + 2 ] = tmp;
		}
		pixelFormat = GL_RGB;
	}
	else if ( bpp == 32 )
	{
		z = 4;

		/* BGRA --> RGBA */
		for (i = 0; i < w * h; i++ )
		{
			tmp = image [ 4 * i ];
			image [ 4 * i ] = image [ 4 * i + 2 ];
			image [ 4 * i + 2 ] = tmp;
		}
		pixelFormat = GL_RGBA;
	}
	else if ( bpp == 8 )
	{
		pixelFormat = GL_LUMINANCE;
	}

	int internalFormat;
	if (m_b16bit)
		internalFormat = GL_RGB5;		// use specific
	else
		internalFormat = pixelFormat;	// use default

	setImage(w, h, 1,		// s, t, r
	   internalFormat,		// int internalFormat,
	   pixelFormat,			// unsigned int pixelFormat,
	   GL_UNSIGNED_BYTE, 	// unsigned int dataType,
	   image,
	   osg::Image::USE_NEW_DELETE);
	m_iRowSize = computeRowWidthInBytes(_s, _pixelFormat, _dataType, _packing);
}


//////////////////////////

#if USE_OSG_FOR_PNG

bool vtImage::_ReadPNG(const char *filename)
{
	return false;
}

#else

#include "png.h"

bool vtImage::_ReadPNG(const char *filename)
{
	FILE *fp = NULL;

	unsigned char header[8];
	png_structp png;
	png_infop   info;
	png_infop   endinfo;
	png_bytep  *row_p;

	png_uint_32 width, height;
	int depth, color;

	png_uint_32 i;
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
	{
		// We compiled against the headers of one version of libpng, but
		// linked against the libraries from another version.  If you get
		// this, fix the paths in your development environment.
		return false;
	}
	info = png_create_info_struct(png);
	endinfo = png_create_info_struct(png);

	fp = fopen(filename, "rb");
	if (fp && fread(header, 1, 8, fp) && png_check_sig(header, 8))
		png_init_io(png, fp);
	else
	{
		png_destroy_read_struct(&png, &info, &endinfo);
		return false;
	}
	png_set_sig_bytes(png, 8);

	png_read_info(png, info);
	png_get_IHDR(png, info, &width, &height, &depth, &color, NULL, NULL, NULL);

	if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	// never strip alpha
//	{
//		png_set_strip_alpha(png);
//		color &= ~PNG_COLOR_MASK_ALPHA;
//	}

	// Always expand paletted images
	if (color == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png);

	/*--GAMMA--*/
//	checkForGammaEnv();
	double screenGamma = 2.2 / 1.0;
#if 0
	// Getting the gamma from the PNG file is disabled here, since
	// PhotoShop writes bizarre gamma values like .227 (PhotoShop 5.0)
	// or .45 (newer versions)
	double	fileGamma;
	if (png_get_gAMA(png, info, &fileGamma))
		png_set_gamma(png, screenGamma, fileGamma);
	else
#endif
		png_set_gamma(png, screenGamma, 1.0/2.2);

	png_read_update_info(png, info);

	m_pPngData = (png_bytep) malloc(png_get_rowbytes(png, info)*height);
	row_p = (png_bytep *) malloc(sizeof(png_bytep)*height);

	bool StandardOrientation = true;
	for (i = 0; i < height; i++) {
		if (StandardOrientation)
			row_p[height - 1 - i] = &m_pPngData[png_get_rowbytes(png, info)*i];
		else
			row_p[i] = &m_pPngData[png_get_rowbytes(png, info)*i];
	}

	png_read_image(png, row_p);
	free(row_p);

	int iBitCount;

	switch (color)
	{
		case PNG_COLOR_TYPE_GRAY:
		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_PALETTE:
			iBitCount = 24;
			break;

		case PNG_COLOR_TYPE_GRAY_ALPHA:
		case PNG_COLOR_TYPE_RGB_ALPHA:
			iBitCount = 32;
			break;

		default:
			return false;
	}

	png_read_end(png, endinfo);
	png_destroy_read_struct(&png, &info, &endinfo);

	// Don't free the data, we're going to pass it to OSG
//	free(m_pPngData);

	if (fp)
		fclose(fp);

	int pixelFormat;
	unsigned int internalFormat;

	if (iBitCount == 24)
		pixelFormat = GL_RGB;
	else if (iBitCount == 32)
		pixelFormat = GL_RGBA;

	if (m_internalformat == -1)
		internalFormat = pixelFormat;	// use default
	else
		internalFormat = m_internalformat;	// use specific

	setImage(width, height, 1,
	   internalFormat,		// int internalFormat,
	   pixelFormat,			// unsigned int pixelFormat
	   GL_UNSIGNED_BYTE,	// unsigned int dataType
	   m_pPngData,
	   osg::Image::USE_MALLOC_FREE);

	return true;
}

#endif	// USE_OSG_FOR_PNG


//////////////////////////

#if USE_OSG_FOR_TIF

bool vtImage::_ReadTIF(const char *filename)
{
	return false;
}

#else

bool vtImage::_ReadTIF(const char *filename, bool progress_callback(int))
{
	// Use GDAL to read a TIF file (or any other format that GDAL is
	//  configured to read) into this OSG image.
	bool bRet = true;
	vtString message;

	g_GDALWrapper.RequestGDALFormats();

	GDALDataset *pDataset = NULL;
	GDALRasterBand *pBand;
	GDALRasterBand *pRed;
	GDALRasterBand *pGreen;
	GDALRasterBand *pBlue;
	GDALColorTable *pTable;
	unsigned char *pScanline = NULL;
	unsigned char *pRedline = NULL;
	unsigned char *pGreenline = NULL;
	unsigned char *pBlueline = NULL;

	CPLErr Err;
	bool bColorPalette = false;
	int iXSize, iYSize;
	int nxBlocks, nyBlocks;
	int xBlockSize, yBlockSize;

	try
	{
		pDataset = (GDALDataset *) GDALOpen(filename, GA_ReadOnly);
		if(pDataset == NULL )
			throw "Couldn't open that file.";

		iXSize = pDataset->GetRasterXSize();
		iYSize = pDataset->GetRasterYSize();

		// Raster count should be 3 for colour images (assume RGB)
		int iRasterCount = pDataset->GetRasterCount();

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
				bColorPalette = true;
			}
			else if (ci == GCI_GrayIndex)
			{
				// we will assume 0-255 is black to white
			}
			else
				throw "Unsupported color interpretation.";

			pBand->GetBlockSize(&xBlockSize, &yBlockSize);
			nxBlocks = (iXSize + xBlockSize - 1) / xBlockSize;
			nyBlocks = (iYSize + yBlockSize - 1) / yBlockSize;
			if (NULL == (pScanline = new unsigned char[xBlockSize * yBlockSize]))
				throw "Couldnt allocate scan line.";
		}

		if (iRasterCount == 3)
		{
			for (int i = 1; i <= 3; i++)
			{
				pBand = pDataset->GetRasterBand(i);
				// Check data type - it's either integer or float
				if (GDT_Byte != pBand->GetRasterDataType())
					throw "Three rasters, but not of type byte.";
				switch (pBand->GetColorInterpretation())
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
			nxBlocks = (iXSize + xBlockSize - 1) / xBlockSize;
			nyBlocks = (iYSize + yBlockSize - 1) / yBlockSize;

			pRedline = new unsigned char[xBlockSize * yBlockSize];
			pGreenline = new unsigned char[xBlockSize * yBlockSize];
			pBlueline = new unsigned char[xBlockSize * yBlockSize];
		}

		// Allocate the image buffer
		if (iRasterCount == 3 || bColorPalette)
			Create(iXSize, iYSize, 24);
		else if (iRasterCount == 1)
			Create(iXSize, iYSize, 8);

		// Read the data
		VTLOG("Reading the image data (%d x %d pixels)\n", iXSize, iYSize);

		int x, y;
		int ixBlock, iyBlock;
		int nxValid, nyValid;
		int iY, iX;
		RGBi rgb;
		if (iRasterCount == 1)
		{
			GDALColorEntry Ent;
			for (iyBlock = 0; iyBlock < nyBlocks; iyBlock++)
			{
				if (progress_callback != NULL)
					progress_callback(iyBlock * 100 / nyBlocks);

				y = iyBlock * yBlockSize;
				for (ixBlock = 0; ixBlock < nxBlocks; ixBlock++)
				{
					x = ixBlock * xBlockSize;
					Err = pBand->ReadBlock(ixBlock, iyBlock, pScanline);
					if (Err != CE_None)
						throw "Problem reading the image data.";

					// Compute the portion of the block that is valid
					// for partial edge blocks.
					if ((ixBlock+1) * xBlockSize > iXSize)
						nxValid = iXSize - ixBlock * xBlockSize;
					else
						nxValid = xBlockSize;

					if( (iyBlock+1) * yBlockSize > iYSize)
						nyValid = iYSize - iyBlock * yBlockSize;
					else
						nyValid = yBlockSize;

					for( iY = 0; iY < nyValid; iY++ )
					{
						for( iX = 0; iX < nxValid; iX++ )
						{
							if (bColorPalette)
							{
								pTable->GetColorEntryAsRGB(pScanline[iY * xBlockSize + iX], &Ent);
								rgb.r = (unsigned char) Ent.c1;
								rgb.g = (unsigned char) Ent.c2;
								rgb.b = (unsigned char) Ent.c3;
								SetPixel24(x + iX, y + iY, rgb);
							}
							else
								SetPixel8(x + iX, y + iY, pScanline[iY * xBlockSize + iX]);
						}
					}
				}
			}
		}
		if (iRasterCount == 3)
		{
			for (iyBlock = 0; iyBlock < nyBlocks; iyBlock++)
			{
				if (progress_callback != NULL)
					progress_callback(iyBlock * 100 / nyBlocks);

				y = iyBlock * yBlockSize;
				for (ixBlock = 0; ixBlock < nxBlocks; ixBlock++)
				{
					x = ixBlock * xBlockSize;
					Err = pRed->ReadBlock(ixBlock, iyBlock, pRedline);
					if (Err != CE_None)
						throw "Cannot read data.";
					Err = pGreen->ReadBlock(ixBlock, iyBlock, pGreenline);
					if (Err != CE_None)
						throw "Cannot read data.";
					Err = pBlue->ReadBlock(ixBlock, iyBlock, pBlueline);
					if (Err != CE_None)
						throw "Cannot read data.";

					// Compute the portion of the block that is valid
					// for partial edge blocks.
					if ((ixBlock+1) * xBlockSize > iXSize)
						nxValid = iXSize - ixBlock * xBlockSize;
					else
						nxValid = xBlockSize;

					if( (iyBlock+1) * yBlockSize > iYSize)
						nyValid = iYSize - iyBlock * yBlockSize;
					else
						nyValid = yBlockSize;

					for (int iY = 0; iY < nyValid; iY++)
					{
						for (int iX = 0; iX < nxValid; iX++)
						{
							rgb.r = pRedline[iY * xBlockSize + iX];
							rgb.g = pGreenline[iY * xBlockSize + iX];
							rgb.b = pBlueline[iY * xBlockSize + iX];
							SetPixel24(x + iX, y + iY, rgb);
						}
					}
				}
			}
		}
	}
	catch (const char *msg)
	{
		VTLOG(msg);
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

	return bRet;
}

#endif	// USE_OSG_FOR_TIF


///////////////////////////////////////////////////////////////////////
// class vtOverlappedTiledImage

vtOverlappedTiledImage::vtOverlappedTiledImage()
{
	m_iTilesize = 0;
	m_iSpacing = 0;
	int r, c;
	for (r = 0; r < 4; r++)
		for (c = 0; c < 4; c++)
			m_Tiles[r][c] = NULL;
}

bool vtOverlappedTiledImage::Create(int iTilesize, int iBitDepth)
{
	// store the tile size
	m_iTilesize = iTilesize;
	m_iSpacing = iTilesize-1;

	// create the 4x4 grid of image tiles
	int r, c;
	for (r = 0; r < 4; r++)
		for (c = 0; c < 4; c++)
		{
			vtImage *image = new vtImage;
			if (!image->Create(iTilesize, iTilesize, iBitDepth))
				return false;
			m_Tiles[r][c] = image;
		}
	return true;
}

void vtOverlappedTiledImage::Release()
{
	int r, c;
	for (r = 0; r < 4; r++)
		for (c = 0; c < 4; c++)
		{
			if (m_Tiles[r][c] != NULL)
				m_Tiles[r][c]->Release();
		}
}

bool vtOverlappedTiledImage::Load(const char *filename, bool progress_callback(int))
{
	g_GDALWrapper.RequestGDALFormats();

	GDALDataset *poDataset = (GDALDataset *) GDALOpen(filename, GA_ReadOnly);
	if (!poDataset)
		return false;

	bool mono = (poDataset->GetRasterCount() == 1);

	GDALRasterBand *poBand1;
	GDALRasterBand *poBand2;
	GDALRasterBand *poBand3;
	int xsize = poDataset->GetRasterXSize();
	int ysize = poDataset->GetRasterYSize();

	if (mono)
		poBand1 = poDataset->GetRasterBand(1);
	else
	{
		poBand1 = poDataset->GetRasterBand(1);
		poBand2 = poDataset->GetRasterBand(2);
		poBand3 = poDataset->GetRasterBand(3);
	}
	unsigned char *lineBuf1 = (unsigned char *) CPLMalloc(sizeof(char)*xsize);
	unsigned char *lineBuf2 = (unsigned char *) CPLMalloc(sizeof(char)*xsize);
	unsigned char *lineBuf3 = (unsigned char *) CPLMalloc(sizeof(char)*xsize);

	int x_off, y_off, x, y, i, j;

	for (i = 0; i < 4; i++)
	{
		x_off = i * m_iSpacing;
		for (j = 0; j < 4; j++)
		{
			if (progress_callback != NULL)
				progress_callback(((i*4)+j)*100 / (4*4));

			y_off = j * m_iSpacing;

			vtImage *target = m_Tiles[j][i];

			RGBi rgb;
			if (mono)
			{
				for (x = 0; x < m_iTilesize; x++)
				{
					poBand1->RasterIO(GF_Read, 0, x_off+x, xsize, 1,
						lineBuf1, xsize, 1, GDT_Byte, 0, 0);
					for (y = 0; y < m_iTilesize; y++)
					{
						unsigned char *targetBandVec1 = lineBuf1 + y_off + y;
						target->SetPixel8(x, y, *targetBandVec1);
					}
				}
			}
			else
			{
				for (x = 0; x < m_iTilesize; x++)
				{
					poBand1->RasterIO(GF_Read, 0, x_off+x, xsize, 1,
						lineBuf1,xsize,1,GDT_Byte,0,0);
					poBand2->RasterIO(GF_Read, 0, x_off+x, xsize, 1,
						lineBuf2,xsize,1,GDT_Byte,0,0);
					poBand3->RasterIO(GF_Read, 0, x_off+x, xsize, 1,
						lineBuf3,xsize,1,GDT_Byte,0,0);
					
					for (y = 0; y < m_iTilesize; y++)
					{
						unsigned char *targetBandVec1 = lineBuf1 + y_off + y;
						unsigned char *targetBandVec2 = lineBuf2 + y_off + y;
						unsigned char *targetBandVec3 = lineBuf3 + y_off + y;
						rgb.Set(*targetBandVec1,*targetBandVec2,*targetBandVec3);
						target->SetPixel24(y, x, rgb);
					}
				}
			}
		}
	}
	CPLFree( lineBuf1 );
	CPLFree( lineBuf2 );
	CPLFree( lineBuf3 );
	GDALClose(poDataset);
	return true;
}


bool vtImageInfo(const char *filename, int &width, int &height, int &depth)
{
	g_GDALWrapper.RequestGDALFormats();

	// open the input image and find out the image depth using gdal
	GDALDataset *poDataset;
	poDataset = (GDALDataset *) GDALOpen(filename, GA_ReadOnly);
	if (!poDataset)
		return false;
	width = poDataset->GetRasterXSize();
	height = poDataset->GetRasterYSize();
	depth = poDataset->GetRasterCount()*8;
	GDALClose(poDataset);
	return true;
}
