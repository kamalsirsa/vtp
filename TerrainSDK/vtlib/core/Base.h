//
// Base.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_BASEH
#define VTLIB_BASEH

#include "vtdata/vtString.h"

#ifndef MAX
#define MAX(a, b) (((a)>(b))?(a):(b))
#endif

#ifndef MIN
#define MIN(a, b) (((a)>(b))?(b):(a))
#endif

enum vtMouseEventType
{
	VT_MOVE,
	VT_DOWN,
	VT_UP
};

#define VT_NONE		0
#define VT_LEFT		1
#define VT_RIGHT	2
#define VT_MIDDLE	4

// event flags: modifier keys for mouse and keyboard events
#define VT_SHIFT	1
#define VT_CONTROL	2
#define VT_ALT		4

struct vtMouseEvent
{
	vtMouseEventType type;
	int button;
	IPoint2 pos;
	int flags;
};

/**
 * This class simply provides the ability to store whether an object is
 * "enabled" or not.  This is generally useful, such as for Nodes or Engines
 * which can be turned on and off.
 */
class vtEnabledBase
{
public:
	vtEnabledBase() { m_bEnabled = true; }

	virtual void SetEnabled(bool bOn) { m_bEnabled = bOn; }
	bool GetEnabled() { return m_bEnabled; }

protected:
	bool m_bEnabled;
};

class vtNode;
class vtMaterial;
class vtMaterialArray;
class vtCamera;
class vtGroup;
class vtEngine;
class vtGeom;

/** \defgroup sg Scene Graph
 * These classes are used for scene graph functionality: geometry, cameras,
 * rendering, LOD, and simulation.
 */
/*@{*/

/** This class is a placeholder parent class for all objects which can
 * be the target of an Engine (vtEngine).  Given a vtTarget point, you
 * can use dynamic_cast<> to downcast safely to any specific child class.
 */
class vtTarget
{
public:
	// need at least one method to make this class polymorphic
	virtual void PlaceHolder() {}
};

/** Virtual base class for vtNode implementation. */
class vtNodeBase : public vtTarget
{
public:
	virtual vtNodeBase*	CreateClone() = 0;
	virtual void		Release() = 0;

	virtual void		SetEnabled(bool bOn) = 0;
	virtual bool		GetEnabled() = 0;

	virtual void		SetName2(const char *str) = 0;
	virtual const char*	GetName2() = 0;

	virtual void	GetBoundBox(FBox3 &box) = 0;
	virtual void	GetBoundSphere(FSphere &sphere, bool bGlobal = false) = 0;

	virtual vtNode*	GetParent(int iParent = 0) = 0;
	virtual int		GetTriCount() = 0;

	static RGBf s_white;
	virtual void SetFog(bool bOn, float start = 0, float end = 10000, const RGBf &color = s_white, int iType = GL_LINEAR) = 0;
};

/** Virtual Base class for a Group node (scene graph node that can have children). */
#if VTLIB_PSM
class vtGroupBase : public vtNodeBase
#else
class vtGroupBase
#endif
{
public:
	virtual void		AddChild(vtNodeBase* pChild) = 0;
	virtual void		RemoveChild(vtNodeBase* pChild) = 0;
	virtual vtNode*		GetChild(int num) = 0;
	virtual int			GetNumChildren() = 0;
	virtual bool		ContainsChild(vtNodeBase *pNode) = 0;
	virtual vtNodeBase*	FindDescendantByName(const char *name) = 0;
};

/** Abstract base class for a scene graph node that can move. */
#if VTLIB_PSM
class vtTransformBase : public vtGroupBase
#else
class vtTransformBase
#endif
{
public:
	virtual void Identity() = 0;

	// translation
	virtual FPoint3 GetTrans() = 0;
	virtual void SetTrans(const FPoint3 &pos) = 0;
	virtual void Translate1(const FPoint3 &pos) = 0;
	virtual void TranslateLocal(const FPoint3 &pos) = 0;

	// rotation
	virtual void Rotate2(const FPoint3 &axis, float angle) = 0;
	virtual void RotateLocal(const FPoint3 &axis, float angle) = 0;
	virtual void RotateParent(const FPoint3 &axis, float angle) = 0;

	// other
	virtual void Scale3(float x, float y, float z) = 0;
	virtual void PointTowards(const FPoint3 &point) = 0;
	virtual void SetTransform1(const FMatrix4 &mat) = 0;
	virtual void GetTransform1(FMatrix4 &mat) = 0;
};

// global function
class vtNodeBase *vtLoadModel(const char *fname);

#define VT_Normals		1
#define VT_Colors		2
#define VT_TexCoords	4

// Visibility return codes
#define	VT_Visible	1
#define	VT_AllVisible	2

/**
 * This class provides basic methods for the vtMesh class.
 */
