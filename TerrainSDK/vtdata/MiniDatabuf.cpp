//
// Copyright (c) 2006 Virtual Terrain Project and Stefan Roettger
// Free for all uses, see license.txt for details.

#include "MiniDatabuf.h"
#include "vtLog.h"
#include "Projections.h"
#include "MathTypes.h"
#include "FilePath.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define ERRORMSG() VTLOG1("Error!")

int MiniDatabuf::MAGIC=12640;

unsigned short int MiniDatabuf::INTEL_CHECK=1;

// default constructor
MiniDatabuf::MiniDatabuf()
   {
   xsize=ysize=zsize=0;
   tsteps=0;
   type=0;

   swx=swy=0.0f;
   nwx=nwy=0.0f;
   nex=ney=0.0f;
   sex=sey=0.0f;
   h0=dh=0.0f;
   t0=dt=0.0f;

   scaling=1.0f;
   bias=0.0f;

   data=NULL;
   bytes=0;
   }

MiniDatabuf::~MiniDatabuf()
{
	release();
}

// allocate a new memory chunk
void MiniDatabuf::alloc(unsigned int xs,unsigned int ys,unsigned int zs,unsigned int ts,unsigned int ty)
   {
   unsigned int bs,cs;

   unsigned char *byteptr;
   short int *shortptr;
   float *floatptr;

   unsigned int count;

   if (ty==0) bs=1;
   else if (ty==1) bs=2;
   else if (ty==2) bs=4;
   else if (ty==3) bs=3;
   else if (ty==4) bs=3;
   else if (ty==5) bs=4;
   else if (ty==6) bs=4;
   else ERRORMSG();

   cs=xs*ys*zs*ts;
   bs*=cs;

   if (ty==4 || ty==6) bs/=6;

   if ((data=malloc(bs))==NULL) ERRORMSG();

   if (ty==1)
      for (shortptr=(short int *)data,count=0; count<cs; count++) *shortptr++=0;
   else if (ty==2)
      for (floatptr=(float *)data,count=0; count<cs; count++) *floatptr++=0.0f;
   else
      for (byteptr=(unsigned char *)data,count=0; count<bs; count++) *byteptr++=0;

   bytes=bs;

   xsize=xs;
   ysize=ys;
   zsize=zs;
   tsteps=ts;
   type=ty;
   }

// reset buffer
void MiniDatabuf::reset()
   {
   data=NULL;
   bytes=0;
   }

// release buffer
void MiniDatabuf::release()
   {
   if (data!=NULL) free(data);

   data=NULL;
   bytes=0;
   }

// data is saved in MSB format
void MiniDatabuf::savedata(const char *filename)
   {
   FILE *file;

   if (data==NULL) return;

   if (bytes==0) ERRORMSG();

   if ((file=vtFileOpen(filename,"wb"))==NULL) ERRORMSG();

   // save magic identifier
   fprintf(file,"MAGIC=%d\n",MAGIC);

   // save mandatory metadata
   fprintf(file,"xsize=%d\n",xsize);
   fprintf(file,"ysize=%d\n",ysize);
   fprintf(file,"zsize=%d\n",zsize);
   fprintf(file,"tsteps=%d\n",tsteps);
   fprintf(file,"type=%d\n",type);

   // save optional metadata
   fprintf(file,"swx=%f\n",swx);
   fprintf(file,"swy=%f\n",swy);
   fprintf(file,"nwx=%f\n",nwx);
   fprintf(file,"nwy=%f\n",nwy);
   fprintf(file,"nex=%f\n",nex);
   fprintf(file,"ney=%f\n",ney);
   fprintf(file,"sex=%f\n",sex);
   fprintf(file,"sey=%f\n",sey);
   fprintf(file,"h0=%f\n",h0);
   fprintf(file,"dh=%f\n",dh);
   fprintf(file,"t0=%f\n",t0);
   fprintf(file,"dt=%f\n",dt);

   // save optional scaling
   fprintf(file,"scaling=%f\n",scaling);
   fprintf(file,"bias=%f\n",bias);

   // save length of data chunk
   fprintf(file,"bytes=%u\n",bytes);

   // write zero byte
   fputc(0, file);

   // save data chunk
   if (*((unsigned char *)(&INTEL_CHECK))==0)
      {
      if (fwrite(data,bytes,1,file)!=1) ERRORMSG();
      fclose(file);
      }
   else
      {
      swapbytes();
      if (fwrite(data,bytes,1,file)!=1) ERRORMSG();
      swapbytes();

      fclose(file);
      }
   }

// swap byte ordering between MSB and LSB
void MiniDatabuf::swapbytes()
   {
   unsigned int i,b;

   unsigned char *ptr,tmp;

   if (type==0 || (type>=3 && type<=6)) return;

   if (type==1) b=2;
   else if (type==2) b=4;
   else ERRORMSG();

   if (bytes==0 || bytes%b!=0) ERRORMSG();

   ptr=(unsigned char *)data+bytes;

   while (ptr!=data)
      {
      ptr-=b;

      for (i=0; i<(b>>1); i++)
         {
         tmp=ptr[i];
         ptr[i]=ptr[b-1-i];
         ptr[b-1-i]=tmp;
         }
      }
   }

bool WriteTilesetHeader(const char *filename, int cols, int rows, int lod0size,
						const DRECT &area, const vtProjection &proj,
						float minheight, float maxheight)
{
	FILE *fp = vtFileOpen(filename, "wb");
	if (!fp)
		return false;

	fprintf(fp, "[TilesetDescription]\n");
	fprintf(fp, "Columns=%d\n", cols);
	fprintf(fp, "Rows=%d\n", rows);
	fprintf(fp, "LOD0_Size=%d\n", lod0size);
	fprintf(fp, "Extent_Left=%.16lg\n", area.left);
	fprintf(fp, "Extent_Right=%.16lg\n", area.right);
	fprintf(fp, "Extent_Bottom=%.16lg\n", area.bottom);
	fprintf(fp, "Extent_Top=%.16lg\n", area.top);
	// write CRS, but pretty it up a bit
	OGRSpatialReference *poSimpleClone = proj.Clone();
	poSimpleClone->GetRoot()->StripNodes( "AXIS" );
	poSimpleClone->GetRoot()->StripNodes( "AUTHORITY" );
	char *wkt;
	poSimpleClone->exportToWkt(&wkt);
	fprintf(fp, "CRS=%s\n", wkt);
	OGRFree(wkt);	// Free CRS
	delete poSimpleClone;

	// For elevation tilesets, also write vertical extents
	if (minheight != INVALID_ELEVATION)
	{
		fprintf(fp, "Elevation_Min=%.f\n", minheight);
		fprintf(fp, "Elevation_Max=%.f\n", maxheight);
	}

	fclose(fp);

	return true;
}

