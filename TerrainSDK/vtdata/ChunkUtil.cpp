//
// ChunkUtil.cpp: support routines for the ChunkLOD code
//

#include <stdio.h>
#include "ChunkUtil.h"

int WriteUint32(FILE *fp, unsigned int val)
{
	return fwrite(&val, 4, 1, fp);
}

int WriteUint16(FILE *fp, unsigned short val)
{
	return fwrite(&val, 2, 1, fp);
}

int WriteFloat32(FILE *fp, float val)
{
	return fwrite(&val, 4, 1, fp);
}

void WriteByte(FILE *fp, unsigned char b)
{
	fwrite(&b, 1, 1, fp);
}

unsigned int ReadUint32(FILE *fp)
{
	unsigned int i;
	fread(&i, 4, 1, fp);
	return i;
}

unsigned short ReadUint16(FILE *fp)
{
	unsigned short s;
	fread(&s, 2, 1, fp);
	return s;
}

double ReadDouble64(FILE *fp)
{
	double d;
	fread(&d, 8, 1, fp);
	return d;
}

//
// Return the number of nodes in a fully populated quadtree of the specified depth.
//
int quadtree_node_count(int depth)
{
	return 0x55555555 & ((1 << depth*2) - 1);
}



