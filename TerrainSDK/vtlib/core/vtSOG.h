//
// vtSOG.h
//
// header for vtlib implementation of SOG file IO.
//

#ifndef VTSOGH
#define VTSOGH

#include <stdio.h>
#include "vtdata/SOG.h"

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
	vtGeom *ReadGeometry(FILE *fp, vtMaterialArray *pMats);
};

class OutputSOG
{
public:
	void WriteHeader(FILE *fp);
	void WriteSingleGeometry(FILE *fp, vtGeom *pGeom);
	void WriteMultiGeometry(FILE *fp, vtGroup *pParent);

private:
	void Write(FILE *fp, FileToken ft, RGBf &rgb);
	void Write(FILE *fp, FileToken ft, RGBAf &rgba);
	void Write(FILE *fp, FileToken ft, bool &b);
	void Write(FILE *fp, FileToken ft, vtString &str);
	void Write(FILE *fp, FileToken ft, short &s1);
	void Write(FILE *fp, FileToken ft, FPoint3 &p);
	void Write(FILE *fp, FileToken ft, FPoint2 &p);
	void WriteToken(FILE *fp, FileToken ft, short length);

	void WriteGeometry(FILE *fp, vtGeom *pGeom, short id);
	void WriteMaterials(FILE *fp, vtMaterialArray *pMats);
	void WriteMaterial(FILE *fp, vtMaterial *pMat);
	void WriteMesh(FILE *fp, vtMesh *pMesh);
};

#endif // VTSOGH