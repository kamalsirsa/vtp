//
// ImageOSG.cpp
//
// Implementation of vtImage for the OpenSG library
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtString.h"
#include "gdal_priv.h"
#include "vtdata/Projections.h"

#include <OpenSG/OSGPathHandler.h>
#include <OpenSG/OSGImageFileHandler.h>
#include <OpenSG/OSGJPGImageFileType.h>
#include <OpenSG/OSGPNGImageFileType.h>

#include <list>

typedef std::map< vtString, vtImage* > ImageCache;
ImageCache s_ImageCache;

void vtImageCacheClear()
{
	s_ImageCache.clear();
}

////////////////////////////////////////////////////////////////////////

vtImage::vtImage()
{
	m_Image = osg::Image::create();
}

vtImage::vtImage(const char *fname, bool bAllowCache)
{
	vtImage();

	Read(fname, bAllowCache);
}

//don't need this one anymore
vtImage::vtImage(vtDIB *pDIB)
{
	vtImage();

	m_b16bit = false;
//	_CreateFromDIB(pDIB);
}

vtImage::~vtImage()
{
}

void vtImage::_BasicInit()
{
	//ref();
	m_b16bit = false;
	m_extents.Empty();
}

bool vtImage::Create(int width, int height, int bitdepth, bool create_palette)
{
	GLenum pixelFormat;
	GLenum dataType = osg::Image::OSG_UINT8_IMAGEDATA;

	if( bitdepth == 24 ) {
		pixelFormat = osg::Image::OSG_RGB_PF;
	} else if( bitdepth == 32 ) {
		pixelFormat = osg::Image::OSG_RGBA_PF;
	} else if( bitdepth == 8 ) {
		pixelFormat = osg::Image::OSG_L_PF;
	} else
		return false;

	osg::UInt8 *image(0);// = new osg::UInt8[width*height*bitdepth/8];
	beginEditCP(m_Image);
	m_Image->set(
				pixelFormat,//UInt32 pixelFormat,
				width,//Int32 width,
				height,//Int32 height=1,
				1,//Int32 depth=1,
				1,//,//Int32 mipmapCount=1,
				1,//Int32 frameCount=1,
				0.f,//Time frameDelay=0.0,
				image,	//,//const UInt8 *data=0,
				dataType,
				true,//bool allocMem=true,
				1//Int32 sideCount=1)
				);
	endEditCP(m_Image);

	//TODO check this one
	m_iRowSize = width * (bitdepth / 8);

	return true;
}

vtImage *vtImageRead(const char *fname, bool bAllowCache)
{
	ImageCache::iterator iter;
	vtImage *image;

	iter = s_ImageCache.find(vtString(fname));
	if( iter == s_ImageCache.end() ) {
		// not found.  must try loading;
		image = new vtImage();
		if( image->Read(fname, bAllowCache) ) {
			s_ImageCache[fname] = image; // store in cache
			return image;
		} else {
			image->Release();
			return NULL;
		}
	} else {
		image = iter->second;//.get(); TODO should return the image
		// because we don't use 'smart pointers', we must pretend that this is a
		//  unique instance with its own self-possessed refcount

		//TODO image->ref();
		return image;
	}
	return NULL;
}

bool vtImage::Read(const char *fname, bool bAllowCache, bool progress_callback(int))
{
	m_b16bit = false;
	m_strFilename = fname;

	if( fname=="" || fname==0 )	return false;

	if( m_Image == osg::NullFC ) m_Image = osg::Image::create();

	if( m_Image != osg::NullFC ) {
		osg::PathHandler paths;
		paths.push_backPath(".");
		osg::ImagePtr img; img = osg::Image::create();
		osg::ImageFileHandler::the().setPathHandler(&paths);
		osg::ImageFileHandler::the().read( img, fname, 0 );
		m_Image = img;

		m_strFilename = fname;

		//TODO write it correctly, packing?
		m_iRowSize = m_Image->getWidth() * GetDepth()/8;
	}
	return(m_Image != osg::NullFC);
}

bool vtImage::WritePNG(const char *fname, bool progress_callback(int) )
{
	//pngimagefiletype changed in 1.8
	return false; //OSG::PNGImageFileType::the().write( m_Image, fname );
}

