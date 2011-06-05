//
// vtSOG.h
//
// header for vtlib implementation of SOG file IO.
//

#ifndef VTSOGH
#define VTSOGH

#include <stdio.h>
#include "vtdata/SOG.h"
#include "vtdata/vtString.h"

/**
 * A class to support reading vtlib scene graphs from .sog files.
 *
 * The .sog format is a tentative, preliminary implementation of an
 * efficient binary format for geometry (and nested geometry) nodes.
 */
class InputSOG
{
public:
	bool ReadHeader(FILE *fp, int &num_geom);
	bool ReadContents(FILE *fp, vtGroup *Parent);

private:
	bool Read(FILE *fp, short &token, short &len);
	bool ReadBool(FILE *fp);
	vtMaterial *ReadMaterial(FILE *fp);
	vtMesh *ReadMesh(FILE *fp);
	vtGeode *ReadGeometry(FILE *fp, vtMaterialArray *pMats);
};

/**
 * A class to support writing vtlib scene graphs from .sog files.
 *
 * The .sog format is a tentative, preliminary implementation of an
 * efficient binary format for geometry (and nested geometry) nodes.
 */
class OutputSOG
{
public:
	void WriteHeader(FILE *fp);
	void WriteSingleGeometry(FILE *fp, const vtGeode *pGeode);
	void WriteMultiGeometry(FILE *fp, const vtGroup *pParent);

private:
	void Write(FILE *fp, FileToken ft, RGBf &rgb);
	void Write(FILE *fp, FileToken ft, RGBAf &rgba);
	void Write(FILE *fp, FileToken ft, bool &b);
	void Write(FILE *fp, FileToken ft, vtString &str);
	void Write(FILE *fp, FileToken ft, short &s1);
	void Write(FILE *fp, FileToken ft, FPoint3 &p);
	void Write(FILE *fp, FileToken ft, FPoint2 &p);
	void WriteToken(FILE *fp, FileToken ft, short length);

	void WriteGeometry(FILE *fp, const vtGeode *pGeode, short id);
	void WriteMaterials(FILE *fp, const vtMaterialArray *pMats);
	void WriteMaterial(FILE *fp, const vtMaterial *pMat);
	void WriteMesh(FILE *fp, const vtMesh *pMesh);
};

#endif // VTSOGH


