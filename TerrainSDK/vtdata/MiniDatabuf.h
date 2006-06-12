// (c) VTP and Stefan Roettger

#ifndef MINIDATABUF_H
#define MINIDATABUF_H

#include <stdio.h>

/* This is a subset of a libMini class which represents a single 1D, 2D, 3D
	or 4D data buffer, with associated metadata such as dimensions and type. */

class MiniDatabuf
{
public:
	//! default constructor
	MiniDatabuf();
	~MiniDatabuf();

	// mandatory metadata
	unsigned int xsize;  // data size along the x-axis of 1D data
	unsigned int ysize;  // optional data size along the y-axis of 2D data
	unsigned int zsize;  // optional data size along the z-axis of 3D data
	unsigned int tsteps; // optional number of frames for time-dependent data
	unsigned int type;   // 0 = unsigned byte, 1 = signed short, 2 = float, 3 = RGB, 4 = RGBA, 5 = compressed RGB, 6 = compressed RGBA

	// optional metadata
	float swx,swy; // SW corner of data brick
	float nwx,nwy; // NW corner of data brick
	float nex,ney; // NE corner of data brick
	float sex,sey; // SE corner of data brick
	float h0,dh;   // base elevation and height of data brick
	float t0,dt;   // time frame start and exposure time

	// optional scaling
	float scaling; // scale factor of data values, default=1.0f
	float bias;    // bias of data values, default=0.0f

	// data chunk
	void *data;         // pointer to raw data, null pointer indicates missing data
	unsigned int bytes; // number of raw data bytes

	// allocate a new memory chunk
	void alloc(unsigned int xs,unsigned int ys,unsigned int zs,unsigned int ts=1,unsigned int ty=0);

	// reset buffer
	void reset();

	// release buffer
	void release();

	// native input/output
	void savedata(const char *filename); // data is saved in MSB format

protected:
	static int MAGIC;

private:
	static unsigned short int INTEL_CHECK;
	void swapbytes();
};

// Helper to write headers
class DRECT;
class vtProjection;
bool WriteTilesetHeader(const char *filename, int cols, int rows, int lod0size,
						const DRECT &area, const vtProjection &proj);

#endif
