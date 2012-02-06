//
// minidata.cpp - interface to the data structures used by the 'mini' library
//
// Copyright (c) 2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// jpeg/png/zlib*.cpp contributed as part the libMini library by Stefan Roettger
//

#include "jpegbase.h"
#include "pngbase.h"
#include "zlibbase.h"

#include <mini/database.h> // for databuf

// parameters for converting external formats
struct MINI_CONVERSION_HOOK_STRUCT
{
	float jpeg_quality;

	float png_gamma;
	int zlib_level;
};

typedef MINI_CONVERSION_HOOK_STRUCT MINI_CONVERSION_PARAMS;

// libMini conversion hook for external formats (JPEG/PNG/Z)
int conversionhook(int israwdata,unsigned char *srcdata,unsigned int bytes,unsigned int extformat,
                   unsigned char **newdata,unsigned int *newbytes,
                   databuf *obj,void *data)
{
	MINI_CONVERSION_PARAMS *conversion_params=(MINI_CONVERSION_PARAMS *)data;

	if (conversion_params==NULL) return(0);

	switch (extformat)
	{
	case databuf::DATABUF_EXTFMT_JPEG:

		if (israwdata==0)
		{
			int width,height,components;

			*newdata=jpegbase::decompressJPEGimage(srcdata,bytes,&width,&height,&components);

			if (*newdata==NULL) return(0); // return failure

			switch (components)
			{
			case 1: if (obj->type!=0) return(0); break;
			case 3: if (obj->type!=3) return(0); break;
			case 4: if (obj->type!=4) return(0); break;
			default: return(0);
			}

			*newbytes=width*height*components;
		}
		else
		{
			int components;

			switch (obj->type)
			{
			case 0: components=1; break;
			case 3: components=3; break;
			case 4: components=4; break;
			default: return(0); // return failure
			}

			jpegbase::compressJPEGimage(srcdata,obj->xsize,obj->ysize,components,conversion_params->jpeg_quality/100.0f,newdata,newbytes);

			if (*newdata==NULL) return(0); // return failure
		}

		break;

	case databuf::DATABUF_EXTFMT_PNG:

		if (israwdata==0)
		{
			int width,height,components;

			*newdata=pngbase::decompressPNGimage(srcdata,bytes,&width,&height,&components);

			if (*newdata==NULL) return(0); // return failure

			switch (components)
			{
			case 1: if (obj->type!=0) return(0); break;
			case 2: if (obj->type!=1) return(0); break;
			case 3: if (obj->type!=3) return(0); break;
			case 4: if (obj->type!=4) return(0); break;
			default: return(0);
			}

			*newbytes=width*height*components;
		}
		else
		{
			int components;

			switch (obj->type)
			{
			case 0: components=1; break;
			case 1: components=2; break;
			case 3: components=3; break;
			case 4: components=4; break;
			default: return(0); // return failure
			}

			pngbase::compressPNGimage(srcdata,obj->xsize,obj->ysize,components,newdata,newbytes,conversion_params->png_gamma,conversion_params->zlib_level);

			if (*newdata==NULL) return(0); // return failure
		}

		break;

	case databuf::DATABUF_EXTFMT_Z:

		if (israwdata==0)
		{
			*newdata=zlibbase::decompressZLIB(srcdata,bytes,newbytes);

			if (*newdata==NULL) return(0); // return failure
		}
		else
		{
			zlibbase::compressZLIB(srcdata,bytes,newdata,newbytes,conversion_params->zlib_level);

			if (*newdata==NULL) return(0); // return failure
		}

		break;

	default: return(0);
	}

	return(1); // return success
}

void InitMiniConvHook(int iJpegQuality)
{
	// specify conversion parameters
	static MINI_CONVERSION_PARAMS conversion_params;
	conversion_params.jpeg_quality = (float) iJpegQuality; // jpeg quality in percent
	conversion_params.png_gamma=0.0f; // png gamma (0.0=default 1.0=neutral)
	conversion_params.zlib_level=9; // zlib compression level (0=none 6=standard 9=highest)

	// register libMini conversion hook (JPEG/PNG)
	databuf::setconversion(conversionhook,&conversion_params);
}
