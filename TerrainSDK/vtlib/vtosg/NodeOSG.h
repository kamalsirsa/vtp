//
// NodeOSG.h
//
// Encapsulate behavior for OSG scene graph nodes.
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_NODEH
#define VTOSG_NODEH

namespace osg
{
	class Node;
	class Referenced;
	class Matrixf;
	class Fog;
}

/** \addtogroup sg */
/*@{*/

/**
 * Represents a Node in the vtlib Scene Graph.
 */
class vtNode : public vtNodeBase, public osg::Referenced
{
public:
	virtual void Release();

	// implement vtNodeBase methods
	void SetEnabled(bool bOn);
	bool GetEnabled() const;

	/** Set the name of the node. */
	void SetName2(const char *str);
	/** Get the name of the node. */
	const char *GetName2() const;

	/** Get the Bounding Box of the node, in world coordinates */
	void GetBoundBox(FBox3 &box);

	/** Get the Bounding Sphere of the node, in world coordinates */
	void GetBoundSphere(FSphere &sphere, bool bGlobal = false);

	vtNode *GetParent(int iParent = 0);

	int GetTriCount() { return 0; }

	void SetFog(bool bOn, float start = 0, float end = 10000, const RGBf &color = s_white, int iType = GL_LINEAR);

	// implementation data
	void SetOsgNode(osg::Node *n);
	osg::Node *GetOsgNode() { return m_pNode.get(); }
	vtNodeBase *Clone() { return NULL; }

	static vtNode *LoadModel(const char *filename);

protected:
	osg::ref_ptr<osg::Node> m_pNode;
	osg::ref_ptr<osg::StateSet> m_pFogStateSet;
	osg::ref_ptr<osg::Fog> m_pFog;

	// Constructor is protected because vtNode is an abstract base class,
	//  not to be instantiated directly.
	vtNode();

	// Destructor is protected so that people will use Release() instead,
	//  to ensure that reference counting is respected.
	virtual ~vtNode();
};

/**
 * Represents a Group (a node that can have children) in the vtlib Scene Graph.
 */
class vtGroup : public vtNode, public vtGroupBase
{
public:
	vtGroup(bool suppress = false);
	vtNodeBase *Clone();
	void CopyFrom(const vtGroup *rhs);
	virtual void Release();

	// implement vtGroupBase methods

	/** Add a node as a child of this Group. */
	void AddChild(vtNodeBase *pChild);

	/** Remove a node as a child of this Group.  If the indicated node is not
	 a child, then this method has no effect. */
	void RemoveChild(vtNodeBase *pChild);

	/** Return a child node, by index. */
	vtNode *GetChild(int num) const;

	/** Return the number of child nodes */
	int GetNumChildren() const;

	/** Looks for a descendent node with a given name.  If not found, NULL
	 is returned. */
	const vtNodeBase *FindDescendantByName(const char *name) const;

	/** Return true if the given node is a child of this group. */
	bool ContainsChild(vtNodeBase *pNode) const;

	// OSG-specific Implementation
	osg::Group *GetOsgGroup() { return m_pGroup.get(); }
	const osg::Group *GetOsgGroup() const { return m_pGroup.get(); }

protected:
	void SetOsgGroup(osg::Group *g);
	virtual ~vtGroup();

	osg::ref_ptr<osg::Group> m_pGroup;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

class CustomTransform : public osg::MatrixTransform
{
public:
	inline osg::Matrix& getMatrix() { return _matrix; }
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
	vtNodeBase *Clone();
	void CopyFrom(const vtTransform *rhs);
	void Release();

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
	osg::ref_ptr<CustomTransform> m_pTransform;
	FPoint3			m_Scale;

protected:
	virtual ~vtTransform();
};

/**
 * A Light node is placed into the scene graph to illumninate all
 * lit geometry with vertex normals.
 */
class vtLight : public vtNode
{
public:
	vtLight();
	vtNodeBase *Clone();
	void CopyFrom(const vtLight *rhs);
	void Release();

	void SetColor(const RGBf &color);
	RGBf GetColor() const;
	void SetAmbient(const RGBf &color);
	RGBf GetAmbient() const;

	// provide override to catch this state
	virtual void SetEnabled(bool bOn);

