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
	class Referenced;
	class Matrix;
}

//////////////////////
// wrapper classes

/**
 * Represents a Node in the vtlib Scene Graph.
 */
class vtNode : public vtNodeBase, public vtEnabledBase, public osg::Referenced
{
public:
	~vtNode();

	// implement vtEnabledBase methods
	void SetEnabled(bool bOn);

	// implement vtNodeBase methods
	/** Get the Bounding Box of the node, in world coordinates */
	void GetBoundBox(FBox3 &box);

	/** Get the Bounding Sphere of the node, in world coordinates */
	void GetBoundSphere(FSphere &sphere);

	vtNode *CreateClone();

	/** Set the name of the node. */
	void SetName2(const char *str);
	/** Get the name of the node. */
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

/**
 * Represents a Group (a node that can have children) in the vtlib Scene Graph.
 */
class vtGroup : public vtNode, public vtGroupBase
{
public:
	vtGroup(bool suppress = false);

	// implement vtGroupBase methods
	/** Add a node as a child of this Group. */
	void AddChild(vtNode *pChild);

	/** Remove a node as a child of this Group.  If the indicated node is not
	 a child, then this method has no effect. */
	void RemoveChild(vtNode *pChild);

	/** Return a child node, by index. */
	vtNode *GetChild(int num);

	/** Return the number of child nodes */
	int GetNumChildren();

	// OSG-specific Implementation
	osg::Group *GetOsgGroup() { return m_pGroup; }
protected:
	Array<vtNode*> m_VtChildren;
	void SetOsgGroup(osg::Group *g);

	osg::Group *m_pGroup;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

class CustomTransform : public osg::Transform
{
public:
	inline osg::Matrix& getMatrix() { return *_matrix; }
};

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
 * A Transform node allows you to apply a transform (scale, rotate, translate)
 * to all its child nodes.
 */
class vtTransform : public vtGroup, public vtTransformBase
{
public:
	vtTransform();

	// implement vtTransformBase methods

	/** Set this transform to identity (no scale, rotation, or translation). */
	void Identity();

	/** Set the translation component of the transform */
	void SetTrans(const FPoint3 &pos);

	/** Get the translation component of the transform */
	FPoint3 GetTrans();

	/** Apply a relative offset (translation) to the transform, in the frame
		of its parent. */
	void Translate1(const FPoint3 &pos);

	/** Apply a relative offset (translation) to the transform, in its own
		frame of reference. */
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
	CustomTransform	*m_pTransform;
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

class vtTextMesh;

/** The vtGeom class represents a Geometry Node which can contain any number
	of visible vtMesh objects.
	\par
	A vtGeom also manages a set of Materials (vtMaterial).  Each contained
	mesh is assigned one of these materials, by index.
	\par
	This separation (Group/Mesh) provides the useful ability to define a vtMesh
	once in memory, and have multiple vtGeom nodes which contain it, which
	permits a large number of visual instances (each with potentially different
	material and transform) with very little memory cost.
 */
class vtGeom : public vtGeomBase, public vtNode
{
public:
	vtGeom();
	~vtGeom();

	/** Add a mesh to this geometry.
		\param pMesh The mesh to add
		\param iMatIdx The material index for this mesh, which is an index
			into the material array of the geometry. */
	void AddMesh(vtMesh *pMesh, int iMatIdx);

	/** Remove a mesh from the geomtry.  Has no effect if the mesh is not
		currently contained. */
	void RemoveMesh(vtMesh *pMesh);

	/** Add a text mesh to this geometry.
		\param pMesh The mesh to add
		\param iMatIdx The material index for this mesh, which is an index
			into the material array of the geometry. */
	void AddText(vtTextMesh *pMesh, int iMatIdx);

	/** Return a contained vtMesh by index. */
	vtMesh *GetMesh(int i);

	/** Return the number of contained meshes. */
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
	}
	vtGeom	*m_pGeom;
};

class vtDynMesh : public osg::Drawable
{
public:
	vtDynMesh();

	// overrides
	virtual osg::Object* cloneType() const { return new vtDynMesh(); }
	virtual osg::Object* clone(const osg::CopyOp &foo) const { return new vtDynMesh(); }
	virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const vtDynMesh*>(obj)!=NULL; }
	virtual const char* className() const { return "vtDynMesh"; }

	virtual const bool computeBound() const;
	virtual void drawImmediateMode(osg::State& state);

	class vtDynGeom		*m_pDynGeom;
};

/**
 * vtDynGeom extends the vtGeom class with the ability to have dynamic geometry
 * which changes every frame.  The most prominent use of this feature is to do
 * Continuous Level of Detail (CLOD) for terrain.
 * \par
 * To implement, you must create your own subclass and override the following
 * methods:
 * - DoRender()
 * - DoCalcBoundBox()
 * - DoCull()
 * \par
 * Many helpful methods are provided to make doing your own view culling very easy:
 * - IsVisible(sphere)
 * - IsVisible(triangle)
 * - IsVisible(point)
 * \par
 * \see vtDynTerrainGeom
 */
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

/**
 * An LOD node controls the visibility of its child nodes.
 *
 * You should set a distance value (range) for each child, which determines
 * at what distance from the camera a node should be rendered.
 */
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