class vtMeshBase
{
public:
	vtMeshBase(GLenum PrimType, int VtxType, int NumVertices)
	{
		m_ePrimType = PrimType;
		m_iVtxType = VtxType;
		m_iNumVertsUsed = 0;
		m_iMatIdx = -1;
	}

	// Accessors
	virtual int GetNumVertices() { return m_iNumVertsUsed; }
	GLenum GetPrimType() { return m_ePrimType; }
	int GetVtxType() { return m_iVtxType; }
	void GetBoundBox(FBox3 &box);

	void SetMatIndex(int i) { m_iMatIdx = i; }
	int GetMatIndex() { return m_iMatIdx; }

	// Adding vertices
	int AddVertex(float x, float y, float z);
	int AddVertexN(float x, float y, float z, float nx, float ny, float nz);
	int AddVertexUV(float x, float y, float z, float u, float v);

	int AddVertex(const FPoint3 &p);
	int AddVertexN(const FPoint3 &p, const FPoint3 &n);
	int AddVertexUV(const FPoint3 &p, float u, float v);
	int AddVertexUV(const FPoint3 &p, const FPoint2 &uv);
	int AddVertexNUV(const FPoint3 &p, const FPoint3 &n, const FPoint2 &uv);

	// Adding primitives
	virtual void AddTri(int p0, int p1, int p2) = 0;
	virtual void AddFan(int p0, int p1, int p2, int p3 = -1, int p4 = -1, int p5 = -1) = 0;
	virtual void AddFan(int *idx, int iNVerts) = 0;
	void AddStrip2(int iNVerts, int iStartIndex);
	virtual void AddStrip(int iNVerts, unsigned short *pIndices) = 0;

	// Access vertex properties
	virtual void SetVtxPos(int, const FPoint3&) = 0;
	virtual FPoint3 GetVtxPos(int i) const = 0;

	virtual void SetVtxNormal(int, const FPoint3&) = 0;
	virtual FPoint3 GetVtxNormal(int i) const = 0;

	virtual void SetVtxColor(int, const RGBf&) = 0;
	virtual RGBf GetVtxColor(int i) const = 0;

	virtual void SetVtxTexCoord(int, const FPoint2&) = 0;
	virtual FPoint2 GetVtxTexCoord(int i) = 0;

	void SetVtxPUV(int i, const FPoint3 &pos, float u, float v)
	{
		SetVtxPos(i, pos);
		SetVtxTexCoord(i, FPoint2(u, v));
	}
	void SetVtxPN(int i, const FPoint3 &pos, const FPoint3 &norm)
	{
		SetVtxPos(i, pos);
		SetVtxNormal(i, norm);
	}

	void TransformVertices(FMatrix4 &mat);

	void CreateEllipsoid(FPoint3 size, int res, bool hemi = false);
	void CreateBlock(const FPoint3& size);
	void CreateOptimizedBlock(const FPoint3& size);
	void CreateRectangularMesh(int xsize, int ysize);
	void CreateCylinder(float height, float radius, int res,
		bool bTop = true, bool bBottom = true, bool bCentered = true,
		int direction = 1);
	void CreateRectangle(float xsize, float ysize);
	void CreateConicalSurface(const FPoint3 &tip, double radial_angle,
							  double theta1, double theta2,
							  double r1, double r2, int res = 40);

protected:
	GLenum m_ePrimType;
	int m_iVtxType;
	int m_iMatIdx;

	// keep track of the number of vertices actually used
	int	m_iNumVertsUsed;
};

/**
 * This class provides basic methods for the vtImage class.
 */
class vtImageBase
{
public:
	vtImageBase() { m_bLoaded = false; }
	vtImageBase(const char *fname) { m_bLoaded = false; }

	/// Call after construction to see whether an image load succeeded.
	bool LoadedOK() { return m_bLoaded; }

	/// Return the name of the file, if any, from which the image was loaded.
	vtString GetFilename() { return m_strFilename; }

protected:
	bool m_bLoaded;
	vtString m_strFilename;
};


/**
 * This class provides the base methods for vtMaterial.
 */
class vtMaterialBase
{
public:
	vtMaterialBase();

	void CopyFrom(vtMaterial *pFrom);

	virtual void SetDiffuse(float r, float g, float b, float a = 1.0f) = 0;
	virtual RGBAf GetDiffuse() = 0;
	void SetDiffuse1(const RGBAf &c) { SetDiffuse(c.r, c.g, c.b, c.a); }
	void SetDiffuse2(float f) { SetDiffuse(f, f, f); }

	virtual void SetSpecular(float r, float g, float b) = 0;
	virtual RGBf GetSpecular() = 0;
	void SetSpecular1(const RGBf &c) { SetSpecular(c.r, c.g, c.b); }
	void SetSpecular2(float f) { SetSpecular(f, f, f); }

	virtual void SetAmbient(float r, float g, float b) = 0;
	virtual RGBf GetAmbient() = 0;
	void SetAmbient1(const RGBf &c) { SetAmbient(c.r, c.g, c.b); }
	void SetAmbient2(float f) { SetAmbient(f, f, f); }

