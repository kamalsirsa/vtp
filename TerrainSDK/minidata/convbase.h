// (c) by Stefan Roettger

#ifndef CONVBASE_H
#define CONVBASE_H

#include <mini/minibase.h>

#include <mini/database.h>

namespace convbase {

//! parameters for converting external formats
struct MINI_CONVERSION_HOOK_STRUCT
   {
   float jpeg_quality;

   BOOLINT usegreycstoration;

   float greyc_p;
   float greyc_a;
   };

typedef MINI_CONVERSION_HOOK_STRUCT MINI_CONVERSION_PARAMS;

//! set conversion hook for external formats
void setconversion(MINI_CONVERSION_PARAMS *params);

//! libMini conversion hook for external formats (JPEG/PNG)
int conversionhook(int israwdata,unsigned char *srcdata,unsigned int bytes,unsigned int extformat,unsigned char **newdata,unsigned int *newbytes,databuf *obj,void *data);

}

using namespace convbase;

#endif
