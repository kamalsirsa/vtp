//
// ByteOrder.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include "ByteOrder.h"

static int GetDataTypeSize( DataType type )
{
	int tsize;
	switch ( type )
	{
		case DT_SHORT:	tsize = sizeof(short);	break;
		case DT_INT:	tsize = sizeof(int);	break;
		case DT_LONG:	tsize = sizeof(long);	break;
		case DT_FLOAT:	tsize = sizeof(float);	break;
		case DT_DOUBLE:	tsize = sizeof(double);	break;
		default: assert(false);
	}
	return tsize;
}


/**
 * If the byte orders differ, swap bytes; if not, don't; return the result.
 * This is the memory buffer version of the SwapBytes() macros, and as such
 * supports an array of data.  It also parametizes the element type to avoid
 * function explosion.
 * \param items the data items to order
 * \param type the type of each item in the array
 * \param nitems the number if items in the array
 * \param data_order the byte order of the data
 * \param desired_order the desired byte ordering
 *
 */
void SwapMemBytes( void *items, DataType type, size_t nitems,
				   ByteOrder data_order, ByteOrder desired_order )
{
	size_t tsize;
	char  *base = (char *) items,
		*p;

	if ( data_order	== BO_MACHINE ) data_order	= NativeByteOrder();
	if ( desired_order == BO_MACHINE ) desired_order = NativeByteOrder();
	if ( data_order == desired_order )
		return;

	tsize = GetDataTypeSize( type );

	switch ( type )
	{
		case DT_SHORT  :
			for ( p = base + (nitems-1) * tsize; p >= base; p -= tsize )
				*(short *)p = SwapShort( *(short *)p );
			break;
		case DT_INT	:
			for ( p = base + (nitems-1) * tsize; p >= base; p -= tsize )
				*(int   *)p = SwapInt( *(int   *)p );
			break;
		case DT_LONG   :
			for ( p = base + (nitems-1) * tsize; p >= base; p -= tsize )
				*(long  *)p = SwapLong( *(long  *)p );
			break;
		case DT_FLOAT  :
			for ( p = base + (nitems-1) * tsize; p >= base; p -= tsize )
				*(float *)p = SwapFloat( *(float *)p );
			break;
		case DT_DOUBLE :
			for ( p = base + (nitems-1) * tsize; p >= base; p -= tsize )
				*(double *)p = SwapDouble( *(double *)p );
			break;
		default:
			assert(false);
	}
}


/**
 * Like stdio's fread(), but adds an optional byte swapping phase for
 * the data read.
 * \param ptr data buffer to read items into
 * \param type the data type of items to be read
 * \param nitems the number of items to read
 * \param stream the stdio stream open for read
 * \param file_order the byte ordering of data read from the file
 * \param desired_order the desired byte ordering
 * \return fread() return value (num items read, or negative for error)
 *
 */
size_t FRead( void *ptr, DataType type, size_t nitems, FILE *stream,
			  ByteOrder file_order, ByteOrder desired_order )
{
	int tsize  = GetDataTypeSize( type );
	size_t ret = fread( ptr, tsize, nitems, stream );

	if ( (int)ret >= 0 )
		SwapMemBytes( ptr, type, ret/tsize, file_order, desired_order );
	return ret;
}


/**
 * Like stdio's fread(), but adds an optional byte swapping phase for
 * the data read. File access is done via zlib's gzip IO routines to
 * be compatible with gzopen(), etc.
 * \param ptr data buffer to read items into
 * \param type the data type of items to be read
 * \param nitems the number of items to read
 * \param stream the stdio stream open for read
 * \param file_order the byte ordering of data read from the file
 * \param desired_order the desired byte ordering
 * \return fread() return value (num items read, or negative for error)
 *
 */
size_t GZFRead( void *ptr, DataType type, size_t nitems, gzFile gzstream,
			  ByteOrder file_order, ByteOrder desired_order )
{
	int tsize  = GetDataTypeSize( type );
	size_t ret = gzread(gzstream, ptr, tsize * nitems);

	if ( (int)ret >= 0 )
		SwapMemBytes( ptr, type, ret/tsize, file_order, desired_order );
	return ret;
}