	virtual void SetEmission(float r, float g, float b) = 0;
	virtual RGBf GetEmission() = 0;
	void SetEmission1(const RGBf &c) { SetEmission(c.r, c.g, c.b); }
	void SetEmission2(float f) { SetEmission(f, f, f); }

	virtual void SetCulling(bool bCulling) = 0;
	virtual bool GetCulling() = 0;

	virtual void SetLighting(bool bLighting) = 0;
	virtual bool GetLighting() = 0;

	virtual void SetTexture(class vtImage *pImage) = 0;

	virtual void SetTransparent(bool bOn, bool bAdd = false) = 0;
	virtual bool GetTransparent() = 0;

	virtual void Apply() = 0;

	void SetName(const vtString &name) { m_Name = name; }
	const vtString& GetName() { return m_Name; }

protected:
	vtString m_Name;
};

/**
 * Contains an array of materials.  Provides useful methods for creating
 * material easily.
 * This is the base class for vtMaterialArray, and provides most of the
 * functionality for that class.
 */
class vtMaterialArrayBase : public Array<vtMaterial *>
{
public:
	~vtMaterialArrayBase();

	void DestructItems(unsigned int first, unsigned int last);

	int AddTextureMaterial(class vtImage *pImage,
						 bool bCulling, bool bLighting,
						 bool bTransp = false, bool bAdditive = false,
						 float fAmbient = 0.0f, float fDiffuse = 1.0f,
						 float fAlpha = 1.0f, float fEmissive = 0.0f,
						 bool bTexGen = false, bool bClamp = false,
						 bool bMipMap = false);
	int AddTextureMaterial2(const char *fname,
						 bool bCulling, bool bLighting,
						 bool bTransp = false, bool bAdditive = false,
						 float fAmbient = 0.0f, float fDiffuse = 1.0f,
						 float fAlpha = 1.0f, float fEmissive = 0.0f,
						 bool bTexGen = false, bool bClamp = false,
						 bool bMipMap = false);
	int AddRGBMaterial(const RGBf &diffuse, const RGBf &ambient,
					 bool bCulling = true, bool bLighting= true, bool bWireframe = false,
					 float fAlpha = 1.0f, float fEmissive = 0.0f);
	int AddRGBMaterial1(const RGBf &diffuse,
					 bool bCulling = true, bool bLighting= true, bool bWireframe = false,
					 float fAlpha = 1.0f, float fEmissive = 0.0f);
	void AddShadowMaterial(float fOpacity);

	void CopyFrom(vtMaterialArrayBase *pFromMats);

	virtual int AppendMaterial(vtMaterial *pMat) = 0;
};

/**
 * vtSceneBase provides the Scene methods that are common across all
 * 3D libraries supported by vtlib.
 * See vtScene for an overview of the Scene class.
 */
class vtSceneBase
{
public:
	vtSceneBase();
	~vtSceneBase();

	virtual void SetBgColor(RGBf color) = 0;
	virtual void SetAmbient(RGBf color) = 0;

	void AddEngine(vtEngine *ptr);
	void RemoveEngine(vtEngine *ptr);
	int GetNumEngines() { return m_Engines.GetSize(); }
	vtEngine *GetEngine(int i) { return m_Engines.GetAt(i); }

	void SetCamera(vtCamera *cam) { m_pCamera = cam; }
	vtCamera *GetCamera() { return m_pCamera; }

	void OnMouse(vtMouseEvent &event);
	void OnKey(int key, int flags);
	void DoEngines();

	void SetWindowSize(int w, int h) { m_WindowSize.Set(w, h); }
	IPoint2 GetWindowSize() { return m_WindowSize; }

	virtual void SetRoot(vtGroup *pRoot) = 0;
	vtGroup *GetRoot() { return m_pRoot; }

protected:
	Array<vtEngine *> m_Engines;
	vtCamera *m_pCamera;
	IPoint2	m_WindowSize;
	vtGroup *m_pRoot;
};

// helper functions
vtGeom *Create3DCursor(float fSize, float fSmall, float fAlpha = 0.5f);
vtGeom *CreateBoundSphereGeom(const FSphere &sphere, int res = 24);
vtGeom *CreateSphereGeom(vtMaterialArray *pMats, int iMatIdx, int iVertType,
						 float fRadius, int res);
vtGeom *CreateCylinderGeom(vtMaterialArray *pMats, int iMatIdx, int iVertType,
						   float hHeight, float fRadius, int res,
						   bool bTop = true, bool bBottom = true,
						   bool bCentered = true, int direction = 1);
vtGeom *CreateLineGridGeom(vtMaterialArray *pMats, int iMatIdx,
					   FPoint3 min1, FPoint3 max1, int steps);

/*@}*/	// Group sg

#endif	// VTLIB_BASEH

