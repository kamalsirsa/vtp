//
// ImageOSG.cpp
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtString.h"
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

// Simple cache
typedef std::map< vtString, osg::ref_ptr<vtImage> > ImageCache;
ImageCache s_ImageCache;

void vtImageCacheClear()
{
	s_ImageCache.clear();
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

vtImage *vtImageRead(const char *fname, bool bAllowCache)
{
	ImageCache::iterator iter;
	vtImage *image;

	iter = s_ImageCache.find(vtString(fname));
	if (iter == s_ImageCache.end())
	{
		// not found.  must try loading;
		image = new vtImage();
		if (image->Read(fname, bAllowCache))
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

bool vtImage::Read(const char *fname, bool bAllowCache)
{
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
		reg->setOptions(opts);

		// Call OSG to attempt image load.
		osg::ref_ptr<osg::Image> pOsgImage = osgDB::readImageFile(fname);

		if (!pOsgImage.valid())
			return false;

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
// OSG 0.9.8-2 has a tag:
//			_modifiedTag = pOsgImage->getModifiedTag();
// OSG 0.9.8-3 (Feb. 12) has a count:
			_modifiedCount = pOsgImage->getModifiedCount();
			for (unsigned int k = 0; k < pOsgImage->getNumMipmapLevels()-1; k++)
				_mipmapData.push_back(pOsgImage->getMipmapData(k) - pOsgImage->data());
			if (pOsgImage->data())
			{
				// Steal the data by copying
//				int size = pOsgImage->getTotalSizeInBytesIncludingMipmaps();
//				setData(new unsigned char [size],USE_NEW_DELETE);
//				memcpy(_data,pOsgImage->data(),size);

				// Steal the data by grabbing the pointer
				pOsgImage->setAllocationMode(osg::Image::NO_DELETE);
				setData(pOsgImage->data(), mode);
			}
		}
		m_iRowSize = computeRowWidthInBytes(_s, _pixelFormat, _dataType, _packing);
	}
	return (_data != NULL);
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
 * Call this method to tell vtlib that you have modified the contents of a
 *  texture so it needs to be sent again to the graphics card.
 */
void vtImage::Modified()
{
	// OSG calls a modified texture 'dirty'
	dirty();
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
	float *lineBuf1;
	float *lineBuf2;
	float *lineBuf3;
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
	lineBuf1 = (float *) CPLMalloc(sizeof(float)*xsize);
	lineBuf2 = (float *) CPLMalloc(sizeof(float)*xsize);
	lineBuf3 = (float *) CPLMalloc(sizeof(float)*xsize);

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
						lineBuf1, xsize, 1, GDT_Float32, 0, 0);
					for (y = 0; y < m_iTilesize; y++)
					{
						float *targetBandVec1 = lineBuf1 + y_off + y;
						target->SetPixel8(x, y, *targetBandVec1);
					}
				}
			}
			else
			{
				for (x = 0; x < m_iTilesize; x++)
				{
					poBand1->RasterIO(GF_Read, 0, x_off+x, xsize, 1,
						lineBuf1,xsize,1,GDT_Float32,0,0);
					poBand2->RasterIO(GF_Read, 0, x_off+x, xsize, 1,
						lineBuf2,xsize,1,GDT_Float32,0,0);
					poBand3->RasterIO(GF_Read, 0, x_off+x, xsize, 1,
						lineBuf3,xsize,1,GDT_Float32,0,0);
					
					for (y = 0; y < m_iTilesize; y++)
					{
						float *targetBandVec1 = lineBuf1 + y_off + y;
						float *targetBandVec2 = lineBuf2 + y_off + y;
						float *targetBandVec3 = lineBuf3 + y_off + y;
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
