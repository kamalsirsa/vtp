//
// vtTin.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTTINH
#define VTTINH

#include "MathTypes.h"
#include "Projections.h"

typedef Array<int> Bin;


class vtTin
{
public:
	int NumVerts() { return m_vert.GetSize(); }
	int NumTris() { return m_tri.GetSize()/3; }

	void AddVert(const DPoint2 &p, float z);
	void AddTri(int i1, int i2, int i3);

	bool Read(const char *fname);
	bool Write(const char *fname);

	bool GetExtents(DRECT &rect, float &minheight, float &maxheight);
	void Offset(const DPoint2 &p);
	bool FindAltitudeAtPoint(const DPoint2 &p, float &fAltitude);
	bool ConvertProjection(vtProjection &proj_new);

	void MergeSharedVerts(void progress_callback(int) = NULL);

	vtProjection	m_proj;

protected:
	bool _ReadTin(FILE *fp);
	bool _ReadTinOld(FILE *fp);

//	void _ChangeVertRefs(int from, int to);
//	void _RemoveVert(int kill);
	void _UpdateIndicesInInBin(int bin);
	void _CompareBins(int bin1, int bin2);

	DLine2		 m_vert;
	Array<float> m_z;
	Array<int>	 m_tri;

	// used only during MergeSharedVerts
	int *m_bReplace;
	Bin *m_vertbin;
	Bin *m_tribin;
};


#endif // VTTINH
