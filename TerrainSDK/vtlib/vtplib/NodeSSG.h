//
// NodeSSG.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTPLIB_MOVABLEH
#define VTPLIB_MOVABLEH

#include "core/Base.h"
//#include "core/Mesh.h"

///////////////////////
// math helpers

void ConvertMatrix4(const sgMat4 mat_sg, FMatrix4 *mat);
void ConvertMatrix4(const FMatrix4 *mat, sgMat4 mat_sg);

inline void v2s(const FPoint2 &f, sgVec2 &s) { s[0] = f.x; s[1] = f.y; }
inline void v2s(const FPoint3 &f, sgVec3 &s) { s[0] = f.x; s[1] = -f.z; s[2] = f.y; }
inline void v2s(const RGBf &f, sgVec4 &s) { s[0] = f.r; s[1] = f.g; s[2] = f.b; s[3] = 1.0f; }

inline s2v(const float *s, FPoint3 &f) { f.x = s[0]; f.y = s[2]; f.z = -s[1]; }
inline s2v(const float *s, FPoint2 &f) { f.x = s[0]; f.y = s[1]; }
inline s2v(const float *s, RGBf &f) { f.r = s[0]; f.g = s[1]; f.b = s[2]; }

inline void s2v(sgSphere *s, FSphere &sph)
{
	const sgFloat *center = s->getCenter();
	sph.center.x = center[0];
	sph.center.y = center[1];
	sph.center.z = center[2];
	sph.radius = s->getRadius();
}

//////////////////////
// wrapper classes

class vtNode : public vtNodeBase, public vtEnabledBase, public ssgBase
{
public:
	// implement vtEnabledBase methods
	void SetEnabled(bool bOn);

	// implement vtNodeBase methods
	void GetBoundBox(FBox3 &box);
	void GetBoundSphere(FSphere &sphere);
	vtNode *CreateClone();

	void SetName2(const char *str);
	const char *GetName2();

	// implementation data
	void SetSsgEntity(ssgEntity *ent);
	ssgEntity *GetSsgEntity() { return m_pEntity; }

protected:
	ssgEntity	*m_pEntity;
};

class vtGroup : public vtGroupBase, public vtNode
{
public:
	vtGroup(bool suppress = false);

	// implement vtGroupBase methods
	void AddChild(vtNode *pChild);
	void RemoveChild(vtNode *pChild);
	vtNode *GetChild(int num);
	int GetNumChildren();

	// SSG-specific Implementation
	void SetBranch(ssgBranch *pBranch)
	{
		m_pBranch = pBranch;
		SetSsgEntity(pBranch);
	}
	ssgBranch *GetBranch() { return m_pBranch; }
protected:

	ssgBranch	*m_pBranch;
};

class vtTransform : public vtGroup, public vtTransformBase
{
public:
	vtTransform();

	// implement vtTransformBase methods
	void Identity();
	FPoint3 GetTrans();
	void SetTrans(const FPoint3 &pos);
	void Translate1(const FPoint3 &pos);
	void TranslateLocal(const FPoint3 &pos);
	void Rotate2(const FPoint3 &axis, float angle);
	void RotateLocal(const FPoint3 &axis, float angle);
	void RotateParent(const FPoint3 &axis, float angle);
	void Scale3(float x, float y, float z);

	void SetTransform1(const FMatrix4 &mat);
	void GetTransform1(FMatrix4 &mat);
	void GetTransform1(const FMatrix4 &mat);
	void PointTowards(const FPoint3 &point);

	// SSG-specific Implementation
	ssgTransform *GetSsgTransform() { return &m_Transform; }

protected:
	ssgTransform	m_Transform;
	FPoint3			m_Scale;
};

class vtLight : public vtNode
{
public:
	vtLight();
	void SetColor2(const RGBf &color);
	void SetAmbient2(const RGBf &color);

	// TODO: write code which copies the transform to the light
	ssgLight	*m_pLight;
};

class vtMovLight : public vtTransform
{
public:
	vtMovLight(vtLight *pContained) : vtTransform()
	{
		m_pLight = pContained;
		AddChild(m_pLight);
	}
	vtLight *GetLight() { return m_pLight; }
	vtLight	*m_pLight;
};

class vtGeom : public vtGeomBase, public vtNode
{
public:
	vtGeom();

	void AddMesh(vtMesh *pMesh, int iMatIdx);
	void RemoveMesh(vtMesh *pMesh);
	vtMesh *GetMesh(int i);
	int GetNumMeshes();
	void SetMeshMatIndex(vtMesh *pMesh, int iMatIdx);

	ssgBranch	m_Branch;
};

class vtMovGeom : public vtTransform
{
public:
	vtMovGeom(vtGeom *pContained) : vtTransform()
	{
		m_pGeom = pContained;
		AddChild(m_pGeom);
	}
	vtGeom	*m_pGeom;
};

class vtDynGeom : public vtGeom
{
public:
	vtDynGeom();

	// culling
	int IsVisible(const FSphere &sphere) const;
	int IsVisible(const FPoint3 &point0,
					const FPoint3 &point1,
					const FPoint3 &point2,
					const float fTolerance = 0.0f) const;
	int IsVisible(const FPoint3 &point, float radius);

	// Tests a single point, returns true if in view
	bool IsVisible(const FPoint3 &point) const;

	// vt methods (must be overriden)
	virtual void DoRender() = 0;
	virtual void DoCalcBoundBox(FBox3 &box) = 0;
	virtual void DoCull(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov) = 0;

	// for culling
	FPlane		cullPlanes[4];

	// PLIB implementation
	void CalcCullPlanes(sgFrustum *pFrust, const FMatrix4* trans);
protected:
	// override for ssgEntity::cull_test
//	virtual ssgCullResult cull_test(sgFrustum *f, sgMat4 m, int test_needed);

};

class DynLeaf : public ssgVtxTable
{
public:
	// overrides of ssgLeaf methods
	virtual void cull(sgFrustum *f, sgMat4 m, int test_needed);
	virtual void draw();
	virtual void recalcBSphere();
	virtual void print(FILE *fd = stderr, char *indent = "" );

	sgBox bbox;
	vtDynGeom *m_pDynGeom;
};

//////////////////////////////////////////////////

class vtLOD : public vtGroup
{
public:
	vtLOD() : vtGroup() { SetBranch(&m_Selector); }

	void SetRanges(float *ranges, int nranges);
	void SetCenter(FPoint3 &center);

protected:
	ssgRangeSelector	m_Selector;
};

class vtCamera : public vtTransform
{
public:
	void SetHither(float f);
	float GetHither();
	void SetYon(float f);
	float GetYon();

	void SetFOV(float f);
	float GetFOV();

	void ZoomToSphere(const FSphere &sphere);

	void SetOrtho(float fWidth);

//  context:
//	void getCameraPosition ( sgVec3 pos ) ;
//	void setCamera ( sgMat4 mat ) ;
//	void setCamera ( sgCoord *coord ) ;
};


class vtSprite : public vtTransform
{
public:
	void SetText(const char *msg);
	void SetWindowRect(float l, float t, float r, float b) {}
};

#endif
