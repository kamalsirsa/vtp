//
// NodePSM.h
//
// Encapsulate behavior for PSM scene graph nodes.
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTPSM_NODEH
#define VTPSM_NODEH

// Visibility return codes
#undef VT_Visible
#undef VT_AllVisible
#define	VT_Visible	CAM_Visible
#define	VT_AllVisible	CAM_AllVisible

//
// A scene graph node that can move
// PSMBASE is the PSM base class to derive from. It must be a subclass of PSGroup.
//
template <class PSMBASE, class VTBASE> class vtpsNode : public VTBASE, public PSMBASE
{
public:
	vtpsNode(bool suppress_unused = true) : PSMBASE() { }

	// node stuff
	void		Destroy()						{ PSMBASE::Delete(); }
	vtNodeBase*	CreateClone()					{ return (vtNodeBase*) PSMBASE::Clone(); }
	void		SetEnabled(bool bOn)			{ PSMBASE::SetActive(bOn); }
	bool		GetEnabled()					{ return PSMBASE::IsActive(); }
	void		SetName2(const char *str)		{ PSMBASE::SetName(str); }
	const char*	GetName2()						{ return PSMBASE::GetName(); }
	void GetBoundBox(FBox3 &box)				{ PSBox3 box1; PSMBASE::GetBound(&box1); box = p2v(box1); }
	void GetBoundSphere(FSphere &sphere)		{ PSMBASE::GetBound((PSSphere*) &sphere); }
	void SetFog(bool bOn, float start = 0, float end = 10000, const RGBf &color = s_white, int iType = GL_LINEAR)
	{
		vtGetScene()->SetFog(bOn, start, end, color, iType);
	}

	// group stuff
	int			GetNumChildren()				{ return PSMBASE::GetSize(); }
	void		AddChild(vtNodeBase* pChild)
	{
		PSModel *pModel = dynamic_cast<PSModel *>(pChild);
		if (pModel)
			PSMBASE::Append(pModel);
	}
	void		RemoveChild(vtNodeBase* pChild)	{ ((vtNode*) pChild)->Remove(GROUP_DontFree); }
	vtNode*		GetChild(int num)				{ return (vtNode*) PSMBASE::GetAt(num); }
	bool		ContainsChild(vtNodeBase *pNode)
	{
		PSGroup* node = (PSGroup*) pNode;		// PSMBASE must derive from PSGroup
		PSGroup* parent = node->Parent();
		while (parent)
			if (parent == (PSGroup*) this)
				return true;
			else
				parent = parent->Parent();
		return false;
	}
	/** Looks for a descendent node with a given name.  If not found, NULL
	 is returned. */
	vtNodeBase*	FindDescendantByName(const char *name);
	{
		return (vtNodeBase*) PSMBASE::Find(name, GROUP_FindChild | GROUP_FindExact);
	}

	// transform stuff
	void Identity()								{ PSMBASE::Reset();	}
	FPoint3 GetTrans()							{ return p2v(PSMBASE::GetTranslation()); }
	void SetTrans(const FPoint3 &pos)			{ PSMBASE::SetTranslation(v2p(pos)); }

	void SetTransform1(const FMatrix4 &mat)		{ PSMatrix p; ConvertMatrix4(&mat, &p); PSMBASE::SetTransform(&p); }

	void GetTransform1(FMatrix4& mtx)			{ const PSMatrix *m = PSMBASE::GetTransform(); ConvertMatrix4(m, &mtx); }

	void Translate1(const FPoint3 &pos)			{ Translate(v2p(pos)); }
	void TranslateLocal(const FPoint3 &pos)		{ Move(v2p(pos)); }
	void Scale3(float x, float y, float z)		{ Size(PSVec3(x, y, z)); }
	void PointTowards(const FPoint3 &point)		{ LookAt(v2p(point), 0); }
	void Rotate2(const FPoint3 &axis, float angle)		{ Rotate(v2p(axis), angle); }
	void RotateLocal(const FPoint3 &axis, float angle)	{ Turn(v2p(axis), angle); }
	void RotateParent(const FPoint3 &axis, float angle)
	{
		PSVec3 trans = GetTranslation();

		Translate(-trans);
		Rotate(v2p(axis), angle);
		Translate(trans);
	}
private:
	void  operator delete (void* ptr)
	{
		vtpsNode<PSMBASE, VTBASE>* obj = (vtpsNode<PSMBASE, VTBASE>*) ptr;
		VAllocator* alloc = obj->GetAllocator();

		if (alloc)
			alloc->Free(ptr);
	}
};

/**
 * A Transform node allows you to apply a transform (scale, rotate, translate)
 * to all its child nodes.
 */
class vtNode : public vtpsNode<PSModel, vtNodeBase>
{
};

class vtGroup : public vtpsNode<PSModel, vtGroupBase>
{
};

class vtTransform : public vtpsNode<PSModel, vtTransformBase>
{
};

class vtLight : public vtpsNode<PSLight, vtTransformBase>
{
public:
	void SetColor2(const RGBf &color);
	void SetAmbient2(const RGBf &color);
	vtLight* GetLight()	{ return this; }
};

class vtMovLight : public vtTransform
{
public:
	vtMovLight(vtLight *pContained) : vtTransform() { m_pLight = pContained; AddChild(pContained); }
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
class vtGeom : public vtpsNode<PSShape, vtGeomBase>
{
public:
	void SetMaterials(class vtMaterialArray *mats);

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
	// psm methods for rendering and culling

	void CalcCullPlanes();

	// vt methods (must be overriden)
	virtual void DoRender() = 0;
	virtual void DoCalcBoundBox(FBox3 &box) = 0;
	virtual void DoCull(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov) = 0;
	virtual int GetPolygonCount() = 0;

	// for culling
	FPlane		m_cullPlanes[6];
};

class vtDynMesh : public PSGeometry
{
public:
	vtDynMesh(vtDynGeom& owner) : PSGeometry(), m_DynGeom(owner) { }

	int		GetNumFaces() const;
	int		GetNumVtx() const;
	int		Cull(const PSMatrix*, PSScene*);
	void	Render(PSScene*, const PSAppearances*);
	bool	GetBound(PSBox3*) const;
	bool	Hit(const PSRay& ray, PSTriHitEvent* hitinfo) const;

protected:
	vtDynGeom&	m_DynGeom;
};


/**
 * An LOD node controls the visibility of its child nodes.
 *
 * You should set a distance value (range) for each child, which determines
 * at what distance from the camera a node should be rendered.
 */
class vtLOD : public vtpsNode<PSLevelOfDetail, vtGroupBase>
{
public:
	void SetRanges(float* ranges, int nranges);
	void SetCenter(FPoint3& ctr);
};

class vtCamera : public vtpsNode<PSCamera, vtTransformBase>
{
public:
	void GetDirection(FPoint3 &dir);
	void ZoomToSphere(const FSphere &sphere);
	void SetOrtho(float fWidth);
};

class vtSprite : public vtpsNode<PSSprite, vtNodeBase>
{
public:
	void SetText(const char *msg);
	void SetWindowRect(float l, float t, float r, float b) {}
};

#endif
