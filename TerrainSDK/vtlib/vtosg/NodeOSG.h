//
// NodeOSG.h
//
// Encapsulate behavior for OSG scene graph nodes
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_NODEH
#define VTOSG_NODEH

namespace osg
{
	class Node;
}

//////////////////////
// wrapper classes

class vtNode : public vtNodeBase, public vtEnabledBase
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
	void SetOsgNode(osg::Node *n, bool bGroup = false);
	osg::Node *GetOsgNode() { return m_pNode.get(); }
	bool IsGroup() { return m_bGroup; }

protected:
	//osg::Node *m_pNode;
	osg::ref_ptr<osg::Node> m_pNode;
	bool m_bGroup;
};

class vtGroup : public vtNode, public vtGroupBase
{
public:
	vtGroup(bool suppress = false);

	// implement vtGroupBase methods
	void AddChild(vtNode *pChild);
	void RemoveChild(vtNode *pChild);
	vtNode *GetChild(int num);
	int GetNumChildren();

	// OSG-specific Implementation
	osg::Group *GetOsgGroup() { return m_pGroup; }
protected:
	Array<vtNode*> m_VtChildren;
	void SetOsgGroup(osg::Group *g);

	osg::Group *m_pGroup;
};

class vtTransform : public vtGroup, public vtTransformBase
{
public:
	vtTransform();

	// implement vtTransformBase methods
	void Identity();
	void SetTrans(const FPoint3 &pos);
	FPoint3 GetTrans();
	void Translate1(const FPoint3 &pos);
	void TranslateLocal(const FPoint3 &pos);
	void Rotate2(const FPoint3 &axis, float angle);
	void RotateLocal(const FPoint3 &axis, float angle);
	void RotateParent(const FPoint3 &axis, float angle);
	void Scale3(float x, float y, float z);

	void SetTransform1(const FMatrix4 &mat);
	void GetTransform1(FMatrix4 &mat);
	void PointTowards(const FPoint3 &point);

	// OSG-specific Implementation
public:
	osg::Transform	*m_pTransform;
	FPoint3			m_Scale;
};

class vtRoot : public vtGroup
{
public:
	vtRoot() : vtGroup(true)
	{
		m_pOsgRoot = new osg::Group();
		SetOsgGroup(m_pOsgRoot);
	}

	osg::Group *m_pOsgRoot;
};

class vtLight : public vtNode
{
public:
	vtLight();
	void SetColor2(const RGBf &color);
	void SetAmbient2(const RGBf &color);

	osg::Light	*m_pLight;
};

class vtMovLight : public vtTransform
{
public:
	vtMovLight(vtLight *pContained);
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

	osg::Geode	*m_pGeode;	// the Geode is a container for Drawables
};

class vtMovGeom : public vtTransform
{
public:
	vtMovGeom(vtGeom *pContained) : vtTransform()
	{
		m_pGeom = pContained;
		AddChild(m_pGeom);
//		m_pGroup->addChild(m_pGeom->m_pGeode);
	}
	vtGeom	*m_pGeom;
};

class vtDynMesh : public osg::Drawable
{
public:
	// overrides
	virtual osg::Object* clone() const { return new vtDynMesh(); }
	virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const vtDynMesh*>(obj)!=NULL; }
	virtual const char* className() const { return "vtDynMesh"; }

	virtual const bool computeBound() const;
	virtual void drawImmediateMode(osg::State& state);

	osg::BoundingBox	m_bbox;
	class vtDynGeom		*m_pDynGeom;
};

class vtDynGeom : public vtGeom
{
public:
	vtDynGeom();

	// Tests a sphere or triangle, and return one of:
	//	0				- not in view
	//  VT_Visible		- partly in view
	//  VT_AllVisible	- entirely in view
	//
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

	void CalcCullPlanes();

	// for culling
	FPlane		m_cullPlanes[4];

protected:
	vtDynMesh	*m_pDynMesh;
};

//////////////////////////////////////////////////

class vtLOD : public vtGroup
{
public:
	vtLOD() : vtGroup(true)
	{
		m_pLOD = new osg::LOD();
		m_pLOD->setCenter(osg::Vec3(0, 0, 0));
		SetOsgGroup(m_pLOD);
	}

	void SetRanges(float *ranges, int nranges);
	void SetCenter(FPoint3 &center);

protected:
	osg::LOD	*m_pLOD;
};

class vtMovLOD : public vtTransform
{
public:
	vtMovLOD(vtLOD *pContained) : vtTransform()
	{
		m_pLOD = pContained;
		AddChild(m_pLOD);
	}
	vtLOD	*m_pLOD;
};

class vtCamera : public vtTransform
{
public:
	vtCamera();

	void SetHither(float f);
	float GetHither();
	void SetYon(float f);
	float GetYon();

	void SetFOV(float f);
	float GetFOV();

	void ZoomToSphere(const FSphere &sphere);

	void SetOrtho(float fWidth);

	osg::Camera	*m_pOsgCamera;
};

class vtBillBoard : public vtNode
{
public:
	vtBillBoard()
	{
		m_pBillboard = new osg::Billboard();
	}
	void SetAxis(FPoint3 &axis)
	{
//		SetAxis(v2d(axis));
	}

	osg::Billboard *m_pBillboard;
};

class vtSprite : public vtGroup
{
public:
	void SetText(const char *msg);
	void SetWindowRect(float l, float t, float r, float b) {}
};

#endif