bool vtImage::WriteJPEG(const char *fname, int quality, bool progress_callback(int))
{
	//jpgimagefiletype changed in 1.8
	//OSG::JPGImageFileType::the().setQuality( quality );
	return false; //OSG::JPGImageFileType::the().write( m_Image, fname );
}

void vtImage::Release()
{
	//EXCEPT unref();
}

unsigned char * vtImage::GetRowData(int row) const
{
	unsigned char *data = m_Image->getData() + row*m_iRowSize;
	return data;
}

unsigned char vtImage::GetPixel8(int x, int y) const
{
	unsigned char *buf = m_Image->getData() + x + (m_Image->getHeight()-1-y)*m_iRowSize;
	//TODO via getPixel(idx) ?
	return *buf;
}

void vtImage::SetPixel8(int x, int y, unsigned char color)
{
	unsigned char *buf = m_Image->getData()+ x + (m_Image->getHeight()-1-y)*m_iRowSize;
	*buf = color;
}

void vtImage::GetPixel24(int x, int y, RGBi &rgb) const
{
	unsigned char *buf = m_Image->getData() + x*3 + (m_Image->getHeight()-1-y)*m_iRowSize;
	rgb.r = buf[0];
	rgb.g = buf[1];
	rgb.b = buf[2];
}

void vtImage::SetPixel24(int x, int y, const RGBi &rgb)
{
	unsigned char *buf = m_Image->getData() + x*3 + (m_Image->getHeight()-1-y)*m_iRowSize;
	buf[0] = rgb.r;
	buf[1] = rgb.g;
	buf[2] = rgb.b;
}

void vtImage::GetPixel32(int x, int y, RGBAi &rgba) const
{
	unsigned char *buf = m_Image->getData() + x*4 + (m_Image->getHeight()-1-y)*m_iRowSize;
	rgba.r = buf[0];
	rgba.g = buf[1];
	rgba.b = buf[2];
	rgba.a = buf[3];
}

void vtImage::SetPixel32(int x, int y, const RGBAi &rgba)
{
	unsigned char *buf = m_Image->getData() + x*4 + (m_Image->getHeight()-1-y)*m_iRowSize;
	buf[0] = rgba.r;
	buf[1] = rgba.g;
	buf[2] = rgba.b;
	buf[3] = rgba.a;
}

unsigned int vtImage::GetWidth() const
{
	return m_Image->getWidth();
}

unsigned int vtImage::GetHeight() const
{
	return m_Image->getHeight();
}

//mw depth==bitdepth.. opensg can handle 3d images...
unsigned int vtImage::GetDepth() const
{
	/*OSG_INVALID_PF = 0, OSG_I_PF = GL_INTENSITY, OSG_L_PF = GL_LUMINANCE, OSG_LA_PF = GL_LUMINANCE_ALPHA,
	  OSG_BGR_PF = 0, OSG_BGRA_PF = 0, OSG_RGB_DXT1 = GL_COMPRESSED_RGB_S3TC_DXT1_EXT, OSG_RGBA_DXT1 = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	  OSG_RGBA_DXT3 = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, OSG_RGBA_DXT5 = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, OSG_RGB_PF = GL_RGB, OSG_RGBA_PF = GL_RGBA */

	/*unsigned int depth(0);
	osg::Int32 pixelformat = m_Image->getPixelFormat();
	switch( pixelformat ) {
		case osg::Image::OSG_RGBA_PF:
			depth=32;
			break;
		case osg::Image::OSG_RGB_PF:
			depth=24;
			break;
		case GL_LUMINANCE:
			depth=8;
			break;
		default: ;
	};*/
	unsigned int depth(0);
	depth = m_Image->getBpp()*( m_Image->getDataType() == osg::Image::OSG_UINT8_IMAGEDATA ? 8 : 16 );
	assert(depth != 0);
	return depth;
}

/**
 * Call this method to tell vtlib that you want it to use a 16-bit texture
 * (internal memory format) to be sent to the graphics card.
 */
void vtImage::Set16Bit(bool bFlag)
{
	if( bFlag ) {
		// use a 16-bit internal
		if( m_Image->getPixelFormat() == GL_RGB ) {
			//_internalTextureFormat = GL_RGB5;
			beginEditCP(m_Image);
			//m_Image->reformat(GL_RGB5);
			endEditCP(m_Image);
		}
		if( m_Image->getPixelFormat() == GL_RGBA ) {
			//_internalTextureFormat = GL_RGB5_A1;
			beginEditCP(m_Image);
			//m_Image->reformat(GL_RGB5_A1);
			endEditCP(m_Image);
		}
	} else ;
	//just leave it as it is
	//_internalTextureFormat = _pixelFormat;

}