	osg::ref_ptr<osg::LightSource> m_pLightSource;
	osg::ref_ptr<osg::Light> m_pLight;

protected:
	virtual ~vtLight();
};

/**
 * A utility class which simply wraps a light (vtLight) inside a
 * transform (vtTransform) so that you can move it.
 */
class vtMovLight : public vtTransform
{
public:
	vtMovLight(vtLight *pContained)
	{
		m_pLight = pContained;
		AddChild(pContained);
	}
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
class vtGeom : public vtNode
{
public:
	vtGeom();
	vtNodeBase *Clone();
	void CopyFrom(const vtGeom *rhs);
	void Release();

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
	void AddTextMesh(vtTextMesh *pMesh, int iMatIdx);

	/** Return the number of contained meshes. */
	int GetNumMeshes() const;

	/** Return a contained vtMesh by index. */
	vtMesh *GetMesh(int i) const;

	/** Return a contained vtTextMesh by index. */
	vtTextMesh *GetTextMesh(int i) const;

	virtual void SetMaterials(const class vtMaterialArray *mats);
	const vtMaterialArray	*GetMaterials() const;

	vtMaterial *GetMaterial(int idx);

	void SetMeshMatIndex(vtMesh *pMesh, int iMatIdx);

	osg::ref_ptr<const vtMaterialArray> m_pMaterialArray;
	osg::ref_ptr<osg::Geode> m_pGeode;	// the Geode is a container for Drawables

protected:
	virtual ~vtGeom();
};

/**
 * A utility class which simply wraps a geometry (vtGeom) inside a
 * transform (vtTransform) so that you can move it.
 */
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

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * We create our own OSG drawable in order to override the draw method.
 */
class OsgDynMesh : public osg::Drawable
{
public:
	OsgDynMesh();

	// overrides
	virtual osg::Object* cloneType() const { return new OsgDynMesh(); }
	virtual osg::Object* clone(const osg::CopyOp &foo) const { return new OsgDynMesh(); }
	virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const OsgDynMesh*>(obj)!=NULL; }
	virtual const char* className() const { return "OsgDynMesh"; }

	virtual bool computeBound() const;
	virtual void drawImplementation(osg::State& state) const;

	class vtDynGeom		*m_pDynGeom;

protected:
	virtual ~OsgDynMesh();
};

#endif // DOXYGEN_SHOULD_SKIP_THIS

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

	// A handy shortcut to the current clipping planes
	FPlane		*m_pPlanes;

protected:
	OsgDynMesh	*m_pDynMesh;
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
	vtLOD();
	void Release();

	void SetRanges(float *ranges, int nranges);
	void SetCenter(FPoint3 &center);

protected:
	osg::ref_ptr<osg::LOD>	m_pLOD;
	virtual ~vtLOD();
};

/**
 * A Camera is analogous to a physical camera: it description the location
 * of a point from which the scene is rendered.  It can either be a
 * perspective or orthographic camera, and it very easy to control
 * since it inherits all the methods of a transform (vtTransform).
 */
class vtCamera : public vtTransform
{
public:
	vtCamera();
	vtNodeBase*	Clone();
	void CopyFrom(const vtCamera *rhs);
	void Release();

	void SetHither(float f);
	float GetHither();
	void SetYon(float f);
	float GetYon();
	void SetFOV(float f);
	float GetFOV();

	void SetOrtho(bool bOrtho);
	bool IsOrtho();
	void SetWidth(float f);
	float GetWidth();

	void GetDirection(FPoint3 &dir);
	void ZoomToSphere(const FSphere &sphere);

protected:
	float m_fFOV;
	float m_fHither;
	float m_fYon;

	bool m_bOrtho;
	float m_fWidth;

	virtual ~vtCamera();
};

/**
 * A Sprite is a 2D object, which can contain text or an image.  It is
 * drawn in window coordinates, rather than 3D coordinates.  It is currently
 * unimplemented.
 */
class vtSprite : public vtNode
{
public:
	vtSprite();

	vtNodeBase *Clone();
	void Release();

	void SetText(const char *szText);
	void SetImage(vtImage *pImage);
	void SetPosition(bool bPixels, float l, float t, float r, float b);
	void SetWindowSize(int x, int y);

	void AddMesh(vtMesh *pMesh);
	void AddTextMesh(vtTextMesh *pTextMesh);

protected:
	vtMesh *m_pMesh;

	osg::ref_ptr<osg::Geode> m_geode;
	osg::ref_ptr<osg::Projection> m_projection;

	virtual ~vtSprite();
};

/*@}*/	// Group sg

#endif	// VTOSG_NODEH