///////////////////////////////////////////////////////////////////////
// class vtOverlappedTiledImage

vtOverlappedTiledImage::vtOverlappedTiledImage()
{
	m_iTilesize = 0;
	m_iSpacing = 0;
	int r, c;
	for( r = 0; r < 4; r++ )
		for( c = 0; c < 4; c++ )
			m_Tiles[r][c] = NULL;
}

bool vtOverlappedTiledImage::Create(int iTilesize, int iBitDepth)
{
	// store the tile size
	m_iTilesize = iTilesize;
	m_iSpacing = iTilesize-1;

	// create the 4x4 grid of image tiles
	int r, c;
	for( r = 0; r < 4; r++ )
		for( c = 0; c < 4; c++ ) {
			vtImage *image = new vtImage;
			if( !image->Create(iTilesize, iTilesize, iBitDepth) )
				return false;
			m_Tiles[r][c] = image;
		}
	return true;
}

void vtOverlappedTiledImage::Release()
{
	int r, c;
	for( r = 0; r < 4; r++ )
		for( c = 0; c < 4; c++ ) {
			if( m_Tiles[r][c] != NULL )
				m_Tiles[r][c]->Release();
		}
}

bool vtOverlappedTiledImage::Load(const char *filename, bool progress_callback(int))
{
	g_GDALWrapper.RequestGDALFormats();

	GDALDataset *poDataset = (GDALDataset *) GDALOpen(filename, GA_ReadOnly);
	if( !poDataset )
		return false;

	bool mono = (poDataset->GetRasterCount() == 1);

	GDALRasterBand *poBand1;
	GDALRasterBand *poBand2;
	GDALRasterBand *poBand3;
	int xsize = poDataset->GetRasterXSize();
	int ysize = poDataset->GetRasterYSize();

	if( mono )
		poBand1 = poDataset->GetRasterBand(1);
	else {
		poBand1 = poDataset->GetRasterBand(1);
		poBand2 = poDataset->GetRasterBand(2);
		poBand3 = poDataset->GetRasterBand(3);
	}
	unsigned char *lineBuf1 = (unsigned char *) CPLMalloc(sizeof(char)*xsize);
	unsigned char *lineBuf2 = (unsigned char *) CPLMalloc(sizeof(char)*xsize);
	unsigned char *lineBuf3 = (unsigned char *) CPLMalloc(sizeof(char)*xsize);

	int x_off, y_off, x, y, i, j;

	for( i = 0; i < 4; i++ ) {
		x_off = i * m_iSpacing;
		for( j = 0; j < 4; j++ ) {
			if( progress_callback != NULL )
				progress_callback(((i*4)+j)*100 / (4*4));

			y_off = j * m_iSpacing;

			vtImage *target = m_Tiles[j][i];

			RGBi rgb;
			if( mono ) {
				for( x = 0; x < m_iTilesize; x++ ) {
					poBand1->RasterIO(GF_Read, 0, x_off+x, xsize, 1,
									  lineBuf1, xsize, 1, GDT_Byte, 0, 0);
					for( y = 0; y < m_iTilesize; y++ ) {
						unsigned char *targetBandVec1 = lineBuf1 + y_off + y;
						target->SetPixel8(x, y, *targetBandVec1);
					}
				}
			} else {
				for( x = 0; x < m_iTilesize; x++ ) {
					poBand1->RasterIO(GF_Read, 0, x_off+x, xsize, 1,
									  lineBuf1,xsize,1,GDT_Byte,0,0);
					poBand2->RasterIO(GF_Read, 0, x_off+x, xsize, 1,
									  lineBuf2,xsize,1,GDT_Byte,0,0);
					poBand3->RasterIO(GF_Read, 0, x_off+x, xsize, 1,
									  lineBuf3,xsize,1,GDT_Byte,0,0);

					for( y = 0; y < m_iTilesize; y++ ) {
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
	if( !poDataset )
		return false;
	width = poDataset->GetRasterXSize();
	height = poDataset->GetRasterYSize();
	depth = poDataset->GetRasterCount()*8;
	GDALClose(poDataset);
	return true;
}
